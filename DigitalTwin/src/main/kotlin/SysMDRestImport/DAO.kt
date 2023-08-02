package SysMDRestImport

import BaseEntities.Element
import BaseEntities.*
import BaseEntities.implementation.*
import com.fasterxml.jackson.annotation.JsonProperty
import com.github.tukcps.jaadd.values.IntegerRange

import java.util.*


/**
 * In a commit, we distinguish four cases:
 * - Change of an element or relationship,
 * - Deleting an element from a commit,
 * - Project usage = reference to another project to be added to session.
 */
enum class DataVersionType {
    ChangedElement, ChangedRelationship, DeletedInCommit, ProjectUsage
}

/**
 * A simple plain old java object for transfer via REST & persistence layer.
 */
data class ElementDAO(
    // Simple version of the 4 kind of Data version types that go via REST in SysML V2 standard.
    val dataVersionType: DataVersionType = DataVersionType.ChangedElement,

    //used to parse JSON and map it to ElementDAO
    @JsonProperty("@id")
    var elementId: UUID, // --> elementId
    @JsonProperty("@type")
    var type: String? = null,       // mandatory type of the metamodel as annotation

    var name: String? = null,
    var shortName: String? = null,

    // ownedElements not supported; just here for future use.
    // Changing an owned element should be done via own endpoint .../elements/id/elements ...
    var ownedElements: MutableList<UUID>? = mutableListOf(),   //  --> Via own endpoint!
    var owner: UUID? = null,

    // For type = Feature
    var direction: Feature.FeatureDirectionKind? = null,

    // For type = Import
    var importedMemberName: String? = null,
    var importedNamespace: String? = null,

    // For type Property; move to owned elements
//    @JsonProperty("valueSpec")
    var valueSpecs: MutableList<Any?> = mutableListOf(),  // Owned element of type Value?
    var unitSpec:  String? = null,  // Owned element of type Value/Unit?
    var dependency: String? = null, // Owned element of type ExpressionValue?

    // For type AnnotationElement:
    var language: String? = null,  // language, e.g. SysMD, SysML
    var body: String? = null,      // The code in e.g. SysMD or SysML v2 textual

    // For Relationship and subtypes thereof:
    var source: MutableList<UUID?>? = mutableListOf(),     // list of id
    var target: MutableList<UUID?>? = mutableListOf(),     // list of if
)

/**
 * Extension methods; kept separately from ElementDAO to avoid any problem with
 * Spring ...
 */
fun ElementDAO.toElement(): Element {
    // if (type == null)
    //     throw Exception("Element with type null in response; must be valid entity type.")
    val element = when (type) {
        "Annotation"    -> AnnotationImplementation(elementId=elementId, name=name, shortName=shortName)
        "NamespaceImport"-> NamespaceImportImplementation(elementId=elementId, owner = Identity(owner),
            importedNamespace = Identity(id=target?.firstOrNull(), str=importedNamespace, ref=null))
        "MembershipImport" -> MembershipImportImplementation(id=elementId, owner = Identity(owner),
            importedNamespace = Identity(id=target?.firstOrNull(), str=importedNamespace, ref=null),
            importedMemberName = Identity(id=null, ref=null, str=importedMemberName))
        "Multiplicity"  -> MultiplicityImplementation(elementId=elementId, name=name, valueSpec=if (valueSpecs[0] != emptyList<Any?>()) IntegerRange(valueSpecs[0] as String) else IntegerRange(1,1))
        "Specialization"-> SpecializationImplementation(elementId=elementId, owner= Identity(owner), subclass= Identity(id = source?.firstOrNull()), superclass= Identity(id = target?.firstOrNull()))
        "FeatureTyping" -> FeatureTypingImplementation(elementId=elementId, owner= Identity(owner), typedFeature= Identity(id = source?.firstOrNull()), type= Identity(id = target?.firstOrNull()))
        "Classifier",
        "Class"         -> ClassImplementation(elementId=elementId, name=name, shortName=shortName)
        "Type"          -> TypeImplementation(elementId=elementId, name=name, shortName=shortName)
        "Feature"       -> FeatureImplementation(elementId=elementId, name=name, shortName=shortName, direction=direction?:Feature.FeatureDirectionKind.IN)
        "ValueFeature"  -> ValueFeatureImplementation(elementId = elementId, name = name, shortName = shortName, dependency = dependency?:"", valueSpecs  = valueSpecs)
        "Package"       -> PackageImplementation(elementId=elementId, name=name, shortName=shortName)
        "Namespace"     -> NamespaceImplementation(elementId=elementId, name=name, shortName=shortName)
        "Documentation" -> DocumentationImplementation(elementId=elementId, name=name, shortName=name, body = body!!)
        "AnnotatingElement" -> AnnotatingElementImplementation(elementId=elementId, name=name, shortName=name, body = body!!)
        "TextualRepresentation" -> TextualRepresentationImplementation(elementId=elementId, name=name, shortName=shortName, body = body!!, language = language!!)
        "Comment"        -> CommentImplementation(elementId=elementId, name=name, shortName=shortName, body = body!!)
        "Relationship"   -> RelationshipImplementation(elementId=elementId, name=name, shortName=shortName)
        "Association"    -> AssociationImplementation(elementId=elementId, name=name, shortName=shortName)
        else             -> throw Exception("Element with unknown type '$type' in response; must be valid entity type.")
    }
    element.owner = Identity(str=null, id=if (owner != null) owner else null, ref=null)
    ownedElements?.forEach {
        element.ownedElements.plusAssign(Identity(str=null, id= it, ref=null))
    }
    if (element is Relationship && element !is Import && element !is Specialization) {
        source?.forEach { element.source.add(Identity(it)) }
        target?.forEach { element.target.add(Identity(it)) }
    }
    if (element is NamespaceImportImplementation) {
        element.importedNamespace.str = importedNamespace
        element.target.first().str = importedNamespace
        // element.importedMemberName = if (importedMemberName==null) null else Identity(str=importedMemberName!!)
    }
    return element
}

/**
 * Maps an element of entities.Element to its respective DAO.
 * The DAO is used for (de)serialization.
 */
fun Element.toDAO(): ElementDAO {
    require(model != null)
    val dao = ElementDAO(
        elementId = elementId,
        name = name,
        shortName = shortName,
        owner = if (owner.id != model!!.global.elementId) owner.id else null
    )

    ownedElements.forEach {
        dao.ownedElements?.add(it.id!!)
    }

    if (this is Relationship) {
        source.forEach {
            dao.source!!.add(if (it.id == model!!.global.elementId) null else it.id)
        }
        target.forEach {
            dao.target!!.add( if (it.id == model!!.any.elementId) null else it.id)
        }
    }
    when(this) {
        is Association ->  { dao.type = "Association" }
        is Multiplicity -> { dao.type = "Multiplicity"; dao.valueSpecs = mutableListOf(intSpecs[0].toString())}
        is ValueFeature -> {
            dao.type = "ValueFeature"
            dao.valueSpecs = valueSpecs
            dao.unitSpec = unitSpec
            dao.dependency = dependency
        }

        is Feature -> { dao.type = "Feature"; dao.direction = direction }
        is Class, is Classifier -> { dao.type = "Class" }
        is Type -> { dao.type = "Type" }
        is Package -> { dao.type = "Package" }
        is Namespace -> { dao.type = "Namespace" }
        is TextualRepresentation -> { dao.type = "TextualRepresentation"; dao.body = body; dao.language = language }

        /* is Comment -> {
            type = "Comment"
            body = e.body
        }*/

        // next version: further differentiation ! ! !
        is AnnotatingElement -> { dao.type = "AnnotatingElement"; dao.body = body }

        is Annotation -> { dao.type = "Annotation" }
        is NamespaceImport -> {
            dao.type = "NamespaceImport"
            dao.importedNamespace = importedNamespace.str

        }
        is MembershipImport -> {
            dao.type = "MembershipImport"
            dao.importedNamespace = importedNamespace.str
            dao.importedMemberName = importedMemberName.str
        }
        is FeatureTyping  -> { dao.type = "FeatureTyping"}
        is Specialization -> { dao.type = "Specialization" }
        is Relationship   -> { dao.type = "Relationship" }
    }
    return dao
}
