package ImportedSysMDExceptions

import BaseEntities.Element
import BaseEntities.TextualRepresentation
import Parser.Scanner


/**
 * General error class that is also used for persisting error list.
 * Each entry has:
 * @param message Mandatory textual description
 * @param textualRepresentation the textual representation in which the error has occurred
 * @param token the token where the error has occurred
 * @param element the element in which the error has occurred
 * @param
 */
open class SysMDInfo(
    message: String,
    textualRepresentation: TextualRepresentation? = null,
    token: Scanner.Definitions.Token? = null,
    element: Element? = null,
    cause: Throwable? = null
): SysMDException(message, textualRepresentation, token, element, cause)
