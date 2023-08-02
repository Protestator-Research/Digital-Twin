package SysMDCopyAst.functions

import ImportedSysMDExceptions.SemanticError
import ImportedSysMDQuantities.Quantity
import ImportedSysMDQuantities.VectorDimensionError
import ImportedSysMDServices.AgilaSession
import SysMDCopyAst.AstNode
import com.github.tukcps.jaadd.AADD
import com.github.tukcps.jaadd.DD
import com.github.tukcps.jaadd.IDD

/**
 * Predefined functions: 2 power parameter of type Real or Int
 */
internal class AstPower(model: AgilaSession, args: ArrayList<AstNode>) : AstFunction("power", model, 2, args) {

    private val exponent: DD
        get() = getParam(1).dd

    init {
        if (args.size != 2)
            throw SemanticError("power expects 2 parameter of type Real or Integer")
    }

    override fun initialize() {
        if(getParam(1).upQuantity.values.size!=1)
            throw VectorDimensionError("Power is not possible with Vector as exponent")
        upQuantity = when (getParam(0).upQuantity.values[0]) {
            is AADD -> Quantity(model.builder.Reals,"")
            is IDD -> Quantity(model.builder.Integers)
            else -> throw SemanticError("power must have Real or Int argument")
        }
        evalUp()
        downQuantity = upQuantity.clone()
    }

    override fun evalUp() {
        upQuantity = if (upQuantity.values[0] is AADD)
            getParam(0).upQuantity.pow(getParam(1).aadd)
        else
            getParam(0).upQuantity.pow(getParam(1).idd)

    }

    override fun evalDown() { //TODO Many special cases for negative numbers are missing
        getParam(0).downQuantity = when(exponent){
            is AADD -> downQuantity.pow(model.builder.scalar(1.0).div(exponent.asAadd()))
            is IDD  -> Quantity((downQuantity.values[0] as IDD).root(exponent as IDD))
            else -> throw SemanticError("Expect base of type Real or Integer")
        }
        getParam(1).downQuantity = when(exponent){
            is AADD -> {
                if(getParam(0).aadd.min==1.0 && getParam(0).aadd.max==1.0) //Log with 1 not possible. All resulting values allowed
                    Quantity(model.builder.Reals, downQuantity.unit)
                else
                    downQuantity.log(getParam(0).downQuantity)
            }
            is IDD  -> {
                if(getParam(0).idd.min==1L && getParam(0).idd.max==1L) //Log with 1 not possible. All resulting values allowed
                    Quantity(model.builder.Integers)
                else
                    downQuantity.log(getParam(0).downQuantity)
            }
            else -> throw SemanticError("Expect base of type Real or Integer")
        }
    }


    override fun clone(): AstPower {
        val parClone = ArrayList<AstNode>()
        for (p in parameters) parClone.add(p.clone())
        return AstPower(model, parClone)
    }
}
