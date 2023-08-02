package SysMDCopyAst.functions

import ImportedSysMDExceptions.SemanticError
import ImportedSysMDQuantities.VectorQuantity
import ImportedSysMDServices.AgilaSession
import SysMDCopyAst.AstNode
import com.github.tukcps.jaadd.AADD
import com.github.tukcps.jaadd.IDD
import com.github.tukcps.jaadd.values.IntegerRange

/**
 * Predefined functions: ceil
 * Computes ceiling function
 */
internal class AstCeil(model: AgilaSession, args: ArrayList<AstNode>) :
    AstFunction("ceil", model, 1, args) {

    override fun initialize() {
        upQuantity = when (getParam(0).upQuantity.values[0]) {
            is AADD -> VectorQuantity(mutableListOf(model.builder.Reals), "?")
            is IDD -> VectorQuantity(mutableListOf(model.builder.Integers))
            else -> throw SemanticError("ceil only takes Real or Integer parameter")
        }
        evalUp()
        downQuantity = upQuantity.clone()
    }

    override fun evalUp() {
        upQuantity = when(getParam(0).upQuantity.values[0]){
            is AADD -> getParam(0).upQuantity.ceil()
            is IDD -> {
                val results = mutableListOf<IDD>()
                getParam(0).upQuantity.values.forEach {
                    var resultMin = it.asIdd().min
                    val resultMax = it.asIdd().max
                    if(resultMin < resultMax) //otherwise there is nothing to do
                        resultMin = IntegerRange().plusOverflowDetection(resultMin,1) //
                    results.add(model.builder.rangeIDD(resultMin,resultMax))
                }
                VectorQuantity(results)
            }
            else -> throw SemanticError("ceil only takes Real or Integer parameter")
        }
    }

    override fun evalDown() {
        getParam(0).downQuantity  = when(getParam(0).upQuantity.values[0]){
            is AADD -> {
                val results = mutableListOf<AADD>()
                downQuantity.values.forEach {
                    results.add((it as AADD).invCeil())
                }
                VectorQuantity(results, downQuantity.unit)
            }
            is IDD -> {
                val results = mutableListOf<IDD>()
                // avoid overflow during -1 calculation
                downQuantity.values.forEach {
                    results.add(model.builder.rangeIDD(IntegerRange().minusOverflowDetection(it.asIdd().min,1), it.asIdd().max))
                }
                VectorQuantity(results)
            }
            else -> throw SemanticError("floor only takes Real or Integer parameter")
        }
    }

    override fun toExpressionString() = "ceil(${getParam(0).toExpressionString()})"

    override fun clone(): AstCeil {
        val parClone = ArrayList<AstNode>()
        for (p in parameters) parClone.add(p.clone())
        return AstCeil(model, parClone)
    }
}
