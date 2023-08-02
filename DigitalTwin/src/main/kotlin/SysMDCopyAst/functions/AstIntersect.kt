package SysMDCopyAst.functions

import ImportedSysMDExceptions.SemanticError
import ImportedSysMDQuantities.VectorQuantity
import ImportedSysMDServices.AgilaSession
import SysMDCopyAst.AstNode
import com.github.tukcps.jaadd.AADD
import com.github.tukcps.jaadd.BDD
import com.github.tukcps.jaadd.DD
import com.github.tukcps.jaadd.IDD


/**
 * Predefined functions: intersect
 * Computes exponential function e to the power of parameter
 */
internal class AstIntersect(model: AgilaSession, args: ArrayList<AstNode>) :
    AstFunction("min", model, 2, args) {

    /**
     * Checks the type of both parameters.
     */
    override fun initialize() {
        if ((getParam(0).upQuantity.values[0] !is AADD) && (getParam(0).upQuantity.values[0] !is IDD))
            throw SemanticError("intersect only takes Real or Integer parameter")
        if ((getParam(1).upQuantity.values[0] !is AADD) && (getParam(1).upQuantity.values[0] !is IDD))
            throw SemanticError("intersect only takes Real or Integer parameter")
        if (getParam(0).upQuantity.values[0] is AADD) {
            upQuantity = VectorQuantity(mutableListOf(model.builder.Reals),"?")
            if (getParam(1).upQuantity.values[0] !is AADD)
                throw SemanticError("intersect requires both parameters of same type")
        }
        if (getParam(0).upQuantity.values[0] is IDD) {
            upQuantity = VectorQuantity(mutableListOf(model.builder.Integers))
            if (getParam(1).upQuantity.values[0] !is IDD)
                throw SemanticError("intersect requires both parameters of same type")
        }
        evalUp()
        downQuantity = upQuantity.clone()
    }

    /**
     * Implementation of intersect(a, b)
     */
    override fun evalUp() {
        // compute intersection
        val results = mutableListOf<DD>()
        getParam(0).upQuantity.values.indices.forEach {
            results.add( when(getParam(0).upQuantity.values[0]) {
                is AADD -> (getParam(0).upQuantity.values[it] as AADD).intersect(getParam(1).upQuantity.values[it] as AADD)
                is IDD -> (getParam(0).upQuantity.values[it] as IDD).intersect(getParam(1).upQuantity.values[it] as IDD)
                is BDD -> (getParam(0).upQuantity.values[it] as BDD).intersect(getParam(1).upQuantity.values[it] as BDD)
                else -> { throw  SemanticError("intersect only defined on Integer, Real, Boolean.") }
            })
        }
        upQuantity =  VectorQuantity(results, getParam(0).upQuantity.unit, getParam(0).upQuantity.unitSpec)
    }

    /**
     * Implementation of inverse function of y = intersect(a, b):
     * we maintain very optimistically the downwards result y for a and b.
     * (could be more precise).
     */
    override fun evalDown() {
        getParam(0).downQuantity = downQuantity.clone()
        getParam(1).downQuantity = downQuantity.clone()
    }

    override fun toExpressionString() = "intersect(${getParam(0).toExpressionString()}, ${getParam(1).toExpressionString()})"

    override fun clone(): AstIntersect {
        val parClone = ArrayList<AstNode>()
        for (p in parameters) parClone.add(p.clone())
        return AstIntersect(model, parClone)
    }
}

