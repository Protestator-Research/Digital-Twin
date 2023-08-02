package SysMDCopyAst

import ImportedSysMDExceptions.SemanticError
import ImportedSysMDQuantities.VectorQuantity
import Parser.Scanner
import SysMDCopyAst.functions.AstNot
import com.github.tukcps.jaadd.AADD
import com.github.tukcps.jaadd.IDD

class AstUnaryOp(
    val op: Scanner.Definitions.Token.Kind,
    val operand: AstNode
): AstNode(operand.model) {

    override fun initialize() {
        upQuantity = when (operand.upQuantity.values[0]) {
            is AADD -> VectorQuantity(mutableListOf(model.builder.Reals), "")
            is IDD -> VectorQuantity(mutableListOf(model.builder.Integers))
            else -> throw SemanticError("unary minus argument must be of type Integer or Real")
        }
        evalUp()
        downQuantity = upQuantity.clone()
    }

    override fun evalUpRec() {
        operand.evalUpRec()
        evalUp()
    }

    override fun evalUp() {
        when(op) {
            Scanner.Definitions.Token.Kind.MINUS -> upQuantity = operand.upQuantity.negate()
            Scanner.Definitions.Token.Kind.PLUS -> upQuantity = operand.upQuantity.clone()
            // Token.Kind.NOT -> upQuantity = operand.upQuantity.not()
            else -> { throw SemanticError("Problem with unary operation")
            }
        }
    }

    override fun evalDownRec() {
        evalDown()
        operand.evalDownRec()
    }

    /** No propagation; Boolean functions are handled by discrete solver */
    override fun evalDown() {
        when(op) {
            Scanner.Definitions.Token.Kind.MINUS -> operand.downQuantity = downQuantity.negate()
            Scanner.Definitions.Token.Kind.PLUS -> operand.downQuantity = downQuantity.clone()
            else -> throw SemanticError("unary minus only supported for Integer or Real")
        }
    }

    override fun <R> runDepthFirst(block: AstNode.() -> R): R {
        operand.runDepthFirst(block)
        return this.run(block)
    }

    override fun <R> withDepthFirst(receiver: AstNode, block: AstNode.() -> R): R =
        with(receiver) {
            withDepthFirst(operand, block)
            return block()
        }

    override fun toExpressionString(): String {
        return "-${operand.toExpressionString()}"
    }

    override fun clone(): AstNot {
        val parClone = arrayListOf(operand.clone())
        return AstNot(model, parClone)
    }
}