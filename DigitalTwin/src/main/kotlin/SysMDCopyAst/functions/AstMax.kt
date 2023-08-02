package SysMDCopyAst.functions

import ImportedSysMDExceptions.SemanticError
import ImportedSysMDQuantities.VectorQuantity
import ImportedSysMDQuantities.max
import ImportedSysMDServices.AgilaSession
import SysMDCopyAst.AstNode
import com.github.tukcps.jaadd.AADD
import com.github.tukcps.jaadd.DD
import com.github.tukcps.jaadd.IDD

/**
 * Predefined functions: exp
 * Computes exponential function e to the power of parameter
 */
internal class AstMax(model: AgilaSession, args: ArrayList<AstNode>) :
    AstFunction("max", model, 2, args) {

    /**
     * Checks the type of both parameters.
     */
    override fun initialize() {
        if ((getParam(0).upQuantity.values[0] !is AADD) && (getParam(0).upQuantity.values[0] !is IDD))
            throw SemanticError("Max only takes Real or Integer parameter")
        if ((getParam(1).upQuantity.values[0] !is AADD) && (getParam(1).upQuantity.values[0] !is IDD))
            throw SemanticError("Max only takes Real or Integer parameter")
        if (getParam(0).upQuantity.values[0] is AADD) {
            upQuantity = VectorQuantity(mutableListOf(model.builder.Reals),"?")
            if (getParam(1).upQuantity.values[0] !is AADD)
                throw SemanticError("Max requires both parameters of same type")
        }
        if (getParam(0).upQuantity.values[0] is IDD) {
            upQuantity = VectorQuantity(mutableListOf(model.builder.Integers))
            if (getParam(1).upQuantity.values[0] !is IDD)
                throw SemanticError("Max requires both parameters of same type")
        }
        evalUp()
        downQuantity = upQuantity.clone()
    }

    /**
     * Implementation of max(a, b)
     */
    override fun evalUp() {
        upQuantity =  max(getParam(0).upQuantity, getParam(1).upQuantity)
    }

    override fun evalDown() {
        val param1Down = getParam(0).downQuantity
        val param2Down = getParam(1).downQuantity
        val resultsParam1 = mutableListOf<DD>()
        val resultsParam2 = mutableListOf<DD>()
        downQuantity.values.indices.forEach {
            when (downQuantity.values[0]) {
                is AADD -> {
                    resultsParam1.add(param2Down.lt(downQuantity).bdd().ite(downQuantity.values[it] as AADD,
                        model.builder.range((param1Down.values[it] as AADD).getRange().min, (downQuantity.values[it] as AADD).getRange().max)))
                    resultsParam2.add( param1Down.lt(downQuantity).bdd().ite(downQuantity.values[it] as AADD,
                        model.builder.range((param2Down.values[it] as AADD).getRange().min, (downQuantity.values[it] as AADD).getRange().max)))
                }
                is IDD -> {
                    resultsParam1.add(param2Down.lt(downQuantity).bdd().ite(downQuantity.values[it] as IDD,
                        model.builder.rangeIDD((param1Down.values[it] as IDD).getRange().min, (downQuantity.values[it] as IDD).getRange().max)))
                    resultsParam2.add(param1Down.lt(downQuantity).bdd().ite(downQuantity.values[it] as IDD,
                        model.builder.rangeIDD((param2Down.values[it] as IDD).getRange().min, (downQuantity.values[it] as IDD).getRange().max)))
                }
                else -> {
                    throw SemanticError("Max only possible for IDD and AADD.")
                }
            }
        }
        getParam(0).downQuantity = getParam(0).downQuantity.constrain(VectorQuantity(resultsParam1, param1Down.unit))
        getParam(1).downQuantity = getParam(1).downQuantity.constrain(VectorQuantity(resultsParam2, param1Down.unit))
    }

    override fun toExpressionString() = "exp(${getParam(0).toExpressionString()})"

    override fun clone(): AstMax {
        val parClone = ArrayList<AstNode>()
        for (p in parameters) parClone.add(p.clone())
        return AstMax(model, parClone)
    }
}
