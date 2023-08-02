package SysMDRestImport.entities.requests

import BaseEntities.*
import BaseEntities.implementation.*
import BaseEntities.Feature
import ImportedSysMDExceptions.RequestInvalidException
import com.fasterxml.jackson.annotation.JsonProperty
import com.github.tukcps.jaadd.values.IntegerRange

import java.util.*


/**
 * The payload model for a request of a client to the Agila backend.
 * The value of _type_ is mandatory.
 * All other fields are optional, depending on the type.
 */
class ElementRequest {
    @JsonProperty("@id")   // elementId?
    var id: UUID = UUID.randomUUID() // elementId?

    @JsonProperty("@type")
    val type: String? = null              // mandatory type of the metamodel as annotation

    // The following are required for every Element
    val name: String? = null
    val shortName: String? = null
    val ownedElements: List<UUID>? = null // The IDs of the owned elements.
    val owner: UUID? = null                 // The ID of the owner; if null, the node is the root of an ownership tree

    // For type Feature :
    val direction: Feature.FeatureDirectionKind? = null

    // For type ValueProperty; will be moved to owned elements within v2.9
    val valueSpecs = mutableListOf<Any?>() // Owned element of type Value?
    val unitSpec: String? = null   // Owned element of type Value/Unit?
    val dependency: String? = null // Owned element of type ExpressionValue?

    val importedMemberName: String? = null
    val importedNamespace: String? = null

    // For type AnnotationElement and subclasses
    val language: String? = null   // language, e.g. SysMD, SysML
    val body: String? = null       // The code in e.g. SysMD or SysML v2 textual

    // For Relationship and subtypes thereof:
    val source: List<UUID>? = null     // qualified names of sources
    val target: List<UUID>? = null     // qualified names of targets


    /**
     * The method converts an ElementRequest object to an Element object of one of the
     * Element subclasses.
     */
    fun toElement(): Element {
        val element = when (type) {
            "NamespaceImport"-> NamespaceImportImplementation(
                elementId=id, owner =
                Identity(owner),
                importedNamespace = Identity(id=target?.firstOrNull(), str=importedNamespace, ref=null))
            "MembershipImport" -> MembershipImportImplementation(
                id=id, owner = Identity(owner),
                importedMemberName = Identity(id=null, ref=null, str=importedMemberName))
            "Multiplicity" -> MultiplicityImplementation(
                valueSpec = if (valueSpecs.size!=0) IntegerRange(valueSpecs[0] as String) else IntegerRange(1, 1)
            )

            "Specialization" -> SpecializationImplementation(
                elementId = id,
                subclass = Identity<Type>(id = source?.get(0)!!, null, null),
                superclass = Identity<Type>(id = target?.get(0)!!, null, null)
            )

            "Classifier",           // Classifier is deprecated and only needed for modeling purposes
            "Class" -> ClassImplementation(elementId = id, name = name, shortName = shortName)

            "Feature" -> FeatureImplementation(
                elementId = id,
                name = name,
                shortName = shortName,
                direction = direction ?: Feature.FeatureDirectionKind.IN
            )

            "ValueFeature", "Property" // Property is deprecated.
            -> ValueFeatureImplementation(
                elementId = id,
                name = name,
                shortName = shortName,
                dependency = dependency ?: "",
                valueSpecs = valueSpecs
            )

            "Package" -> PackageImplementation(elementId = id, name = name, shortName = shortName)
            "Documentation" -> DocumentationImplementation(elementId = id, name = name, shortName = name, body = body!!)
            "AnnotatingElement" -> AnnotatingElementImplementation(
                elementId = id,
                name = name,
                shortName = name,
                body = body!!
            )

            "TextualRepresentation" -> TextualRepresentationImplementation(
                elementId = id,
                name = name,
                shortName = shortName,
                body = body!!,
                language = language!!
            )

            "Comment" -> CommentImplementation(elementId = id, name = name, shortName = shortName, body = body!!)
            "Relationship" -> RelationshipImplementation(
                elementId = id,
                name = name,
                shortName = shortName,
                source = source?.toIdentityList() ?: mutableListOf(),
                target = target?.toIdentityList() ?: mutableListOf()
            )

            "Association" -> AssociationImplementation(
                elementId = id,
                name = name,
                shortName = shortName,
                sources = source?.toIdentityList() ?: mutableListOf(),
                targets = target?.toIdentityList() ?: mutableListOf()
            )

            else -> throw RequestInvalidException("Invalid type in request: '$type'")
        }
        element.owner = Identity(owner)
        return element
    }
}



fun ElementRequest.update(element: Element) : Element {
    // identification, description, ... are common fields.
    element.name = name?:element.name
    element.shortName = shortName?:element.shortName

    // common method for elements with owned elements
    fun updateOwnedElements(element: Element) {
        element.ownedElements = if (ownedElements != null ) {
            mutableListOf<Identity<Element>>() .also { list -> ownedElements.forEach { list.add(Identity(it)) } }
        } else element.ownedElements
    }

    when(element) {
        is Multiplicity -> {
            val range = if (valueSpecs.size!=0) IntegerRange(valueSpecs[0] as String) else IntegerRange(1, 1)
            element.valueSpecs = mutableListOf(range)
        }

        is Import -> {
            element.target = this.target?.toIdentityList()!!
        }

        is Package -> {
            if (type != "Package")
                throw RequestInvalidException("Update does not allow change of existing type")
            updateOwnedElements(element)
        }

        is ValueFeature -> {
            if (type != "Property" && type != "ValueFeature")
                throw RequestInvalidException("Update does not allow change of type")
            if (dependency != null)
                element.dependency = dependency
            element.unitSpec = this.unitSpec?:element.unitSpec
            element.valueSpecs = this.valueSpecs?:element.valueSpecs
        }

        is Feature -> {
            if (type != "Feature")
                throw RequestInvalidException("Update does not allow change of type")
            updateOwnedElements(element)
        }

        is Classifier -> {
            if (type != "Classifier")
                throw RequestInvalidException("Update does not allow change of existing type")
            updateOwnedElements(element)
        }

        is TextualRepresentation -> {
            if (body != null) element.body = body
            if (language != null) element.language = language
        }

        is Comment -> {
            if (body != null) element.body = body
        }

        is AnnotatingElement -> {
            if (body != null) element.body = body
        }

        else ->
            throw RequestInvalidException("Unknown type; expected Classifier, Package, Property, Feature")
    }
    return element
}
