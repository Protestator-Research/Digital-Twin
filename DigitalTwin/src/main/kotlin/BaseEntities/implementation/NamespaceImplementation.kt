package BaseEntities.implementation


import BaseEntities.Element
import BaseEntities.Identity
import BaseEntities.Namespace
import java.util.*


/**
 * KerML foresees a namespace that is an element.
 * "Namespaces can assign unique names to Namespace members, but support multiple aliases per Element.
 * They also support Import of Elements from other Namespaces, enabling an Element to have
 * a different name when imported."
 */
open class NamespaceImplementation(
    elementId: UUID = UUID.randomUUID(),
    name: String? = null,
    shortName: String? = null,
    ownedElements: MutableList<Identity<Element>> = mutableListOf(),
    owner: Identity<Element> = Identity(),
): ElementImplementation(elementId=elementId, name=name, shortName=shortName, ownedElements=ownedElements, owner=owner), Namespace {


    override fun toString(): String {
        return "Namespace {id='$elementId', " +
                "name='$name', " +
                "imports='$imports'}"
    }

    override fun resolveNames(): Boolean {
        updated = super.resolveNames() or updated
        return updated
    }

    override fun clone(): Namespace {
        return NamespaceImplementation(
            name=name,
            shortName = shortName,
            owner = Identity(owner),
            ownedElements = Identity.copyOfIdentityList(ownedElements)
        ).also { klon ->
            klon.model = model
            klon.isTransient = isTransient
        }
    }
}