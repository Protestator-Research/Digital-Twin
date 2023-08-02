package SysMDCopyAst.functions

import BaseEntities.*
import ImportedSysMDExceptions.SemanticError
import ImportedSysMDQuantities.Quantity
import ImportedSysMDServices.AgilaSession
import ImportedSysMDServices.findAllOwnedElements
import ImportedSysMDServices.resolveName
import Parser.Scanner
import SysMDCopyAst.AstBinOp
import SysMDCopyAst.AstLeaf
import SysMDCopyAst.AstNode
import com.github.tukcps.jaadd.AADD
import com.github.tukcps.jaadd.IDD

/**
 * The SumHasA function with parameter propertyAST.
 * The function takes a single parameter that is either name of a property of components
 * or a calculation with some properties. This properties must all be contained in the same subclasses,
 * otherwise it is not possible
 * The property is searched in each of its elements.
 * - if it is found in an element connected via hasA-parts, the value is used.
 * - if it is not found in an element connected via has-parts,
 *   it is applied to its elements recursively.
 */
internal class AstSumHasA(
    model: AgilaSession,
    private val namespace: Namespace,
    private var propertyAst: List<AstNode>,
    private var transitive: Boolean
) :
    AstAggregationFunction("sumOverParts", model) {

    private var generatedAst: AstNode? = null

    /**
     * Initialization; starts from bottom-up
     */
    override fun initialize() {
        upQuantity = Quantity(model.builder.Reals, "?")
        if (propertyAst.size != 1)
            throw SemanticError("function 'sumOverParts' expects one parameter" )
        generatedAst = model.initSumOverComposition(namespace, propertyAst.first(), transitive)
        generatedAst!!.runDepthFirst { initialize() } //initialize Real fkt in AST
        generatedAst!!.evalUpRec()
        evalUpRec()
        downQuantity = upQuantity.clone()
    }


    /**
     * Just compute the AST as set up in the init section.
     * Still no support for integers, requires adding operators Real * Int on dD
     **/
    override fun evalUp() {
        // upQuantity = model.getElement(elementUId).sumOverComposition(model, propertyName)!!
        generatedAst!!.evalUpRec()
        upQuantity = generatedAst!!.upQuantity
    }


    /**
     * Evaluate the properties of all owned elements.
     */
    override fun evalUpRec() {
        val ownedElements = namespace.getOwnedElementsOfType<ValueFeature>()
        for (elem in ownedElements) {
            try {
                elem.ast?.evalUp()
            } catch (ignore: Exception) {
            }
        }
        evalUp()
    }


    /** Just compute the AST as set up in the init section.*/
    override fun evalDown() {
        val resultingSum = downQuantity
        //only do evalDown, if value is ready (interval should not be empty)
        val resultIsReady = when (resultingSum.values[0]) {
            is AADD -> !resultingSum.values.any{it.asAadd().getRange().isEmpty()}
            is IDD -> !resultingSum.values.any{it.asIdd().getRange().isEmpty()}
            else -> false
        }
        if (resultIsReady) {
            // Set downQuantity to the root of the generatedAST
            generatedAst!!.upQuantity = resultingSum
            generatedAst!!.downQuantity = resultingSum
            generatedAst!!.evalDownRec()
            // Iterate through all leafs of the generatedAST and update downQuantity of the associated ValueFeature
            for (leaf in generatedAst!!.getLeaves().filter { it.qualifiedName != null }) {
                val valueFeature = model.global.resolveName<ValueFeature>(leaf.qualifiedName!!)
                if (valueFeature != null) {
                    when (leaf.downQuantity.values[0]) {
                        is AADD -> valueFeature.vectorQuantity = valueFeature.vectorQuantity.constrain(leaf.downQuantity)
                        is IDD -> valueFeature.vectorQuantity = valueFeature.vectorQuantity.constrain(leaf.downQuantity)
                        else -> {}
                    }
                }
            }
        }
    }

    override fun getDependentPropertyStrings(): Set<String> {
        return getSubclassDependencyStrings(namespace, propertyAst.first())
    }

    override fun <T> runDepthFirst(block: AstNode.() -> T): T {
        val elements = namespace.getOwnedElementsOfType<Element>()
        for (element in elements) {
//            try { model.getProperty(model.selfId, Identification(null, propertyName))!!.ast!!.runDepthFirst(block) }
//            catch (ignore: Exception){ } // No property found, we can do nothing or recurse.
        }
        return this.run(block)
    }
    override fun clone(): AstSumHasA {
        return AstSumHasA(model, namespace, listOf(propertyAst.first().clone()), transitive)
    }
}


/**
 * Function that generates an AST for a Sum over a composition.
 * The function considers all owned elements and searches in these elements for propertyName.
 * Then, it builds an AST that computes the sum.
 */
fun AgilaSession.initSumOverComposition(element: Namespace, propertyAST: AstNode, transitive: Boolean, isReal: Boolean = true): AstNode {
    var ast: AstNode? = null
    var isRealSum = isReal //indicates if the property is a real or an int
    for (elementIterator in element.findAllOwnedElements().filterIsInstance<Feature>()) {
        var newAstNode: AstNode = propertyAST.clone()
        for (leaf in newAstNode.getLeaves().filter { it.qualifiedName != null }) {
            // Find property with propertyName owned by element ...
            val ownedProperty =
                (elementIterator.ofClass?.ref as Namespace).resolveName<ValueFeature>(leaf.qualifiedName as String)
            if (ownedProperty != null) {
                leaf.upQuantity = ownedProperty.vectorQuantity
                leaf.downQuantity = ownedProperty.vectorQuantity
                leaf.qualifiedName = ownedProperty.qualifiedName
                leaf.property = ownedProperty
                if (leaf.upQuantity.values[0] is IDD) isRealSum = false
            } else if (transitive && elementIterator !is ValueFeature) { // Transitive: search property in parts (not for ValueFeatures)
                val elementRef = elementIterator.ofClass?.ref!!
                newAstNode = initSumOverComposition(elementRef, propertyAST, true, isRealSum)
                break   // if one property of a leaf is not included in the current Element, there is no need to search
                // for the properties of the other leafs, because all properties of one propertyAST must contain to the same element
                // without this break statement, subclasses would be added multiple times to the ast
            } else
                break // no further look in parts because transitive search is not enabled
        }
        if (newAstNode.toString() != propertyAST.toString()) {
            val multiplicity = elementIterator.multiplicityProperty
            val multiplicityLeaf = if (multiplicity != null) AstLeaf(this, multiplicity) else null
            val multiplicityConverted = if (isRealSum)
                if (multiplicityLeaf != null) AstReal(this, arrayListOf(multiplicityLeaf)) else null //convert int node to real
            else
                multiplicityLeaf // is int
            ast = if (ast == null) {
                if (multiplicityConverted != null) AstBinOp(newAstNode,
                    Scanner.Definitions.Token.Kind.TIMES, multiplicityConverted) else newAstNode
            } else {
                if (multiplicityConverted != null) AstBinOp(AstBinOp(newAstNode,
                    Scanner.Definitions.Token.Kind.TIMES, multiplicityConverted),
                    Scanner.Definitions.Token.Kind.PLUS, ast)
                else AstBinOp(newAstNode, Scanner.Definitions.Token.Kind.PLUS, ast)
            }
        }
    }
    return ast ?: if (isRealSum)
        AstLeaf(this, Quantity(builder.scalar(0.0), "?"))
    else
        AstLeaf(this, Quantity(builder.scalar(0)))
}
