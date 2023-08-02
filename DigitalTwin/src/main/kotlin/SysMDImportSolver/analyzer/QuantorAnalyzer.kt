package SysMDImportSolver.analyzer

import BaseEntities.ValueFeature
import BaseEntities.implementation.TypeImplementation
import ImportedSysMDServices.AgilaSession


interface QuantorAnalyzer: AnalyzerIF {
    override val model: AgilaSession

    fun forAll(property: ValueFeature, classifier: TypeImplementation): Boolean

    fun oneExists(property: ValueFeature, classifier: TypeImplementation): List<ValueFeature>
}