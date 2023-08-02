package BaseEntities

import ImportedSysMDServices.AgilaSession
import Parser.QualifiedName
import Parser.dropFirstName


/**
 * A textual model of something in a modeling language.
 * In extension of the SysMLv2 metamodel we also save here results of the compilation:
 * - errorsByLine in a hashmap lineno -> error message
 * - infoByLine in a hashmap lineno -> infotext
 */
interface TextualRepresentation: AnnotatingElement {
    var language: String

    /**
     * Runs the parser depending on the language field.
     * @param model The service that permits the parser modification of the internal representation of the model.
     */
    fun parseCode(model: AgilaSession)
    override fun clone(): TextualRepresentation

    fun getNamespacePrefix(): QualifiedName {
        var namespace =  language.dropFirstName()
        if (namespace.isBlank())
            namespace = "Global"
        return namespace
    }
}