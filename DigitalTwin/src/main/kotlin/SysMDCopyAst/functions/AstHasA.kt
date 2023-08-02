package SysMDCopyAst.functions

import BaseEntities.Element
import BaseEntities.Namespace
import ImportedSysMDQuantities.Quantity
import ImportedSysMDServices.AgilaSession
import ImportedSysMDServices.report
import ImportedSysMDServices.resolveName
import Parser.QualifiedName
import SysMDCopyAst.functions.AstFunction


class AstHasA (model: AgilaSession,
               private val owningNamespace: Namespace,
               private val ownerName: QualifiedName,
               private val ownedName: QualifiedName)
    : AstFunction("hasA", model, 0) {

    override fun initialize() {
        upQuantity = Quantity(model.builder.Bool)
        evalUp()
        downQuantity = upQuantity.clone()
    }

    override fun evalUp() {
        // Search for
        val owner = owningNamespace.resolveName<Element>(ownerName)
        if (owner is Namespace) {
            upQuantity = if (owner.resolveName<Element>(ownedName) != null)
                Quantity(model.builder.True)
            else Quantity(model.builder.False)
        } else
            model.report(null, "Evaluation of hasA() not possible as parameter not a namespace.")
    }

    override fun evalDown() {
        // getParam(0).downQuantity = downQuantity.log()
    }

    override fun toExpressionString() = "hasA(${getParam(0).toExpressionString()}, ${getParam(1).toExpressionString()})"
}
