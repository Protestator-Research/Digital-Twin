package ImportedSysMDQuantities.derivedUnits

import ImportedSysMDQuantities.DerivedUnit
import ImportedSysMDQuantities.NoPrefix
import ImportedSysMDQuantities.Prefix
import ImportedSysMDQuantities.baseUnits.Length
import ImportedSysMDQuantities.baseUnits.LuminousIntensity
import ImportedSysMDQuantities.baseUnits.Mass
import ImportedSysMDQuantities.baseUnits.Time

private var siUnitSet = setOf(
    Time.Second.copy(3), LuminousIntensity.Candela.copy(),
    Length.Meter.copy(-2), Mass.Kilogram.copy(-1)
)

open class LuminousEfficacy(name: String, symbol: String, prefix: Prefix, convFac: Double, exponent: Int = 1, isLogarithmic: Boolean = false) :
    DerivedUnit(name, symbol, prefix, "luminous efficacy", siUnitSet, convFac, exponent, isLogarithmic) {

    /**
     * generate UnitObjects and add them to the UnitList
     */
    object LumenPerWatt : LuminousEfficacy("lumen per watt", "lm/W", NoPrefix, 1.0)

    override fun copy(): LuminousEfficacy {
        return LuminousEfficacy(name, symbol, prefix, convFac, exponent, isLogarithmic)
    }
}