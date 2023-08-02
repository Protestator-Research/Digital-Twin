package BaseEntities.implementation

import BaseEntities.*
import java.util.*

/**
 * Connector / Binary link
 * connector c : A from f1 to f2;
 */
class ConnectorImplementation(
    id: UUID = UUID.randomUUID(),
    name: String? = null,
    shortName: String? = null,
    ownedElements: MutableList<Identity<Element>> = mutableListOf(),
    owner: Identity<Element> = Identity(),
    override var from: MutableList<Identity<Element>> = mutableListOf(),
    override var to: MutableList<Identity<Element>> = mutableListOf(),
) : Connector, Type, FeatureImplementation(
    elementId = id,
    name = name,
    shortName = shortName,
    ownedElements = ownedElements,
    owner = owner)
{
    override var source: MutableList<Identity<Element>>
        get() = from
        set(value) {from = value}

    override var target: MutableList<Identity<Element>>
        get() = to
        set(value) {to = value}


    override fun resolveNames(): Boolean {
        updated = updated or super<FeatureImplementation>.resolveNames()
        return updated
    }

    override fun clone(): ConnectorImplementation {
        return ConnectorImplementation(
            name = name, shortName = shortName,
            ownedElements = Identity.copyOfIdentityList(ownedElements),
            owner = Identity(owner),
            from = Identity.copyOfIdentityList(from),
            to = Identity.copyOfIdentityList(to),
        ).also { klon ->
            klon.model = model
            klon.direction = direction
            klon.updated = updated
        }
    }

    override fun updateFrom(template: Element) {
        super.updateFrom(template)
        if (template is Relationship) {
            target = Identity.copyOfIdentityList(template.target)
            source = Identity.copyOfIdentityList(template.source)
        }
    }
}
