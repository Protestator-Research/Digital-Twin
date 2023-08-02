package SysMDRestImport.entities.requests.commitData

import SysMDRestImport.entities.Element
import SysMDRestImport.entities.Identity
import com.fasterxml.jackson.annotation.JsonTypeName
import java.util.*
import kotlin.collections.ArrayList

@JsonTypeName(CommitType.ELEMENT)
class ElementCommitData(
    id: UUID,
    type: String,       // mandatory type of the metamodel as annotation
    aliasIds: List<String> = ArrayList(),
    declaredName: String? = null,
    declaredShortName: String? = null,
    documentation: Identity? = null,
    elementId: String? = null,
    isImpliedIncluded: Boolean? = null,
    isLibraryElement: Boolean? = null,
    name: String? = null,
    ownedAnnotation: List<Identity> = ArrayList(),
    ownedElement: List<Identity> = ArrayList(),
    ownedRelationship: List<Identity> = ArrayList(),
    owner: Identity? = null,
    owningMembership: Identity? = null,
    owningNamespace: Identity? = null,
    owningRelationship: Identity? = null,
    qualifiedName: String? = null,
    shortName: String? = null,
    textualRepresentation: List<Identity> = ArrayList()
): CommitData(
    id = id,
    type = type,
    aliasIds = aliasIds,
    declaredName = declaredName,
    declaredShortName = declaredShortName,
    documentation = documentation,
    elementId = elementId,
    isImpliedIncluded = isImpliedIncluded,
    isLibraryElement = isLibraryElement,
    name= name,
    ownedAnnotation = ownedAnnotation,
    ownedElement = ownedElement,
    ownedRelationship = ownedRelationship,
    owner = owner,
    owningMembership = owningMembership,
    owningNamespace = owningNamespace,
    owningRelationship = owningRelationship,
    qualifiedName = qualifiedName,
    shortName = shortName,
    textualRepresentation = textualRepresentation
) {
    override fun toElement(): Element {
        return Element(
            id = id,
            type = type,
            name = name,
            shortName = shortName,
            ownedElements = ownedElement.map { it.id !! }.toMutableList(),
            owner = owner?.id,
            direction = direction,
            importedMemberName = owningMembership?.toString(),
            importedNamespace = owningNamespace?.toString(),
            valueSpecs = valueSpecs,
            unitSpec = unitSpec,
            dependency = dependency,
            language = language,
            body = body,
            source = arrayListOf(),
            target = arrayListOf()
        )
    }
    override fun copyFromElement(element: Element): ElementCommitData {
        return ElementCommitData(
            id = element.id,
            type = element.type,
            name = element.name,
            shortName = element.shortName,
            ownedElement = element.ownedElements.map { Identity(id=it) }.toMutableList(),
            owner = Identity(id = element.owner),
        )
    }
}