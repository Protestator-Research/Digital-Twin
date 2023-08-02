package ImportedSysMDQuantities.baseUnits

import ImportedSysMDQuantities.BaseUnit
import ImportedSysMDQuantities.NoPrefix
import ImportedSysMDQuantities.Prefix


open class LuminousIntensity(name: String, symbol: String, prefix: Prefix, convFac: Double = 1.0, exponent: Int = 1) :
    BaseUnit(name, symbol, prefix, "luminous intensity", convFac, exponent) {

    /**
     * generate UnitObjects and add them to the UnitList
     */
    object Candela : LuminousIntensity("candela", "cd", NoPrefix)

    override fun copy(exponentValue: Int): LuminousIntensity {
        return LuminousIntensity(name, symbol, prefix, convFac, exponentValue)
    }
}