package SysMDCopyAst

import BaseEntities.ValueFeature
import com.github.tukcps.jaadd.*
import kotlin.math.absoluteValue


/**
 * Checks if a property value has been restricted to a smaller interval, and if so, sets the marker
 * stable to false, otherwise to true.
 */
fun ValueFeature.checkEvent() {
    stable = vectorQuantity.values.indices.all {
        when (vectorQuantity.values[0]) {
        is AADD -> {
                // We allow here some percent of slack to become robust against potential FP rounding errors.
                val lb = vectorQuantity.values[it].asAadd().max - 0.00001 * vectorQuantity.values[it].asAadd().min.absoluteValue
                val ub = vectorQuantity.values[it].asAadd().max + 0.00001 * vectorQuantity.values[it].asAadd().min.absoluteValue
                val oldValue = if(oldVectorQuantity!!.values.size>1) oldVectorQuantity!!.values[it] else oldVectorQuantity!!.value
                ((oldValue as AADD) in lb..ub || (lb >= ub))
                        && (vectorQuantity.unit == oldVectorQuantity!!.unit)
                        || (lb.isNaN() && ub.isNaN())
        }
        is IDD -> {
            val lb = vectorQuantity.values[it].asIdd().getRange().min
            val ub = vectorQuantity.values[it].asIdd().getRange().max
            if (oldVectorQuantity != null) {
                val oldValue = if (oldVectorQuantity!!.values.size > 1) oldVectorQuantity!!.values[it] else oldVectorQuantity!!.value
                (oldValue as IDD).getRange().min in lb..ub && oldValue.getRange().max in lb..ub || (lb >= ub)
            }else
                false
        }
        is BDD -> {
            // Only covers really serious changes. More by Alex?
            val oldValue = if(oldVectorQuantity!!.values.size>1) oldVectorQuantity!!.values[it] else oldVectorQuantity!!.value
            when (vectorQuantity.values[it]) {
                vectorQuantity.values[it].builder.True -> {
                    oldValue == vectorQuantity.values[it]
                }
                vectorQuantity.values[it].builder.False -> {
                    oldValue == vectorQuantity.values[it]
                }
                else -> true
            }
        }
        else -> throw DDException("checkUpdated called with wrong type")
    }}
    if (!stable)
        updated = true

    oldVectorQuantity = vectorQuantity.clone()
}