package SysMDImportSolver.analyzer

import BaseEntities.ValueFeature
import BaseEntities.implementation.TypeImplementation
import ImportedSysMDServices.AgilaSession
import Parser.Semantics


class SemanticAnalyzer(override val model: AgilaSession): SemanticAnalyzerIF {
    override val semantics = Semantics(model)

    override fun updateProperty(updatedProperty: ValueFeature) {
        //TODO!
    }

    override fun getClassifier(property: ValueFeature): TypeImplementation {
        //TODO
        return TypeImplementation(name = semantics.global.name, shortName = semantics.global.shortName)
    }

    override fun getAnnotations(property: ValueFeature): List<Annotation> {
        //TODO
        return listOf()
    }
}