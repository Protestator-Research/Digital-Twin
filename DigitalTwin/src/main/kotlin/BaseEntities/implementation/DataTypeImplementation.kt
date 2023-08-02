package BaseEntities.implementation


import BaseEntities.DataType
import BaseEntities.Element
import BaseEntities.Identity
import java.util.*

class DataTypeImplementation(
    elementId: UUID = UUID.randomUUID(),
    name: String? = null,
    shortName: String? = null,
    ownedElements: MutableList<Identity<Element>> = mutableListOf(),
    owner: Identity<Element> = Identity()
): DataType, ClassifierImplementation(elementId, name, shortName, ownedElements, owner) {

    override fun toString(): String {
        return "DataType {" +
            if (name != null) "name='$name', " else "" +
            if (shortName != null) "name='$shortName', " else "" +
            "superclass='$superclass', " +
            "imports='$imports', " +
            "id='${elementId}...'}"
    }

    override fun clone(): DataType {
        return DataTypeImplementation(
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