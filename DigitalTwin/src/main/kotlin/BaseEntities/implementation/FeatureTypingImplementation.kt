package BaseEntities.implementation

import BaseEntities.*
import java.util.*

/**
 * Feature typing relationship.
 */
class FeatureTypingImplementation(
    elementId: UUID = UUID.randomUUID(),
    owner: Identity<Element> = Identity(),
    typedFeature: Identity<Feature> = Identity(),
    type:  Identity<Type> = Identity(),
): FeatureTyping, SpecializationImplementation(
    elementId = elementId,
    owner = owner,
    subclass = typedFeature,
    superclass = type
) {
    override val owningFeature: Identity<Feature>
        get() = owner as Identity<Feature>

    override val type: Identity<Type>
        get() = superclass

    override val typedFeature: Identity<Feature>
        get() = owner as Identity<Feature>


    override fun clone(): FeatureTyping {
        return FeatureTypingImplementation(
            owner=Identity(owner),
            typedFeature = Identity(source[0] as Identity<Feature>),
            type = if (target.firstOrNull() == null) Identity() else Identity(target.firstOrNull() as Identity<Type>)
        ).also { klon ->
            klon.isTransient = isTransient
            klon.model = model
        }
    }
}