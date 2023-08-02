package SysMDCopyAst.functions

import ImportedSysMDExceptions.SemanticError
import ImportedSysMDQuantities.Quantity
import ImportedSysMDQuantities.VectorQuantity
import ImportedSysMDServices.AgilaSession
import SysMDCopyAst.AstNode
import com.github.tukcps.jaadd.BDD
/**
 * Predefined functions: not
 */
class AstNot(model: AgilaSession, args: ArrayList<AstNode>) :
    AstFunction("not", model, 1, args) {

    init {
        if (parameters.size != 1)
            throw SemanticError("function not expects 1 parameter")
    }

    override fun initialize() {
        upQuantity = when (getParam(0).upQuantity.values[0]) {
            is BDD -> Quantity(model.builder.Bool)
            else -> throw SemanticError("function not parameter must be of type Bool")
        }
        evalUp()
        downQuantity = upQuantity.clone()
    }


    override fun evalUp() {
        var results = mutableListOf<BDD>()
        getBDDParam(0).upQuantity.values.forEach{
            results.add(it.asBdd().not())
        }
        upQuantity = VectorQuantity(results, getParam(0).upQuantity.unit, getParam(0).upQuantity.unitSpec)
    }

    /** No propagation; Boolean functions are handled by discrete solver */
    override fun evalDown() {}


    override fun toExpressionString(): String {
        return "not(${getParam(0).toExpressionString()})"
    }

    override fun clone(): AstNot {
        val parClone = ArrayList<AstNode>()
        for (p in parameters) parClone.add(p.clone())
        return AstNot(model, parClone)
    }
}
