package SysMDImportSolver.analyzer

import BaseEntities.ValueFeature
import ImportedSysMDServices.AgilaSession


interface AnalyzerIF {
    val model: AgilaSession

    fun updateProperty(updatedProperty: ValueFeature)

    //fun conditionInAPath(conditionIndex: Int, property: Property): Boolean

    //fun conditionInAllPaths(conditionIndex: Int, property: Property): Boolean

    //fun getAllocationFromAllPaths(index: Int, property: Property): DD
}