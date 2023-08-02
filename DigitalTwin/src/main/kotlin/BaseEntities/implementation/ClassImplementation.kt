package BaseEntities.implementation

import BaseEntities.Element
import BaseEntities.Identity
import BaseEntities.Class
import java.util.*

class ClassImplementation(
    elementId: UUID = UUID.randomUUID(),
    name: String? = null,
    shortName: String? = null,
    ownedElements: MutableList<Identity<Element>> = mutableListOf(),
    owner: Identity<Element> = Identity()
): Class, ClassifierImplementation(
    elementId=elementId, name=name, shortName = shortName, ownedElements = ownedElements, owner = owner
) {
    override fun toString(): String {
        return "Class {" +
                (if (name != null) "name='$name', " else "") +
                (if (shortName != null) "shortNamse='$shortName', " else "") +
                "superclass='$superclass', " +
                "imports='$imports', " +
                "id='${elementId}...'}"
    }

    override fun clone(): Class {
        return ClassImplementation(
            name=name,
            shortName=shortName,
            ownedElements=Identity.copyOfIdentityList(ownedElements),
            owner=Identity(owner)
        ).also {
            it.model = model
            it.updated = updated
        }
    }

}