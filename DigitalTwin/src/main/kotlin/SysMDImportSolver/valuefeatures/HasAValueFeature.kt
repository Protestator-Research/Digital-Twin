package SysMDImportSolver.valuefeatures



import BaseEntities.Feature
import BaseEntities.ValueFeature
import BaseEntities.implementation.ValueFeatureImplementation
import com.github.tukcps.jaadd.values.XBool
import java.util.*

class HasAValueFeature(
    id: UUID = UUID.randomUUID(),
    name: String? = null,
    shortName: String? = null,
    direction: Feature.FeatureDirectionKind = Feature.FeatureDirectionKind.IN,
    dependency: String,
    valueSpec: MutableList<Any?>,
    private val createdBy: ValueFeature,
    val reason: String = "",
    private val src: ValueFeature,
    private val dst: ValueFeature
): ValueFeatureImplementation(
    elementId=id, name=name, shortName = shortName,
    direction = direction, dependency = dependency, valueSpecs = valueSpec) {

    init {
        isTransient = true
        require(boolSpecs == XBool.True)
    }

    override fun clone(): HasAValueFeature {
        return HasAValueFeature(
            name = name,
            shortName = shortName,
            dependency = dependency.plus(""),
            valueSpec = valueSpecs,
            createdBy = createdBy,
            reason = reason.plus(""),
            src = src,
            dst = dst
        ).also {
            it.unitSpec = unitSpec.plus("")
            it.owner.ref = owner.ref
            it.updated = updated
            it.isTransient = isTransient
        }
    }

}