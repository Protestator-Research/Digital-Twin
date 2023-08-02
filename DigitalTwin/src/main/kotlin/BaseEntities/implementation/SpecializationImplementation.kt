package BaseEntities.implementation


import BaseEntities.Element
import BaseEntities.Identity
import BaseEntities.Specialization
import BaseEntities.Type
import java.util.*


@Suppress("UNCHECKED_CAST")
open class SpecializationImplementation(
    elementId: UUID = UUID.randomUUID(),
    owner: Identity<Element> = Identity(),
    subclass: Identity<Type>? = null,
    superclass: Identity<Type>? = null
): Specialization, RelationshipImplementation(
    elementId = elementId,
    owner = owner,
    source = if (subclass != null) mutableListOf(subclass) else mutableListOf(),
    target = if (superclass != null) mutableListOf(superclass) else mutableListOf()
) {
    constructor(subclass: Type, superclass: String):
            this(owner=Identity(), subclass=Identity(ref=subclass), superclass=Identity(str=superclass))

    constructor(subclass: Type, superclass: Type):
            this(owner=Identity(), subclass=Identity(ref=subclass), superclass=Identity(ref =superclass)) {
        target[0].str = target[0].ref!!.qualifiedName
    }

    constructor() : this(owner=Identity(), subclass=null, superclass = null)

    override var superclass: Identity<Type>
        get() = target[0] as Identity<Type>
        set(value) { target = mutableListOf(value) }

    override var subclass: Identity<Type>
        get() = source[0] as Identity<Type>
        set(value) { source= mutableListOf(value) }

    override fun toString(): String {
        return "Specialization(${source[0]}, ${target[0]})"
    }

    override fun clone(): Specialization {
        return SpecializationImplementation(
            owner=Identity(owner),
            subclass = Identity(source[0] as Identity<Type>),
            superclass = if (target.firstOrNull() == null) Identity() else Identity(target.firstOrNull() as Identity<Type>)
        ).also { klon ->
            klon.isTransient = isTransient
            klon.model = model
        }
    }

    override fun updateFrom(template: Element) {
        require(template is Specialization)
        super.updateFrom(template)
    }


    /**
     * Initialize resolves the QualifiedNames and/or uid and adds references and uid to Elements.
     */
    override fun resolveNames(): Boolean {
        require( model != null )

        // Search all sources & targets.
        source.forEach {
            if (it.resolveIdentity(owningNamespace!!, Identity.RefType.TYPE))
                updated = true
        }
        target.forEach {
            if (it.resolveIdentity(owningNamespace!!, Identity.RefType.TYPE))
                updated = true
        }

        source.forEach { relatedElement ->
            if (relatedElement.ref != null && model!!.repo.sourceOfRelationship[relatedElement.ref!!] == null)
                model!!.repo.sourceOfRelationship[relatedElement.ref!!] = arrayListOf()
        }

        target.forEach { relatedElement ->
            if (relatedElement.ref != null && model!!.repo.targetOfRelationship[relatedElement.ref!!] == null)
                model!!.repo.targetOfRelationship[relatedElement.ref!!] = arrayListOf()
        }

        // Add found sources & targets to hashmap for faster lookup
        source.forEach { relatedElement ->
            if (relatedElement.ref != null) {
                model!!.repo.sourceOfRelationship[relatedElement.ref!!]?.add(this)
            }
        }
        target.forEach { relatedElement ->
            if (relatedElement.ref != null) {
                model!!.repo.targetOfRelationship[relatedElement.ref!!]?.add(this)
            }
        }
        return updated
    }

}