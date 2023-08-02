package BaseEntities.implementation

import BaseEntities.Comment
import BaseEntities.Element
import BaseEntities.Identity
import Parser.SimpleName
import java.util.*


open class CommentImplementation(
    elementId: UUID = UUID.randomUUID(),
    name: SimpleName?,
    shortName: SimpleName?,
    ownedElements: MutableList<Identity<Element>> = mutableListOf(),
    owner: Identity<Element> = Identity(),
    body: String,
    override var locale: String? = null
): Comment, AnnotatingElementImplementation(
    elementId = elementId, name = name, shortName = shortName,
    owner=owner, ownedElements = ownedElements,
    body = body) {

    override fun clone(): Comment {
        return CommentImplementation(
            name=name, shortName = shortName,
            ownedElements = Identity.copyOfIdentityList(ownedElements),
            owner = Identity(owner),
            body = body,
            locale = locale
        ).also {
            it.model = model
        }
    }

    override fun toString(): String =
        "Comment {id='${elementId}', name='$name', body=$body}"
}
