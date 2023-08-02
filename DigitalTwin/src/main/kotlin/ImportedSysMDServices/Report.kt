package ImportedSysMDServices

import BaseEntities.Element
import BaseEntities.TextualRepresentation
import ImportedSysMDExceptions.SysMDError
import ImportedSysMDExceptions.SysMDException
import ImportedSysMDExceptions.SysMDInfo
import ImportedSysMDExceptions.InternalError
import Parser.Scanner



/**
 * The error reporting function adds error messages to the status.
 * If the flag catchExceptions is set to false, an exception is thrown.
 * The function uses also the value of lineNo to add a line number to the error message, if present.
 * Avoid use of this function; better directly pass the exception instead.
 * @param element The element in which the error occurred .
 * @param message A descriptive error message.
 */
fun AgilaSession.reportInfo(element: Element, textualRepresentation: TextualRepresentation, lineNo: Int, message: String) {
    report(
        SysMDInfo(
        message=message,
        textualRepresentation = textualRepresentation,
        token= Scanner.Definitions.Token(kind=Scanner.Definitions.Token.Kind.INFO, string = "", lineNo=lineNo),
        element = element)
    )
}

fun AgilaSession.reportInfo(element: Element, message: String) {
    report(SysMDInfo(element=element, message = message))
}


/**
 * The error reporting function adds error messages to the status.
 * If the flag catchExceptions is set to false, an exception is thrown.
 * The function uses also the value of lineNo to add a line number to the error message, if present.
 * Avoid use of this function; better directly pass the exception instead.
 * @param exception the exception that caused the error
 */
fun AgilaSession.report(exception: Throwable) {
    if (exception is SysMDException) {
        status.exceptions.add(exception)
    } else
        status.exceptions.add(InternalError("Internal error: ${exception.message}", exception))
}

fun AgilaSession.report(message: String) {
    status.exceptions.add(
        SysMDError(
            message = message
        )
    )
}

fun AgilaSession.report(element: Element?, message: String, cause: Throwable? = null) {
    val generator = if (element != null) getRelationshipsTo(element, "Generated elements") else null
    val textualRepresentation = generator?.firstOrNull()?.source?.firstOrNull()
    status.exceptions.add(
        SysMDError(
            textualRepresentation = textualRepresentation?.ref as? TextualRepresentation,
            message = message,
            element = element,
            cause = cause
        )
    )
}

fun AgilaSession.reportInconsistency(element: Element, message: String) {
    status.exceptions.add(SysMDError("Inconsistency in internal data: $message of element ${element.name}"))
}

