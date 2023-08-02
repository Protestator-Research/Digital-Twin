package SysMDImportSolver.propagator

import BaseEntities.ValueFeature
import ImportedSysMDServices.AgilaSession


interface PropagatorIF {
    val model: AgilaSession

    fun execute(updatedValueFeature: ValueFeature)
}