package BaseEntities.implementation



import BaseEntities.*
import Parser.Identification

import java.util.*


/**
 * A Type (KerML 7.3.3)
 */
open class TypeImplementation(
    elementId: UUID = UUID.randomUUID(),
    name: String? = null,
    shortName: String? = null,
    ownedElements: MutableList<Identity<Element>> = mutableListOf(),
    owner: Identity<Element> = Identity()
): Type, NamespaceImplementation(elementId=elementId, name=name, shortName=shortName, ownedElements=ownedElements, owner=owner) {
    constructor(identification: Identification):
            this(name = identification.name, shortName = identification.shortName)

    override fun clone(): Type {
        return TypeImplementation(
            name = name,
            shortName = shortName,
            ownedElements = Identity.copyOfIdentityList(ownedElements),
            owner = Identity(owner)
        ).also { klon ->
            klon.model = model
            klon.updated = updated
        }
    }

    override fun toString(): String {
        return "Type (" +
                "name='$name', " +
                (if (shortName != null) "shortName=$shortName" else "") +
                "superclass='${getOwnedElementOfType<Specialization>()?.superclass}', " +
                "imports='$imports', " +
                "id='${elementId}...')"
    }

    /**
     * Does name resolution and, if successful, sets updated=true
     */
    override fun resolveNames(): Boolean {
        updated = super<Type>.resolveNames() or updated
        updated = super<NamespaceImplementation>.resolveNames() or updated
        return updated
    }

    override fun updateFrom(template: Element) {
        if (template is TypeImplementation) {
            super.updateFrom(template)
        }
    }
}
