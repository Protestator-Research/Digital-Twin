package SysMDCopyAst.functions

import ImportedSysMDExceptions.SemanticError
import ImportedSysMDQuantities.Quantity
import ImportedSysMDServices.AgilaSession
import SysMDCopyAst.AstNode
import com.github.tukcps.jaadd.AADD
import com.github.tukcps.jaadd.IDD

/**
 * Predefined functions: 2 power parameter of type Real or Int
 */
internal class AstPower2(model: AgilaSession, args: ArrayList<AstNode>) :
    AstFunction("power2", model, 1, args) {

    init {
        if (args.size !in 1..1)
            throw SemanticError("pow2 expects 1 parameter of type Real")
    }

    override fun initialize() {
        upQuantity = when (getParam(0).upQuantity.values[0]) {
            is AADD -> Quantity(model.builder.Reals,"")
            is IDD -> Quantity(model.builder.Integers)
            else -> throw SemanticError("power2 must have Real or Int argument")
        }
        evalUp()
        downQuantity = upQuantity.clone()
    }

    override fun evalUp() {
        upQuantity = getParam(0).upQuantity.pow2()
    }

    override fun evalDown() {
        when (getParam(0).dd) {
            is AADD -> getParam(0).downQuantity = downQuantity.log(model.builder.scalar(2.0))
            is IDD -> getParam(0).downQuantity = downQuantity.log(model.builder.scalar(2))
            else -> {}
        }
    }

    override fun clone(): AstPower2 {
        val parClone = ArrayList<AstNode>()
        for (p in parameters) parClone.add(p.clone())
        return AstPower2(model, parClone)
    }
}
