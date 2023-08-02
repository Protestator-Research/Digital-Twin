package BaseEntities.implementation


import BaseEntities.Documentation
import BaseEntities.Element
import BaseEntities.Identity
import Parser.SimpleName
import java.util.*


/**
 * Documentation is an Annotation whose annotatingElement is a Comment that provides
 * documentation of the annotatedElement. Documentation is always an ownedRelationship
 * of its annotatedElement.
 * @param identification optional name
 * @param body the documentation as a string
 */
class DocumentationImplementation(
    elementId: UUID = UUID.randomUUID(),
    name: String? = null,
    shortName: SimpleName? = null,
    ownedElements: MutableList<Identity<Element>> = mutableListOf(),
    owner: Identity<Element> = Identity(),
    body: String
): Documentation, CommentImplementation(
    elementId=elementId, name=name, shortName = shortName,
    ownedElements=ownedElements, owner=owner, body = body) {

    override fun clone(): Documentation {
        return DocumentationImplementation(
            name=name,
            shortName=shortName,
            ownedElements=Identity.copyOfIdentityList(ownedElements),
            owner=Identity(owner),
            body=body).also {
                model=it.model
        }
    }
}