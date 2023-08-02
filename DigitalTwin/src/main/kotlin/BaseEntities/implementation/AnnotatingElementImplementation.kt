package BaseEntities.implementation

import BaseEntities.AnnotatingElement
import BaseEntities.Element
import BaseEntities.Identity
import Parser.Identification

import java.util.*


/**
 * Annotating element following 7.2.3.3.1, 2 KerMLv2
 * - it is an Element
 * - Can own further annotating elements.
 * - The annotated element is defined by relationship Annotation, or else is the owning namespace.
 * An AnnotatingElement is an Element that provides additional description of or metadata on some other Element.
 * An AnnotatingElement is attached to its annotatedElement by an Annotation Relationship
 */
open class AnnotatingElementImplementation(
    elementId: UUID = UUID.randomUUID(),
    name: String? = null,
    shortName: String? = null,
    ownedElements: MutableList<Identity<Element>> = mutableListOf(),
    owner: Identity<Element> = Identity(),
    override var body: String = "",
): BaseEntities.AnnotatingElement, ElementImplementation(
    elementId=elementId, name= name, shortName = shortName,
    owner = owner, ownedElements = ownedElements
) {

    constructor(identification: Identification, body: String):
            this(shortName=identification.shortName, name=identification.name, body = body)

    override fun resolveNames() = false

    override fun clone(): BaseEntities.AnnotatingElement {
        return AnnotatingElementImplementation(
            name = name,
            shortName = shortName,
            ownedElements = Identity.copyOfIdentityList(ownedElements),
            owner = Identity(owner),
            body = body)
    }
    override fun toString(): String =
        "AnnotatingElement { name='$name', shortName='${shortName}', body=$body, id='${elementId}'}"

    /**
     * The annotated element is defined by an annotation (relationship) or, if no annotation
     * is available, is the owning element.
     */
    override fun annotatedElement(): Nothing = TODO()
}
