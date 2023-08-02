package ImportedSysMDQuantities.baseUnits

import ImportedSysMDQuantities.BaseUnit
import ImportedSysMDQuantities.NoPrefix
import ImportedSysMDQuantities.Prefix


open class ElectricCurrent(name: String, symbol: String, prefix: Prefix, convFac: Double = 1.0, exponent: Int = 1) :
    BaseUnit(name, symbol, prefix, "electric current", convFac, exponent) {

    /**
     * generate UnitObjects and add them to the UnitList
     */
    object Ampere : ElectricCurrent("ampere", "A", NoPrefix)

    override fun copy(exponentValue: Int): ElectricCurrent {
        return ElectricCurrent(name, symbol, prefix, convFac, exponentValue)
    }
}