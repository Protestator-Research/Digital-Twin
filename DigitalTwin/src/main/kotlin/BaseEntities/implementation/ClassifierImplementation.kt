package BaseEntities.implementation

import BaseEntities.Classifier
import BaseEntities.Element
import BaseEntities.Identity
import Parser.Identification
import java.util.*

open class ClassifierImplementation(
    elementId: UUID = UUID.randomUUID(),
    name: String? = null,
    shortName: String? = null,
    ownedElements: MutableList<Identity<Element>> = mutableListOf(),
    owner: Identity<Element> = Identity()
): Classifier, TypeImplementation(
    elementId=elementId, name=name, shortName = shortName,
    ownedElements = ownedElements, owner = owner
) {

    constructor(identification: Identification): this(shortName = identification.shortName, name = identification.name)

    override fun toString(): String {
        return "Classifier {" +
                (if (name != null) "name='$name', " else "") +
                (if (shortName != null) "name='$shortName', " else "") +
                "superclass='$superclass', " +
                "imports='$imports', " +
                "id='${elementId}...'}"
    }

    override fun clone(): Classifier {
        return ClassifierImplementation(
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
