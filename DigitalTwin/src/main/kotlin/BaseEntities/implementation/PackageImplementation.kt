package BaseEntities.implementation

import BaseEntities.Element
import BaseEntities.Identity
import BaseEntities.Package
import Parser.SimpleName
import java.util.*


class PackageImplementation(
    elementId: UUID = UUID.randomUUID(),
    name: SimpleName? = null,
    shortName: SimpleName? = null,
    ownedElements: MutableList<Identity<Element>> = mutableListOf(),
    owner: Identity<Element> = Identity()
): Package, NamespaceImplementation(elementId=elementId, name=name, shortName=shortName, ownedElements=ownedElements, owner=owner)
{

    override fun clone(): PackageImplementation {
        return PackageImplementation(
            name = name,
            shortName = shortName,
            owner = Identity(owner),
            ownedElements = Identity.copyOfIdentityList(ownedElements)
        ).also { klon ->
            klon.model = model
        }
    }

    override fun toString(): String {
        return "Package {" +
                "name='$name', " +
                (if(shortName != null) "shortName='$shortName', " else "") +
                "owner='${owner.ref?.qualifiedName}', " +
                "#owned=${ownedElements.size}, " +
                "#imports=${imports.size}, +" +
                "id='${elementId}')"
    }

    override fun resolveNames(): Boolean {
        if (name == "Global") {
            importNamespace(model!!, "ScalarValues")
            importNamespace(model!!, "ISO26262")
        }
        return super.resolveNames()
    }
}
