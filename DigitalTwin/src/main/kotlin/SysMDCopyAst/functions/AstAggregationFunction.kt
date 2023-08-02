package SysMDCopyAst.functions

import ImportedSysMDServices.AgilaSession


/**
 * A function call of a user-defined function.
 */
abstract class AstAggregationFunction(
    name: String,
    model: AgilaSession
) :
    AstFunction(name, model, 0, ArrayList()) {
    abstract fun getDependentPropertyStrings(): Set<String>
}
