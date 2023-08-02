package ImportedSysMDQuantities.derivedUnits

import ImportedSysMDQuantities.DerivedUnit
import ImportedSysMDQuantities.NoPrefix
import ImportedSysMDQuantities.Prefix
import ImportedSysMDQuantities.baseUnits.Length
import ImportedSysMDQuantities.baseUnits.LuminousIntensity

private var siUnitSet = setOf(LuminousIntensity.Candela.copy(), Length.Meter.copy(-2))

open class Luminance(name: String, symbol: String, prefix: Prefix, convFac: Double, exponent: Int = 1, isLogarithmic: Boolean = false) :
    DerivedUnit(name, symbol, prefix, "luminance", siUnitSet, convFac, exponent, isLogarithmic) {

    /**
     * generate UnitObjects and add them to the UnitList
     */
    object Stilb : Luminance("stilb", "sb", NoPrefix, 1.0)

    override fun copy(): Luminance {
        return Luminance(name, symbol, prefix, convFac, exponent, isLogarithmic)
    }
}