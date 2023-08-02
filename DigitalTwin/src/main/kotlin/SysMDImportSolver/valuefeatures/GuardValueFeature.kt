package SysMDImportSolver.valuefeatures


import BaseEntities.Feature
import BaseEntities.ValueFeature
import BaseEntities.implementation.ValueFeatureImplementation
import java.util.*

class GuardValueFeature(
    id: UUID = UUID.randomUUID(),
    name: String? = null,
    shortName: String? = null,
    direction: Feature.FeatureDirectionKind = Feature.FeatureDirectionKind.IN,
    dependency: String,
    valueSpecs: MutableList<Any?>,
    val createdBy: ValueFeature,
    paths: HashMap<Int, Boolean>,
    val reason: String = ""
): ValueFeatureImplementation(
    elementId=id, name=name, shortName = shortName,
    direction = direction, dependency = dependency, valueSpecs = valueSpecs), Cloneable {
    //TODO!
    init {
        isTransient = true
    }

    val pathsToInfeasibility = paths

    override fun clone(): GuardValueFeature {
        return GuardValueFeature(
            name = name,
            shortName = shortName,
            dependency = dependency.plus(""),
            valueSpecs = valueSpecs,
            createdBy = createdBy,
            paths = HashMap(pathsToInfeasibility),
            reason = reason.plus("")
        ).also {
            it.unitSpec = unitSpec.plus("")
            it.owner.ref = owner.ref
            it.updated = updated
            it.isTransient = isTransient
        }
    }
}
