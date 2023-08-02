package ImportedSysMDServices

import ImportedSysMDExceptions.SysMDError
import SysMDCopyAst.checkEvent



/**
 * Most simple constraint propagation; just until Jack is finished.
 * Requires calling initialize if ast is not yet initialized, e.g. if it comes from database or REST.
 * Or as benchmark to demonstrate benefit of his method.
 */
fun AgilaSession.propagate() {
    try {
        if (repo.schedule.isEmpty())
            initialize()
//        if (!dSolver.isInitialized())
//            dSolver.initialize(this)
        var modelIsStable: Boolean
        repo.schedule.forEach {
            it.stable = false
            it.updated = false
        }
        status.numberOfPropagateIterations = 1
        do {
            modelIsStable = true
            repo.schedule.forEach { value ->
                try {
                    if (value.ofClass?.ref != repo.stringType) {
                        modelIsStable = modelIsStable and value.stable
                        if (value.ast != null) {
                            value.ast!!.evalUpRec()
                            value.ast!!.evalDownRec()
                            value.checkEvent()     // Sets property.stable to false, if changed in iteration step, and property.updated iff changed in propagate call
                            if (value.ofClass?.ref?.isSubclassOf(repo.booleanType) == true && value.updated) {
//                                dSolver.update(value)
                            }
                        } else
                            value.stable = true
                    }
                } catch (e: Exception) {
                    value.stable = true
                    report(value, e.message ?: "(no message)", e)
                }
                status.numberOfPropagateIterations += 1
            }
//            dSolver.advanceState()
//            dSolver.assertConstraints()
        } while (!modelIsStable && status.numberOfPropagateIterations < (repo.schedule.size+1) * 5)
        if (status.numberOfPropagateIterations >= (repo.schedule.size+1) * 100)
            reportInfo(global, "Number of constraint propagation iterations exceeded. Increase it if needed.")

        // Copy updated entries into status map, check consistency.
        repo.schedule.forEach {
            if (it.updated)
                status.updates[it.elementId] = it.valueStr
        }
    } catch (error: Exception) {
        report(SysMDError("During propagation: ", cause = error))
    }
}