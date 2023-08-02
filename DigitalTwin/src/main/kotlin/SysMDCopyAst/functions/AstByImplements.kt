package SysMDCopyAst.functions

import BaseEntities.Association
import BaseEntities.Namespace
import BaseEntities.ValueFeature
import ImportedSysMDExceptions.SemanticError
import ImportedSysMDQuantities.Quantity
import ImportedSysMDServices.AgilaSession
import ImportedSysMDServices.getRelationshipsTo
import ImportedSysMDServices.report
import ImportedSysMDServices.resolveName
import Parser.QualifiedName
import com.github.tukcps.jaadd.AADD
import com.github.tukcps.jaadd.BDD
import com.github.tukcps.jaadd.IDD
import SysMDCopyAst.AstLeaf
import SysMDCopyAst.AstNode


/**
 * Computes the range of a value given as parameter via the "implements" relationship.
 */
class AstByImplements(model: AgilaSession, namespace: Namespace, args: ArrayList<AstNode>) :
    AstFunction("byImplements", model, 1, args) {

    private val inNameSpace: Namespace
    private val propertyName: QualifiedName
    private var property: ValueFeature? = null
    private var implementsAssociation: Association? = null

    init {
        inNameSpace = namespace
        propertyName = (getParam(0) as AstLeaf).qualifiedName !!
    }

    /**
     * Searches in all subclasses for the respective property and sets it in this element to the ITE-combination of
     * all found properties in subclasses.
     */
    override fun evalUp() {
        // TODO: Currently simple 1:1 relationship
        val implements = model.getRelationshipsTo(property?.owner?.ref!!, "*", implementsAssociation)
        if (implements.size < 1)
            throw SemanticError("implements relationship not found")
        if (implements.first().source.size < 1) throw SemanticError("implements source not found")
        val component = implements.first().source.first().ref as Namespace
        val result = component.resolveName<ValueFeature>(propertyName)!!.vectorQuantity
        this.upQuantity = result.clone()
        property?.vectorQuantity = result.clone()
      //  model.reportInfo(inNameSpace, "bySubclasses($propertyName) used, but class ${inNameSpace.qualifiedName} has no subclasses")
    }

    override fun initialize() {
        property = inNameSpace.resolveName(propertyName)
        implementsAssociation = model.global.resolveName("implements")
        if (property == null)
            model.report(this.property, "Could not find property '$propertyName'")
        if (implementsAssociation == null)
            model.report(this.property, "Could not find Association 'implements' in model.")
        upQuantity = when (super.getParam(0).upQuantity.values[0]) {
            is BDD -> Quantity(model.builder.Bool)
            is IDD -> Quantity(model.builder.Integers)
            is AADD -> Quantity(model.builder.Reals, "?")
            else -> throw Exception("Unknown data type!")
        }
        evalUp()
        downQuantity = upQuantity.clone()
    }

    /**
     * Does a depth-first traversal and applies the lambda parameter block.
     * @param receiver the ast node that is visited
     * @param block the lambda that is applied depth-first
     */
    override fun <R> withDepthFirst(receiver: AstNode, block: AstNode.() -> R): R {
        return block()
    }

    /**
     * Does nothing so far.
     */
    override fun evalDown() {}

    /**
     * Does nothing so far.
     */
    override fun evalDownRec() {}

    override fun toExpressionString() = "byImplements($propertyName)"
}
