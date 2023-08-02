package SysMDCopyAst.functions

import ImportedSysMDExceptions.SemanticError
import ImportedSysMDQuantities.VectorQuantity
import ImportedSysMDServices.AgilaSession
import SysMDCopyAst.AstNode
import com.github.tukcps.jaadd.AADD
import com.github.tukcps.jaadd.IDD

/**
 * Predefined functions: ln, natural logarithm.
 */
internal class AstLn(model: AgilaSession, args: ArrayList<AstNode>) :
    AstFunction("ln", model, 1, args) {

    override fun initialize() {
        upQuantity = when (getParam(0).upQuantity.values[0]) {
            is AADD -> VectorQuantity(mutableListOf(model.builder.Reals), "?")
            is IDD -> VectorQuantity(mutableListOf(model.builder.Integers))
            else -> throw SemanticError("ln must have Real or Int argument")
        }
        evalUp()
        downQuantity = upQuantity.clone()
    }

    override fun evalUp() {
        upQuantity = getParam(0).upQuantity.ln()
    }

    override fun evalDown() {
        getParam(0).downQuantity = downQuantity.exp()
    }

    override fun clone(): AstLn {
        val parClone = ArrayList<AstNode>()
        for (p in parameters) parClone.add(p.clone())
        return AstLn(model, parClone)
    }
}
