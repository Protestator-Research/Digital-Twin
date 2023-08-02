package BaseEntities


import BaseEntities.implementation.ClassifierImplementation
import BaseEntities.implementation.SpecializationImplementation
import ImportedSysMDServices.AgilaSession
import java.util.*

/**
 * The root of the inheritance tree.
 * Superclass is null.
 */
@Suppress("UNUSED_PARAMETER")
class Anything(
    owner: Identity<Element>,
    model: AgilaSession
): Classifier, ClassifierImplementation(
    elementId = UUID.randomUUID(),
    name = "Any",
    shortName = "Any",
    owner = owner,
) {
    init {
        this.model = model
    }

    override val specialization = SpecializationImplementation(
        owner= Identity(), subclass = Identity(ref=this), superclass = superclass)

    override var superclass: Identity<Type>?
        get() = Identity(str="none", ref=null, id=null)
        set(value) { }  // Cannot be changed.

    override fun resolveNames() = false
    override fun clone(): Anything = this
    override fun toString(): String = "Anything"
}

