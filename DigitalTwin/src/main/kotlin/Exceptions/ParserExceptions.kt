package Exceptions

import com.github.tukcps.sysmd.parser.ParserSysMD
import Parser.Scanner




/**
 * This Exception is thrown for all errors that are caused in the parsing methods.
 * It just writes an error message. Line and column are optional.
 */
class SyntaxError(
    parser: ParserSysMD? = null,
    message: String,
): Exception(message = message) {
}


/**
 * This Exception is thrown for all errors that are caused in the parsing methods.
 * It just writes an error message. Line and column are optional.
 */
class LexicalError(
    scanner: Scanner,
    message: String,
): Exception(message = message) {
}

/**
 * This Exception is thrown for all errors during initialized and propagate phases.
 * It just creates an error message.
 */
class SemanticError(message: String, cause: Throwable? = null): Exception(message, cause = cause)
