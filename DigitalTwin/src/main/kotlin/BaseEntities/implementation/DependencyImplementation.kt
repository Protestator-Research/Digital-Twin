package BaseEntities.implementation


import BaseEntities.Dependency
import BaseEntities.Element
import BaseEntities.Identity
import java.util.*

class DependencyImplementation(
    id: UUID = UUID.randomUUID(),
    name: String? = null,
    shortName: String? = null,
    clientDependency: MutableList<Identity<Element>> = mutableListOf(),
    supplierDependency: MutableList<Identity<Element>> = mutableListOf(),
    ownedElements: MutableList<Identity<Element>> = mutableListOf(),
    owner: Identity<Element> = Identity()
): Dependency, RelationshipImplementation(
    elementId=id, name=name, shortName=shortName, ownedElements=ownedElements, owner=owner
) {
    override var clientDependency: MutableList<Identity<Element>> = mutableListOf()
    override var supplierDependency: MutableList<Identity<Element>> = mutableListOf()

    init {
        source.addAll(clientDependency)
        target.addAll(supplierDependency)
        clientDependency.addAll(clientDependency)
        supplierDependency.addAll(supplierDependency)
    }
}