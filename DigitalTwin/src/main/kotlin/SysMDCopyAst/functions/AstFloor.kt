package SysMDCopyAst.functions

import ImportedSysMDExceptions.SemanticError
import ImportedSysMDQuantities.VectorQuantity
import ImportedSysMDServices.AgilaSession
import SysMDCopyAst.AstNode
import com.github.tukcps.jaadd.AADD
import com.github.tukcps.jaadd.IDD
import com.github.tukcps.jaadd.values.IntegerRange

/**
 * Predefined functions: floor
 * Computes floor function
 */
internal class AstFloor(model: AgilaSession, args: ArrayList<AstNode>) :
    AstFunction("floor", model, 1, args) {

    override fun initialize() {
        upQuantity = when (getParam(0).upQuantity.values[0]) {
            is AADD -> VectorQuantity(mutableListOf(model.builder.Reals), "?")
            is IDD -> VectorQuantity(mutableListOf(model.builder.Integers))
            else -> throw SemanticError("floor only takes Real or Integer parameter")
        }
        evalUp()
        downQuantity = upQuantity.clone()
    }

    override fun evalUp() {
        upQuantity = when(getParam(0).upQuantity.values[0]){
            is AADD -> getParam(0).upQuantity.floor()
            is IDD -> {
                val results = mutableListOf<IDD>()
                getParam(0).upQuantity.values.forEach {
                    val resultMin = it.asIdd().min
                    var resultMax = it.asIdd().max
                    if (resultMin < resultMax)
                        resultMax = IntegerRange().minusOverflowDetection(resultMax, 1) //only reduce maximum border by one if not infinity
                    results.add(model.builder.rangeIDD(resultMin,resultMax))
                }
                VectorQuantity(results)
            }
            else -> throw SemanticError("floor only takes Real or Integer parameter")
        }
    }

    override fun evalDown() {
        getParam(0).downQuantity  = when(getParam(0).upQuantity.values[0]){
            is AADD -> {
                val results = mutableListOf<AADD>()
                downQuantity.values.forEach {
                    results.add((it as AADD).invFloor())
                }
                VectorQuantity(results, downQuantity.unit)
            }
            is IDD -> {
                val results = mutableListOf<IDD>()
                // avoid overflow during -1 calculation
                downQuantity.values.forEach {
                    results.add(model.builder.rangeIDD(it.asIdd().min, IntegerRange().plusOverflowDetection(it.asIdd().max,1)))
                }
                VectorQuantity(results)
            }
            else -> throw SemanticError("floor only takes Real or Integer parameter")
        }
    }

    override fun toExpressionString() = "floor(${getParam(0).toExpressionString()})"

    override fun clone(): AstFloor {
        val parClone = ArrayList<AstNode>()
        for (p in parameters) parClone.add(p.clone())
        return AstFloor(model, parClone)
    }
}
