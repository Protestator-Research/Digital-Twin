package BaseEntities.implementation


import BaseEntities.Element
import BaseEntities.Identity
import BaseEntities.TextualRepresentation
import ImportedSysMDServices.AgilaSession
import Parser.ParserSysML
import Parser.SysMDParser
import Parser.firstName
import java.util.*


/**
 * A textual model of something in a modeling language.
 * In extension of the SysMLv2 metamodel we also save here results of the compilation:
 * - errorsByLine in a hashmap lineno -> error message
 * - infoByLine in a hashmap lineno -> infotext
 * @param shortName a short abbreviation for the name
 * @param name an optional name
 * @param language the language in which the model is given, e.g. SysML v2 textual or SysMD or Markdown
 * @param body the model itself
 */
class TextualRepresentationImplementation(
    elementId: UUID = UUID.randomUUID(),
    name: String? = null,
    shortName: String? = null,
    owner: Identity<Element> = Identity(),
    ownedElements: MutableList<Identity<Element>> = mutableListOf(),
    override var language: String,
    body: String
): TextualRepresentation, AnnotatingElementImplementation(
    elementId = elementId, name = name, shortName=shortName, owner = owner, ownedElements = ownedElements, body=body
) {

    /**
     * Runs the parser depending on the language field.
     * @param model The service that permits the parser modification of the internal representation of the model.
     */
    override fun parseCode(model: AgilaSession) {
        when(language.firstName()) {
            "SysMD" -> SysMDParser(model, this).parseSysMD()
            "SysML" -> ParserSysML(model, body).parseSysML()
        }
    }

    override fun clone(): TextualRepresentationImplementation {
        return TextualRepresentationImplementation(
            name = name,
            shortName = shortName,
            owner = Identity(owner),
            ownedElements = Identity.copyOfIdentityList(ownedElements),
            language = language,
            body = body).also { klon ->
            klon.model = model
            klon.updated = updated
        }
    }

    override fun updateFrom(template: Element) {
        super.updateFrom(template)
        if (template is TextualRepresentation) {
            // owner = template.owner
            // ownedElements = Identity.copyOfIdentityList(ownedElements)
            language = template.language
            body = template.body
        }
    }

    override fun toString(): String {
        return "TextualRepresentation {" +
                "body=$body, +" +
                "id='${elementId}' }"
    }
}
