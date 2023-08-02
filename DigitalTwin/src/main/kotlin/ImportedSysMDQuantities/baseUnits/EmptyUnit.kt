package ImportedSysMDQuantities.baseUnits

import ImportedSysMDQuantities.BaseUnit
import ImportedSysMDQuantities.NoPrefix
import ImportedSysMDQuantities.Prefix


open class EmptyUnit(name: String, symbol: String, prefix: Prefix, convFac: Double = 1.0, exponent: Int = 1) :
    BaseUnit(name, symbol, prefix, "emptiness", convFac, exponent) {

    /**
     * generate UnitObjects and add them to the UnitList
     */
    object Empty : EmptyUnit("empty", "?", NoPrefix)

    override fun copy(exponentValue: Int): EmptyUnit {
        return EmptyUnit(name, symbol, prefix, convFac, exponentValue)
    }
}