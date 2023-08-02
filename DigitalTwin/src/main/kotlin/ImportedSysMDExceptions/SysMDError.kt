package ImportedSysMDExceptions

import BaseEntities.Element
import BaseEntities.TextualRepresentation
import Parser.Scanner
import Parser.SysMDParser


/**
 * General error class that is also used for Copersisting error list.
 * Each entry has:
 * @param message Mandatory textual description
 * @param textualRepresentation the textual representation in which the error has occurred
 * @param token the token where the error has occurred
 * @param element the element in which the error has occurred
 * @param
 */
open class SysMDError(
    message: String,
    textualRepresentation: TextualRepresentation? = null,
    token: Scanner.Definitions.Token? = null,
    element: Element? = null,
    cause: Throwable? = null
) : SysMDException(message, textualRepresentation, token, element, cause) {

    /**
     * The priority the exception has for the functionality of the model
     */
    protected var priority: Int = 1
}

/**
 * This Exception is thrown for all errors that are caused in the parsing methods.
 * It just writes an error message. Line and column are optional.
 */
class SyntaxError(
    parser: SysMDParser? = null,
    message: String,
) : SysMDError(message = message) {
    init {
        textualRepresentation = parser?.textualRepresentation
        token = parser?.token
        priority = 2
    }
}


/**
 * This Exception is thrown for all errors that are caused in the parsing methods.
 * It just writes an error message. Line and column are optional.
 */
class LexicalError(
//TODO integrate into exception hierarchy
    scanner: Scanner,
    message: String,
) : SysMDError(message = message) {
    init {
        if (scanner is SysMDParser) {
            textualRepresentation = scanner.textualRepresentation
            token = scanner.token
        }
        priority = 2
    }
}

/**
 * This Exception is thrown for all errors during initialized and propagate phases.
 * It just creates an error message.
 */
class SemanticError(message: String, cause: Throwable? = null) : SysMDError(message, cause = cause) {
    init {
        priority = 2
    }
}


/**
 * Error in the setup of a property.
 */
class PropertyError internal constructor(msg: String) :
    Exception("Error in expression: $msg")

/**
 * Internal problem caused exception; e.g. due to inconsistent internal data structures.
 */
class InternalError(message: String, cause: Throwable? = null) : SysMDError(message, cause = cause) {
    init {
        priority = 2
    }
}


/**
 * Exception for an Element not found, where element can be an arbitrary entity.
 * @param name: id or name that is used as string for reporting the error
 */
class ElementNotFoundException(
    textualRepresentation: TextualRepresentation?,
    element: Element? = null,
    val name: String,
    token: Scanner.Definitions.Token? = null
) : SysMDError(
    message = "Element '$name' not found",
    textualRepresentation = textualRepresentation,
    element = element,
    token = token
) {
    constructor(parser: SysMDParser?, name: String)
            : this(parser?.textualRepresentation, null, name)

    constructor(element: Element?, name: String)
            : this(null, element, name, null)

    constructor(textualRepresentation: TextualRepresentation?, token: Scanner.Definitions.Token?)
            : this(textualRepresentation, null, name = token?.string!!, token = token)

    constructor(name: String)
            : this(null, null, name = name, null)

    init {
        priority = 2
    }
}

/**
 * Exception for static errors.
 *
 * Use more precise exception class derived from this class if possible
 */
open class StaticException(textualRepresentation: TextualRepresentation?, message: String, cause: Throwable? = null) :
    SysMDError(
        message = message,
        textualRepresentation = textualRepresentation,
        cause = cause
    ) {
    init {
        priority = 2
    }
}

/**
 * Exception for initialisation problems.
 *
 * For import errors use [ImportException]
 */
class InitialisationException(
    textualRepresentation: TextualRepresentation?,
    message: String,
    cause: Throwable? = null
) :
    StaticException(textualRepresentation, message, cause) {
    init {
        priority = 3
    }
}

/**
 * Exception for import errors.
 */
class ImportException(
    textualRepresentation: TextualRepresentation?,
    message: String,
    cause: Throwable?
) :
    StaticException(textualRepresentation, message, cause) {
    init {
        priority = 3
    }
}

/**
 * Exception for problems occurring during constraint evaluation, propagation
 *
 * This exception is used highly generically. Consider creating request for more specific error class.
 */
open class SolverException(
    textualRepresentation: TextualRepresentation?,
    message: String,
    element: Element?,
    cause: Throwable?
) : SysMDError(textualRepresentation = textualRepresentation, message = message, element = element, cause = cause) {
    init {
        priority = 2
    }
}

/**
 * Issues like ranges or missing specifications of elements are caught with this exception.
 *
 * Beware that this class and its subclasses are **not** to be used for actual errors!
 */
open class ExportIssue(
    textualRepresentation: TextualRepresentation?,
    message: String,
    cause: Throwable?,
    element: Element?
) : SysMDException(
    message = message, textualRepresentation = textualRepresentation, cause = cause, element = element
)

/**
 * Exception for elements which are yet undefined but not affecting the model
 * and should be defined when exporting
 */
class ClassificationIssue(
    textualRepresentation: TextualRepresentation?,
    message: String,
    cause: Throwable?,
    element: Element?
) : ExportIssue(
    textualRepresentation = textualRepresentation,
    message = message,
    cause = cause,
    element = element
)

/**
 * Exception for elements parameterised with a range of values which should have
 * a single value for export
 */
class ParametrisationIssue(
    textualRepresentation: TextualRepresentation?,
    message: String,
    cause: Throwable?,
    element: Element?
) : ExportIssue(
    textualRepresentation = textualRepresentation,
    message = message,
    cause = cause,
    element = element
)

//TODO specify reason for this exception
class SpecificationIssue(
    textualRepresentation: TextualRepresentation?,
    message: String,
    cause: Throwable?,
    element: Element?
) : ExportIssue(
    textualRepresentation = textualRepresentation,
    message = message,
    cause = cause,
    element = element
)

/**
 * Exception for inheritance errors
 *
 * Example: Cyclic dependencies
 */
open class InheritanceException(
    message: String,
    textualRepresentation: TextualRepresentation? = null,
    element: Element? = null,
    cause: Throwable? = null
) : SolverException(
    message = message,
    textualRepresentation = textualRepresentation,
    element = element,
    cause = cause
)