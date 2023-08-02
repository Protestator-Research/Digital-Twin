package SysMDCopyAst.functions

import ImportedSysMDExceptions.SemanticError
import ImportedSysMDQuantities.DDError
import ImportedSysMDQuantities.VectorQuantity
import ImportedSysMDServices.AgilaSession
import com.github.tukcps.jaadd.AADD
import com.github.tukcps.jaadd.BDD
import SysMDCopyAst.AstNode


/**
 * Predefined functions: toReal x: Bool -> Real
 * Casts an integer value to a real value.
 */
internal class AstToReal(model: AgilaSession, args: ArrayList<AstNode>) :
    AstFunction("toReal", model, 1, args) {

    init {
        if (parameters.size != 1)
            throw SemanticError("function toReal : Boolean -> Real expects 1 parameter")
    }

    override fun initialize() {
        if (!parameters[0].isBool)
            throw SemanticError("function toReal parameter must be of type Bool")
        upQuantity = VectorQuantity(mutableListOf(model.builder.Reals), "")
        evalUp()
        downQuantity = upQuantity.clone()
    }

    /**
     * up = if (true) 1.0 else 0.0
     */
    override fun evalUp() {
        val results = mutableListOf<AADD>()
        getParam(0).bdds.forEach {
            results.add(it.asBdd().ite(model.builder.range(1.0, 1.0), model.builder.range(0.0, 0.0)))
        }
        upQuantity = VectorQuantity(results,"1")
    }

    /**
     * down = if (
     */
    override fun evalDown() {
        val results = mutableListOf<BDD>()
        downQuantity.aadds().forEach {
            if(it.min == 0.0 && it.max == 0.0)
                results.add(model.builder.False)
            else if(it.min == 1.0 && it.max == 1.0)
                results.add(model.builder.True)
            else
                throw DDError("Eval Down of toReal only possible, if the value is 1.0 or 0.0.")
        }
        getParam(0).downQuantity = VectorQuantity(results)
    }


    override fun toExpressionString(): String {
        return "toReal(${getParam(0).toExpressionString()})"
    }

    override fun clone(): AstToReal {
        val parClone = ArrayList<AstNode>()
        for (p in parameters) parClone.add(p.clone())
        return AstToReal(model, parClone)
    }
}

