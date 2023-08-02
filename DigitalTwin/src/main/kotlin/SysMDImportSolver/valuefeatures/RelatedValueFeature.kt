package SysMDImportSolver.valuefeatures

import BaseEntities.ValueFeature
import BaseEntities.implementation.ValueFeatureImplementation
import BaseEntities.Feature
import java.util.*

class RelatedValueFeature(
    id: UUID = UUID.randomUUID(),
    name: String? = null,
    shortName: String? = null,
    direction: Feature.FeatureDirectionKind = Feature.FeatureDirectionKind.IN,
    dependency: String,
    valueSpecs: MutableList<Any?>,
    val createdBy: ValueFeature,
    val relatedIndex: Int,
    val reason: String = ""
): ValueFeatureImplementation(
    elementId=id, name=name, shortName = shortName,
    direction = direction, dependency = dependency, valueSpecs = valueSpecs) {

    init { isTransient = true }

    override fun clone(): RelatedValueFeature {
        return RelatedValueFeature(
            name = name,
            shortName = shortName,
            dependency = dependency.plus(""),
            valueSpecs = valueSpecs,
            createdBy = createdBy,
            relatedIndex = relatedIndex,
            reason = reason.plus("")
        ).also {
            it.vectorQuantity = vectorQuantity
            it.oldVectorQuantity = oldVectorQuantity
            it.unitSpec = unitSpec.plus("")
            it.owner.ref = owner.ref
            it.updated = updated
            it.isTransient = isTransient
        }
    }
}
