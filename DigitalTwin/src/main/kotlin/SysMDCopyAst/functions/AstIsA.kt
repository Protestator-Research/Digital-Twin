package SysMDCopyAst.functions

import BaseEntities.Element
import BaseEntities.Namespace
import BaseEntities.Type
import ImportedSysMDQuantities.Quantity
import ImportedSysMDServices.AgilaSession
import ImportedSysMDServices.report
import ImportedSysMDServices.resolveName
import Parser.QualifiedName
import SysMDCopyAst.functions.AstFunction



/**
 * Pre-defined SysMD function that checks if an element owns an element with a name given as parameter.
 * Example:
 *
 * Global hasA a: Boolean = hasA(x::y)
 *
 * evaluates to true, iff x::y is resolvable.
 */
class AstIsA(model: AgilaSession,
             private val owningNamespace: Namespace,
             private val subclassName: QualifiedName,
             private val superclassName: QualifiedName)
: AstFunction("hasA", model, 0) {

    override fun initialize() {
        upQuantity = Quantity(model.builder.Bool)
        evalUp()
        downQuantity = upQuantity.clone()
    }

    override fun evalUp() {
        // Search for
        val subclass = owningNamespace.resolveName<Element>(subclassName)
        val superclass = owningNamespace.resolveName<Element>(superclassName)

        if (subclass is Type && superclass is Type) {
            upQuantity = Quantity(
                if (subclass.isSubclassOf(superclass)) model.builder.True else model.builder.False
            )
        } else
            model.report(null, "Evaluation of hasA() not possible as parameter not a classifier or type.")
    }

    override fun evalDown() {
        // getParam(0).downQuantity = downQuantity.log()
    }

    override fun toExpressionString() = "hasA(${getParam(0).toExpressionString()}, ${getParam(1).toExpressionString()})"
}
