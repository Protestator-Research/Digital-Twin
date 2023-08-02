package SysMDCopyAst.functions

import ImportedSysMDExceptions.SemanticError
import ImportedSysMDQuantities.VectorQuantity
import ImportedSysMDServices.AgilaSession
import SysMDCopyAst.AstNode
import com.github.tukcps.jaadd.AADD
import com.github.tukcps.jaadd.IDD

/**
 * Predefined functions: abs, absolute value for IDD and AADD
 */
internal class AstAbs(model: AgilaSession, args: ArrayList<AstNode>) :
    AstFunction("abs", model, 1, args) {
    private val arg: AstNode = getParam(0)

    override fun initialize() {
        upQuantity = when (arg.upQuantity.values[0]) {
            is AADD -> VectorQuantity(mutableListOf(model.builder.Reals), "?")
            is IDD -> VectorQuantity(mutableListOf(model.builder.Integers))
            else -> throw SemanticError("abs must have Real or Int argument")
        }
        evalUp()
        downQuantity = upQuantity.clone()
    }

    override fun evalUp() {
        upQuantity = getParam(0).upQuantity.abs()
    }

    override fun evalDown() {
        when (arg.upQuantity.values[0]) {
            is IDD -> {
                val max = downQuantity.idd().getRange().max
                //interval from -max to max, because max is always positive
                val results = mutableListOf<IDD>()
                getParam(0).downQuantity.values.forEach {
                    results.add(it.asIdd().constrainTo(model.builder.rangeIDD(-max,max)))
                }
                getParam(0).downQuantity =  VectorQuantity(results)
            }
            is AADD ->{
                val max = downQuantity.aadd().getRange().max
                //interval from -max to max, because max is always positive
                val results = mutableListOf<AADD>()
                getParam(0).downQuantity.values.forEach {
                    results.add(it.asAadd().constrainTo(model.builder.range(-max,max)))
                }
                getParam(0).downQuantity =  VectorQuantity(results,getParam(0).downQuantity.unit,getParam(0).downQuantity.unitSpec)            }
            else -> {}
        }
    }

    override fun clone(): AstAbs {
        val parClone = ArrayList<AstNode>()
        for (p in parameters) parClone.add(p.clone())
        return AstAbs(model, parClone)
    }
}
