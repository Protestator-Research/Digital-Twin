package SysMDRestImport.entities

import BaseEntities.Feature
import com.fasterxml.jackson.annotation.JsonProperty
import java.util.*

//@EqualsAndHashCode(exclude = ["snapshotId"])
//@Document("ElementSnapshot")
open class ElementSnapshot(
//    @Id
    @JsonProperty("@id")
    var snapshotId: UUID = UUID.randomUUID(),
    var elementId: UUID,
    var type: String,       // mandatory type of the metamodel as annotation

    var name: String? = null,
    var shortName: String? = null,

    // ownedElements not supported; just here for future use.
    // Changing an owned element should be done via own endpoint .../elements/id/elements ...
    var ownedElements: MutableList<UUID> = mutableListOf(),   //  --> Via own endpoint!
    var owner: UUID? = null,

    // For type = Feature
    var direction: Feature.FeatureDirectionKind? = null,

    // For type = Import
    var importedMemberName: String? = null,
    var importedNamespace: String? = null,

    // For type Property; move to owned elements
    var valueSpecs: MutableList<Any?> = mutableListOf(),  // Owned element of type Value?
    var unitSpec:  String? = null,  // Owned element of type Value/Unit?
    var dependency: String? = null, // Owned element of type ExpressionValue?

    // For type AnnotationElement:
    var language: String? = null,  // language, e.g. SysMD, SysML
    var body: String? = null,      // The code in e.g. SysMD or SysML v2 textual

    // For Relationship and subtypes thereof:
    var source: MutableList<String> = mutableListOf(),     // list of id
    var target: MutableList<String> = mutableListOf(),     // list of if
) {
    override fun equals(other: Any?): Boolean {
        if (other == null || other !is ElementSnapshot) return false
        return elementId == other.elementId && type == other.type && name == other.name
                && shortName == other.shortName && ownedElements == other.ownedElements
                && owner == other.owner && direction == other.direction
                && importedMemberName == other.importedMemberName
                && importedNamespace == other.importedNamespace && valueSpecs == other.valueSpecs
                && unitSpec == other.unitSpec && dependency == other.dependency
                && language == other.language && body == other.body && source == other.source
                && target == other.target
    }
}
