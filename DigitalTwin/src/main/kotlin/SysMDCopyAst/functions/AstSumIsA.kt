package SysMDCopyAst.functions

import BaseEntities.Element
import BaseEntities.Namespace
import BaseEntities.ValueFeature
import BaseEntities.getOwnedElementsOfType
import ImportedSysMDExceptions.SemanticError
import ImportedSysMDQuantities.Quantity
import ImportedSysMDServices.AgilaSession
import ImportedSysMDServices.resolveName
import Parser.Scanner
import SysMDCopyAst.AstBinOp
import SysMDCopyAst.AstLeaf
import SysMDCopyAst.AstNode
import com.github.tukcps.jaadd.AADD
import com.github.tukcps.jaadd.IDD

/**
 * The SumIsA function with parameter propertyAST.
 * The function takes a single parameter that is either name of a property of components
 * or a calculation with some properties. This properties must all be contained in the same subclasses,
 * otherwise it is not possible
 * The property is searched in each of its elements.
 * - if it is found in an element connected via isA-parts, the value is used.
 * - if it is not found in an element connected via has-parts,
 *   it is applied to its elements recursively.
 */
internal class AstSumIsA(
    model: AgilaSession,
    private val namespace: Namespace,
    private var propertyAst: List<AstNode>,
    private var transitive: Boolean
) : AstAggregationFunction("sumOverSubclasses", model) {

    private var generatedAst: AstNode? = null

    /**
     * Initialization; starts from bottom-up
     */
    override fun initialize() {
        upQuantity = Quantity(model.builder.Reals, "?")
        downQuantity = upQuantity
        if(propertyAst.size!=1)
            throw SemanticError("function 'sumOverSubclasses' expects one parameter" )
        generatedAst = model.initAstSumSubclasses(namespace, propertyAst.first(), transitive)
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
        val resultIsReady = when(resultingSum.values[0]){
            is AADD -> !resultingSum.values.any{it.asAadd().getRange().isEmpty()}
            is IDD -> !resultingSum.values.any{it.asIdd().getRange().isEmpty()}
            else -> false
        }
        if(resultIsReady) {
            // Set downQuantity to the root of the generatedAST
            generatedAst!!.upQuantity = resultingSum
            generatedAst!!.downQuantity = resultingSum
            generatedAst!!.evalDownRec()
            // Iterate through all leafs of the generatedAST and update downQuantity of the associated ValueFeature
            for (leaf in generatedAst!!.getLeaves().filter { it.qualifiedName != null }) {
                val valueFeature = model.global.resolveName<ValueFeature>(leaf.qualifiedName!!)

                when (leaf.downQuantity.values[0]) {
                    is AADD -> valueFeature!!.vectorQuantity = valueFeature.vectorQuantity.constrain(leaf.downQuantity)
                    is IDD -> valueFeature!!.vectorQuantity = valueFeature.vectorQuantity.constrain(leaf.downQuantity)
                    else -> {}
                }
            }
        }
    }

    override fun getDependentPropertyStrings(): Set<String> {
        return model.getPartDependencies(namespace,propertyAst.first())
    }

    override fun <T> runDepthFirst(block: AstNode.() -> T): T {
        val elements = namespace.getOwnedElementsOfType<Element>()
        for (element in elements) {
//            try { model.getProperty(model.selfId, Identification(null, propertyName))!!.ast!!.runDepthFirst(block) }
//            catch (ignore: Exception){ } // No property found, we can do nothing or recurse.
        }
        return this.run(block)
    }

    override fun clone(): AstFunction {
        return AstSumIsA(model, namespace.clone() as Namespace , listOf(propertyAst.first().clone()),transitive)
    }
}


/**
 * Function that generates an AST for a Sum over a composition.
 * The function considers all owned elements and searches in these elements for propertyName.
 * Then, it builds an AST that computes the sum.
 */
fun AgilaSession.initAstSumSubclasses(element: Element, propertyAST: AstNode, transitive: Boolean, isReal: Boolean = true): AstNode {
    var ast: AstNode? = null
    var isRealSum = isReal //indicates if the property is a real or an int
    for (subclass in getSubclasses(element)) {
        //iterate through all leafs of the propertyAST (which do not include only a number) to find the value for the properties.
        var newAstNode: AstNode = propertyAST.clone()
        for (leaf in newAstNode.getLeaves().filter { it.qualifiedName != null }) {
            // Find property with propertyName owned by element ...
            //TODO Could cause problems with inheritance or imports
            val ownedProperty = global.resolveName<ValueFeature>(subclass.qualifiedName + "::" + leaf.qualifiedName)
            if (ownedProperty != null) {
                leaf.upQuantity = ownedProperty.vectorQuantity
                leaf.downQuantity = ownedProperty.vectorQuantity
                leaf.qualifiedName = ownedProperty.qualifiedName
                leaf.property = ownedProperty
                if (leaf.upQuantity.values[0] is IDD) isRealSum = false
            } else if (transitive) { // Transitive: search property in parts
                newAstNode = this.initAstSumSubclasses(subclass as Element, propertyAST, true, isRealSum)
                break   // if one property of a leaf is not included in the current Element, there is no need to search
                // for the properties of the other leafs, because all properties of one propertyAST must contain to the same element
                // without this break statement, subclasses would be added multiple times to the ast
            } else
                break // no further look in subclasses because transitive search is not enabled
        }
        if(newAstNode.toString()!=propertyAST.toString())
            ast = if (ast == null) newAstNode else AstBinOp(newAstNode, Scanner.Definitions.Token.Kind.PLUS, ast)
    }
    return ast ?: if(isRealSum)
        AstLeaf(this, Quantity(builder.scalar(0.0),"?"))
    else
        AstLeaf(this, Quantity(builder.scalar(0)))
}
