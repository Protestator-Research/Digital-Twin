package SysMDImportSolver.analyzer

import BaseEntities.ValueFeature
import ImportedSysMDServices.AgilaSession
import com.github.tukcps.jaadd.DD

interface StructuralAnalyzerIF: AnalyzerIF {
    override val model: AgilaSession

    override fun updateProperty(updatedProperty: ValueFeature) {
        TODO("Not yet implemented")
    }

    fun conditionInAPath(conditionIndex: Int, property: ValueFeature): Boolean {
        TODO("Not yet implemented")
    }

    fun conditionInAllPaths(conditionIndex: Int, property: ValueFeature): Boolean {
        TODO("Not yet implemented")
    }

    fun getAllocationFromAllPaths(index: Int, property: ValueFeature): DD {
        TODO("Not yet implemented")
    }
}