package BaseEntities.implementation

import BaseEntities.Annotation
import BaseEntities.AnnotatingElement
import BaseEntities.Element
import BaseEntities.Identity
import Parser.QualifiedName
import java.util.*


/**
 * An Annotation is a Relationship between an AnnotatingElement and the Element that
 * is annotated by that AnnotatingElement.
 * Attributes - annotatedElement : Element {redefines target}
 * The Element that is annotated by the annotatingElement of this Annotation.
 * annotatingElement : AnnotatingElement {redefines source}
 * The AnnotatingElement that annotates the annotatedElement of this Annotation.
 * owningAnnotatedElement : Element [0..1] {subsets annotatedElement, redefines owningRelatedElement}
 * The annotatedElement of this Annotation, when it is also its owningRelatedElement.
 */
open class AnnotationImplementation(
    elementId: UUID = UUID.randomUUID(),
    name: String? = null,
    shortName: String? = null,
    sources: MutableList<Identity<Element>> = mutableListOf(),
    targets: MutableList<Identity<Element>> = mutableListOf(),
    ownedElements: MutableList<Identity<Element>> = mutableListOf(),
    owner: Identity<Element> = Identity()
): Annotation, RelationshipImplementation(
    elementId=elementId, name=name, shortName=shortName,
    source = sources, target = targets,
    ownedElements = ownedElements,
    owner = owner
), kotlin.Annotation {
    override val annotatingElements
        get() = target

    override val annotations
        get() = source

    override fun addAnnotation(element: Identity<AnnotatingElement>) {
        source.add(element)
    }

    override fun addAnnotatedElement(qualifiedName: QualifiedName) {
        target.add(Identity(qualifiedName))
    }

    override fun clone(): Annotation {
        return AnnotationImplementation(
            name=name,
            shortName=shortName,
            targets = Identity.copyOfIdentityList(annotatingElements),
            sources = Identity.copyOfIdentityList(annotations),
            ownedElements = Identity.copyOfIdentityList(ownedElements),
            owner = Identity(owner)
        )
    }

    override fun toString(): String {
        return "Annotation { " +
                "name='$name', " +
                (if (shortName != null) "shortName=' $shortName" else "") +
                "#sources=${source.size}, " +
                "#targets=${target.size}, " +
                "id='${elementId}' }"
    }
}