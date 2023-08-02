package SysMDCopyAst.functions

import ImportedSysMDExceptions.SemanticError
import ImportedSysMDQuantities.VectorQuantity
import ImportedSysMDServices.AgilaSession
import SysMDCopyAst.AstNode
import com.github.tukcps.jaadd.AADD
import com.github.tukcps.jaadd.IDD

/**
 * Predefined functions: sqrt
 */
internal class AstSqrt(model: AgilaSession, args: ArrayList<AstNode>) : AstFunction("sqrt", model, 1, args) {
    init {
        if (args.size !in 1..1)
            throw SemanticError("sqrt expects 1 parameter of type Real or Integer")
    }

    override fun initialize() {
        upQuantity = when (getParam(0).upQuantity.values[0]) {
            is AADD -> VectorQuantity(mutableListOf(model.builder.Reals), "?")
            is IDD -> VectorQuantity(mutableListOf(model.builder.Integers))
            else -> throw SemanticError("sqrt must have Real or Int argument")
        }
        evalUp()
        downQuantity = upQuantity.clone()
    }

    override fun evalUp() {
        upQuantity = (getParam(0).upQuantity.sqrt())
    }

    override fun evalDown() {
        getParam(0).downQuantity = downQuantity.sqr()
    }


    override fun <T> runDepthFirst(block: AstNode.() -> T): T {
        for (p in parameters) p.runDepthFirst(block)
        return block()
    }

    override fun clone(): AstSqrt {
        val parClone = ArrayList<AstNode>()
        for (p in parameters) parClone.add(p.clone())
        return AstSqrt(model, parClone)
    }
}
