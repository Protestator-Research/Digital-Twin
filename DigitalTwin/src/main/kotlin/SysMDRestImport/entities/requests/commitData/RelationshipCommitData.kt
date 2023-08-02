package SysMDRestImport.entities.requests.commitData

import BaseEntities.Feature
import SysMDRestImport.entities.Element
import SysMDRestImport.entities.Identity
import com.fasterxml.jackson.annotation.JsonTypeName
import java.util.UUID
import kotlin.collections.ArrayList

@JsonTypeName(CommitType.RELATIONSHIP)
class RelationshipCommitData(
    id: UUID,
    type: String,       // mandatory type of the metamodel as annotation
    aliasIds: List<String> = ArrayList(),
    declaredName: String? = null,
    declaredShortName: String? = null,
    documentation: Identity? = null,
    elementId: String? = null,
    var isImplied: Boolean? = null,
    isImpliedIncluded: Boolean? = null,
    isLibraryElement: Boolean? = null,
    name: String? = null,
    ownedAnnotation: List<Identity> = ArrayList(),
    ownedElement: List<Identity> = ArrayList(),
    var ownedRelatedElement: Identity? = null,
    ownedRelationship: List<Identity> = ArrayList(),
    owner: Identity? = null,
    owningMembership: Identity? = null,
    owningNamespace: Identity? = null,
    var owningRelatedElement: Identity? = null,
    owningRelationship: Identity? = null,
    qualifiedName: String? = null,
    var relatedElement: Identity? = null,
    shortName: String? = null,
    var source: List<Identity> = ArrayList(),
    var target: List<Identity> = ArrayList(),
    textualRepresentation: List<Identity> = ArrayList(),
    body: String? = null,
    direction: Feature.FeatureDirectionKind? = null,
    language: String? = null,
    valueSpecs: MutableList<Any?> = mutableListOf(),
    unitSpec: String? = null,
    dependency: String? = null,
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
    textualRepresentation = textualRepresentation,
    body = body,
    direction = direction,
    language = language,
    valueSpecs = valueSpecs,
    unitSpec = unitSpec,
    dependency = dependency
) {
    override fun toElement(): Element {
        return Element(
//            id = id!!,
//            type = type,
//            name = name,
//            shortName = shortName,
//            ownedElements = ownedElement.map { id }.toMutableList(),
//            owner = owner,
//            direction = direction,
//            importedMemberName = owningMembership?.toString(),
//            importedNamespace = owningNamespace?.toString(),
//            valueSpec = valueSpec,
//            unitSpec = unitSpec,
//            dependency = dependency,
//            language = language,
//            body = body,
//            source = source.map { it.toString() }.toMutableList(),
//            target = target.map { it.toString() }.toMutableList()
        )
    }
    override fun copyFromElement(element: Element): RelationshipCommitData {
        return RelationshipCommitData(
            id = element.id,
            type = element.type,
            name = element.name,
            shortName = element.shortName,
            ownedElement = element.ownedElements.map { Identity(id=it) }.toMutableList(),
            owner = Identity(id=element.owner),
            direction = element.direction,
            //TODO to Fix
//            valueSpecs = element.valueSpecs !!,
            unitSpec = element.unitSpec,
            dependency = element.dependency,
            language = element.language,
            body = element.body,
            source = element.source.map { Identity(id=UUID.fromString(it)) }.toMutableList(),
            target = element.target.map { Identity(id=UUID.fromString(it)) }.toMutableList(),
        )
    }
}