package SysMDImportSolver.normalizer

import SysMDCopyAst.AstNode


/**
 * Simpler version of [AstRoot].
 */
data class SimpleAstRoot (
    val originalAst : AstNode,
) {

    var dependency: AstNode = originalAst

    fun toExpressionString(): String {
        return dependency.toExpressionString()
    }

}