package SysMDCopyAst.functions

import ImportedSysMDExceptions.SemanticError
import ImportedSysMDQuantities.VectorQuantity
import ImportedSysMDQuantities.min
import ImportedSysMDServices.AgilaSession
import SysMDCopyAst.AstNode
import com.github.tukcps.jaadd.AADD
import com.github.tukcps.jaadd.DD
import com.github.tukcps.jaadd.IDD

/**
 * Predefined functions: exp
 * Computes exponential function e to the power of parameter
 */
internal class AstMin(model: AgilaSession, args: ArrayList<AstNode>) :
    AstFunction("min", model, 2, args) {

    /**
     * Checks the type of both parameters.
     */
    override fun initialize() {
        if ((getParam(0).upQuantity.values[0] !is AADD) && (getParam(0).upQuantity.values[0] !is IDD))
            throw SemanticError("Min only takes Real or Integer parameter")
        if ((getParam(1).upQuantity.values[0] !is AADD) && (getParam(1).upQuantity.values[0] !is IDD))
            throw SemanticError("Min only takes Real or Integer parameter")
        if (getParam(0).upQuantity.values[0] is AADD) {
            upQuantity = VectorQuantity(mutableListOf(model.builder.Reals),"?")
            if (getParam(1).upQuantity.values[0] !is AADD)
                throw SemanticError("Min requires both parameters of same type")
        }
        if (getParam(0).upQuantity.values[0] is IDD) {
            upQuantity = VectorQuantity(mutableListOf(model.builder.Integers))
            if (getParam(1).upQuantity.values[0] !is IDD)
                throw SemanticError("Min requires both parameters of same type")
        }
        evalUp()
        downQuantity = upQuantity.clone()
    }

    /**
     * Implementation of min(a, b)
     */
    override fun evalUp() {
        upQuantity =  min(getParam(0).upQuantity, getParam(1).upQuantity)
    }

    override fun evalDown() {
        val param1Down = getParam(0).upQuantity
        val param2Down = getParam(1).upQuantity
        val resultsParam1 = mutableListOf<DD>()
        val resultsParam2 = mutableListOf<DD>()
        downQuantity.values.indices.forEach {
            when(downQuantity.values[0]) {
                is AADD -> {
                    resultsParam1.add(param2Down.gt(downQuantity).bdd().ite(downQuantity.values[it] as AADD,
                        model.builder.range((downQuantity.values[it] as AADD).getRange().min,(param1Down.values[it] as AADD).getRange().max)))
                    resultsParam2.add(param1Down.gt(downQuantity).bdd().ite(downQuantity.values[it] as AADD,
                        model.builder.range((downQuantity.values[it] as AADD).getRange().min,(param2Down.values[it] as AADD).getRange().max)))
                }
                is IDD -> {
                    resultsParam1.add(param2Down.gt(downQuantity).bdd().ite(downQuantity.values[it] as IDD,
                        model.builder.rangeIDD((downQuantity.values[it] as IDD).getRange().min,(param1Down.values[it] as IDD).getRange().max)))
                    resultsParam2.add(param1Down.gt(downQuantity).bdd().ite(downQuantity.values[it] as IDD,
                        model.builder.rangeIDD((downQuantity.values[it] as IDD).getRange().min,(param2Down.values[it] as IDD).getRange().max)))
                }
                else -> {throw SemanticError("Max only possible for IDD and AADD.")}
            }
        }
        getParam(0).downQuantity = getParam(0).downQuantity.constrain(VectorQuantity(resultsParam1,param1Down.unit))
        getParam(1).downQuantity = getParam(1).downQuantity.constrain(VectorQuantity(resultsParam2,param2Down.unit))
    }

    override fun toExpressionString() = "min(${getParam(0).toExpressionString()}, ${getParam(1).toExpressionString()})"

    override fun clone(): AstMin {
        val parClone = ArrayList<AstNode>()
        for (p in parameters) parClone.add(p.clone())
        return AstMin(model, parClone)
    }
}
