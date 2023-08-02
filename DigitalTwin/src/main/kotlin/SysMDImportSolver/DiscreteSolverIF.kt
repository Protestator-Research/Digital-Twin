package SysMDImportSolver

import BaseEntities.ValueFeature
import ImportedSysMDServices.AgilaSession


interface DiscreteSolverIF {

    fun initialize(model: AgilaSession)

    fun update(scheduledProperties: List<ValueFeature>)

    fun update(updatedProperty: ValueFeature)

    fun advanceState()

    fun returnToLastState()

}