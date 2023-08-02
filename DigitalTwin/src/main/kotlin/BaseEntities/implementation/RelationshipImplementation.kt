package BaseEntities.implementation


import BaseEntities.Element
import BaseEntities.Identity
import BaseEntities.Relationship
import ImportedSysMDServices.report
import Parser.SimpleName
import java.util.*


/**
 * Following KerML Specification, 7.2.2.2.2 Relationships;
 * Represents the SysMD relationship source relation and inherits ElementImplementation
 */
open class RelationshipImplementation(
    elementId: UUID = UUID.randomUUID(),
    name: SimpleName? = null,
    shortName: SimpleName? = null,
    ownedElements: MutableList<Identity<Element>> = mutableListOf(),
    owner: Identity<Element> = Identity(),
    final override var source: MutableList<Identity<Element>> = mutableListOf(),
    final override var target: MutableList<Identity<Element>> = mutableListOf()
): Relationship, ElementImplementation(elementId = elementId, name = name, shortName = shortName, owner=owner, ownedElements = ownedElements) {

    /**
     * Initialize resolves the QualifiedNames and/or uid and adds references and uid to Elements.
     */
    override fun resolveNames(): Boolean {
        super.resolveNames()
        require( model != null )
        if ( owningNamespace == null)
            model!!.report(this, "unowned element: $this")

        // Search all sources & targets.
        source.forEach {
            if (it.resolveIdentity(owningNamespace!!))
                updated = true
        }
        target.forEach {
            if (it.resolveIdentity(owningNamespace!!))
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

    override fun clone(): Relationship {
        return RelationshipImplementation(
            name = name,
            shortName = shortName,
            source = Identity.copyOfIdentityList(source),
            target = Identity.copyOfIdentityList(target),
            ownedElements = Identity.copyOfIdentityList(ownedElements),
            owner = Identity(owner)
        ).also{
            it.model = model
        }
    }

    override fun toString(): String {
        return "Relationship { " +
                "name='$name', " +
                "shortName=' $shortName" +
                "#sources=${source.size}, " +
                "#targets=${target.size}, " +
                "id='${elementId}' }"
    }

    override fun updateFrom(template: Element) {
        super.updateFrom(template)
        if (template is Relationship) {
          target = Identity.copyOfIdentityList(template.target)
          source = Identity.copyOfIdentityList(template.source)
        }
    }
}
