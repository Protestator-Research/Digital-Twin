package SysMDCopyAst.functions

import BaseEntities.Namespace
import BaseEntities.ValueFeature
import ImportedSysMDExceptions.SemanticError
import ImportedSysMDQuantities.Quantity
import ImportedSysMDQuantities.VectorDimensionError
import ImportedSysMDQuantities.VectorQuantity
import ImportedSysMDServices.AgilaSession
import ImportedSysMDServices.report
import ImportedSysMDServices.resolveName
import Parser.QualifiedName
import SysMDCopyAst.AstLeaf
import SysMDCopyAst.AstNode
import com.github.tukcps.jaadd.DD
import com.github.tukcps.jaadd.functions.ite
/**
 * Computes the range of a property given as parameter in all subclasses.
 * I.e. if subclasses have the values 1,2,3, an AADD containing 1, 2, 3
 * will be built.
 */
class AstBySubclasses(model: AgilaSession, namespace: Namespace, args: ArrayList<AstNode>) :
    AstFunction("bySubclasses", model, 1, args) {

    private val inNameSpace: Namespace
    private val propertyName: QualifiedName

    init {
        inNameSpace = namespace
        propertyName = (getParam(0) as AstLeaf).qualifiedName !!
    }

    /**
     * Searches in all subclasses for the respective property and sets it in this element to the ITE-combination of
     * all found properties in subclasses.
     */
    override fun evalUp() {
        var subclasses = model.getSubclasses(inNameSpace)
        if (subclasses.isNotEmpty()) {
            val firstSubclass = subclasses.first()
            subclasses = subclasses.drop(1)
            val quantity = firstSubclass.resolveName<ValueFeature>(propertyName)!!.vectorQuantity
            var result: DD = quantity.values[0].clone()
            for (subclass in subclasses) {
                // TODO: generate a property for it!
                val chooser = model.builder.variable("choose_+${subclass.qualifiedName}")
                val newSubclassProperty = subclass.resolveName<ValueFeature>(propertyName)
                    ?: throw SemanticError("Missing value $propertyName in ${subclass.qualifiedName}")
                if (newSubclassProperty.vectorQuantity.unit != quantity.unit)
                    model.report(inNameSpace, "different units in different subclasses")
                result = chooser.ite(result, newSubclassProperty.vectorQuantity.values[0])
            }
            this.upQuantity = VectorQuantity(result, quantity.unit.clone())
        }
    }

    /**
     * Sets the result data type for static type checking; as the subclasses might
     * not yet be known, the function just uses the declared type the value.
     * NO EVAL-UP is done; this function shall only be used alone.
     */
    override fun initialize() {
        //TODO Add Vectos to bySubclasses
        if(getParam(0).upQuantity.values.size!=1)
            throw VectorDimensionError("BySubclasses is not possible with Vectors")
        val type = inNameSpace.resolveName<ValueFeature>(propertyName)?.ofClass?.ref

        if (type != null && type.model?.builder != model.builder)
            throw Exception("Internal error -- Mix of two models?")

        upQuantity = when  {
            type == null -> throw SemanticError("Null type in function bySubclasses")
            type.isSubclassOf(model.repo.booleanType) -> Quantity(model.builder.Bool)
            type.isSubclassOf(model.repo.integerType) -> Quantity(model.builder.Integers)
            type.isSubclassOf(model.repo.realType) -> Quantity(model.builder.Reals, "?")
            else -> throw Exception("Unknown data type in function bySubclasses: '${type.qualifiedName}'")
        }
        downQuantity = upQuantity.clone()
    }

    /**
     * Does a depth-first traversal and applies the lambda parameter block.
     * @param receiver the ast node that is visited
     * @param block the lambda that is applied depth-first
     */
    override fun <R> withDepthFirst(receiver: AstNode, block: AstNode.() -> R): R {
        val subclasses = model.getSubclasses(inNameSpace)
        for (subclass in subclasses) {
            val ast = subclass.resolveName<ValueFeature>(propertyName)?.ast
            if (ast != null) withDepthFirst(ast, block)
        }
        return block()
    }

    /**
     * Does nothing.
     */
    override fun evalDown() {
    }

    /**
     * Does nothing so far.
     */
    override fun evalDownRec() {
    }

    override fun toExpressionString() = "bySubclasses($propertyName)"
}
