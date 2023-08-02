package SysMDImportSolver.analyzer

import BaseEntities.ValueFeature
import BaseEntities.implementation.TypeImplementation
import Parser.Semantics


interface SemanticAnalyzerIF: AnalyzerIF {
    val semantics: Semantics

    override fun updateProperty(updatedProperty: ValueFeature)

    fun getAnnotations(property: ValueFeature): List<Annotation>

    fun getClassifier(property: ValueFeature): TypeImplementation
}