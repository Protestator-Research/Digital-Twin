package SysMDRestImport.entities.requests.commitData;

import BaseEntities.Feature
import SysMDRestImport.entities.Element
import SysMDRestImport.entities.Identity
import com.fasterxml.jackson.annotation.JsonInclude
import com.fasterxml.jackson.annotation.JsonProperty
import com.fasterxml.jackson.annotation.JsonSubTypes
import com.fasterxml.jackson.annotation.JsonTypeInfo

import java.util.UUID

@JsonTypeInfo(use = JsonTypeInfo.Id.NAME, include = JsonTypeInfo.As.PROPERTY, property = "@type", visible = true)
@JsonSubTypes(
        JsonSubTypes.Type(
                value = ElementCommitData::class,
                names = [
                        CommitType.ELEMENT,
                        CommitType.PACKAGE,
                        CommitType.NAMESPACE_IMPORT,
                        CommitType.MEMBERSHIP_IMPORT,
                        CommitType.MULTIPLICITY,
                        CommitType.SPECIALIZATION,
                        CommitType.CLASSIFIER,
                        CommitType.CLASS,
                        CommitType.TYPE,
                        CommitType.FEATURE,
                        CommitType.NAMESPACE,
                        CommitType.DOCUMENTATION,
                        CommitType.ANNOTATING_ELEMENT,
                        CommitType.TEXTUAL_REPRESENTATION,
                        CommitType.COMMENT,
                        CommitType.ASSOCIATION,
                ]
        ),
        JsonSubTypes.Type(
                value = RelationshipCommitData::class,
                names = [
                        CommitType.RELATIONSHIP,
                        CommitType.ANNOTATION,
                        CommitType.FEATURE_VALUE,
                ]
        )
)

abstract class CommitData(
        id: UUID,
        type: String,       // mandatory type of the metamodel as annotation
        @JsonInclude(JsonInclude.Include.NON_EMPTY)
        var aliasIds: List<String> = ArrayList(),
        @JsonInclude(JsonInclude.Include.NON_NULL)
        var declaredName: String? = null,
        @JsonInclude(JsonInclude.Include.NON_NULL)
        var declaredShortName: String? = null,
        @JsonInclude(JsonInclude.Include.NON_NULL)
        var documentation: Identity? = null,
        @JsonInclude(JsonInclude.Include.NON_NULL)
        var elementId: String? = null,
        @JsonInclude(JsonInclude.Include.NON_NULL)
        var isImpliedIncluded: Boolean? = null,
        @JsonInclude(JsonInclude.Include.NON_NULL)
        var isLibraryElement: Boolean? = null,
        @JsonInclude(JsonInclude.Include.NON_NULL)
        var name: String? = null,
        @JsonInclude(JsonInclude.Include.NON_EMPTY)
        var ownedAnnotation: List<Identity> = ArrayList(),
        @JsonInclude(JsonInclude.Include.NON_EMPTY)
        var ownedElement: List<Identity> = ArrayList(),
        @JsonInclude(JsonInclude.Include.NON_EMPTY)
        var ownedRelationship: List<Identity> = ArrayList(),
        @JsonInclude(JsonInclude.Include.NON_NULL)
        var owner: Identity? = null,
        @JsonInclude(JsonInclude.Include.NON_NULL)
        var owningMembership: Identity? = null,
        @JsonInclude(JsonInclude.Include.NON_NULL)
        var owningNamespace: Identity? = null,
        @JsonInclude(JsonInclude.Include.NON_NULL)
        var owningRelationship: Identity? = null,
        @JsonInclude(JsonInclude.Include.NON_NULL)
        var qualifiedName: String? = null,
        @JsonInclude(JsonInclude.Include.NON_NULL)
        var shortName: String? = null,
        @JsonInclude(JsonInclude.Include.NON_EMPTY)
        var textualRepresentation: List<Identity> = ArrayList(),

        // Additional attributes included in the Specification for some types of Commits
        @JsonInclude(JsonInclude.Include.NON_NULL)
        var body: String? = null,
        @JsonInclude(JsonInclude.Include.NON_NULL)
        var direction: Feature.FeatureDirectionKind? = null,
        @JsonInclude(JsonInclude.Include.NON_NULL)
        var language: String? = null,

        //Additional attributes used by only the backend
        var valueSpecs: MutableList<Any?> = mutableListOf(),
        @JsonInclude(JsonInclude.Include.NON_NULL)
        var unitSpec: String? = null,
        @JsonInclude(JsonInclude.Include.NON_NULL)
        var dependency: String? = null,
) {
        @JsonProperty("@id")
        var id: UUID = id
        @JsonProperty("@type")
        var type: String = type

        abstract fun toElement(): Element
        abstract fun copyFromElement(element: Element): CommitData
}
