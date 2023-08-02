package SysMDCopyAst.functions

import ImportedSysMDExceptions.SemanticError
import ImportedSysMDQuantities.VectorQuantity
import ImportedSysMDServices.AgilaSession
import SysMDCopyAst.AstNode
import com.github.tukcps.jaadd.AADD
import com.github.tukcps.jaadd.IDD

/**
 * Predefined functions: exp
 * Computes exponential function e to the power of parameter
 */
internal class AstExp(model: AgilaSession, args: ArrayList<AstNode>) :
    AstFunction("exp", model, 1, args) {

    override fun initialize() {
        upQuantity = when (getParam(0).upQuantity.values[0]) {
            is AADD -> VectorQuantity(mutableListOf(model.builder.Reals), "?")
            is IDD -> VectorQuantity(mutableListOf(model.builder.Integers))
            else -> throw SemanticError("Exp must have Real or Int argument")
        }
        evalUp()
        downQuantity = upQuantity.clone()
    }

    override fun evalUp() {
        upQuantity = getParam(0).upQuantity.exp()
    }

    override fun evalDown() {
        getParam(0).downQuantity = downQuantity.ln()
    }

    override fun toExpressionString() = "exp(${getParam(0).toExpressionString()})"

    override fun clone(): AstExp {
        val parClone = ArrayList<AstNode>()
        for (p in parameters) parClone.add(p.clone())
        return AstExp(model, parClone)
    }
}
