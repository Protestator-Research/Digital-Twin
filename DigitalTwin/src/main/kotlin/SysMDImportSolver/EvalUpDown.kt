package SysMDImportSolver

import BaseEntities.ValueFeature
import Parser.parseDependency


/** Evaluates the dependency string in the property and updates its values top-down */
fun ValueFeature.evalDown() {
    if ( ast == null )
        parseDependency()
    ast?.evalDownRec()
}

/**
 * Evaluates the dependency in the property with key and updates its values bottom up.
 * If the AST is not yet built, it is parsed from the dependency expression string.
 */
fun ValueFeature.evalUp() {
    if (ast == null)
        parseDependency()
    ast?.evalUpRec()
}