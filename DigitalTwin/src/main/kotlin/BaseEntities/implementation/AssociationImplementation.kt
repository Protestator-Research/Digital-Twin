package BaseEntities.implementation

import BaseEntities.Association
import BaseEntities.Element
import BaseEntities.Identity
import Parser.Identification

import java.util.*


/**
 * Association inherits from Relationship and Classifier (Classifiable)
 */
class AssociationImplementation(
    elementId: UUID = UUID.randomUUID(),
    name: String? = null,
    shortName: String? = null,
    sources: MutableList<Identity<Element>> = mutableListOf(),
    targets: MutableList<Identity<Element>> = mutableListOf(),
    ownedElements: MutableList<Identity<Element>> = mutableListOf(),
    owner: Identity<Element> = Identity()
): Association, RelationshipImplementation(
    elementId=elementId, name=name, shortName=shortName, source=sources, target=targets, ownedElements=ownedElements, owner=owner
) {
    constructor(
        identification: Identification,
        sources: MutableList<Identity<Element>> = mutableListOf(),
        targets: MutableList<Identity<Element>> = mutableListOf()
    ) : this(name=identification.name, shortName = identification.shortName, sources = sources, targets = targets)

    override fun resolveNames(): Boolean {
        var updated = super.resolveNames()
        updated = superclass?.resolveIdentity(owningNamespace!!) == true or updated
        return updated
    }

    override fun toString(): String {
        return "Association { " +
                (if (name != null) "name='$name', " else "") +
                (if (shortName != null) "shortName='$shortName', " else "") +
                "#sources=${source.size}, " +
                "#targets=${target.size}, " +
                "id='${elementId}' }"
    }

    override fun clone(): AssociationImplementation {
        return AssociationImplementation(
            name = name,
            shortName = shortName,
            ownedElements = Identity.copyOfIdentityList(ownedElements),
            owner = Identity(owner),
            sources = Identity.copyOfIdentityList(source),
            targets = Identity.copyOfIdentityList(target)
        )
    }
}