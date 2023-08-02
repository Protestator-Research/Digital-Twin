package ImportedSysMDQuantities.derivedUnits

import ImportedSysMDQuantities.DerivedUnit
import ImportedSysMDQuantities.NoPrefix
import ImportedSysMDQuantities.Prefix
import ImportedSysMDQuantities.baseUnits.Length
import ImportedSysMDQuantities.baseUnits.Mass


private var siUnitSet = setOf(Mass.Kilogram.copy(), Length.Meter.copy(-3))

open class Density(name: String, symbol: String, prefix: Prefix, convFac: Double, exponent: Int = 1, isLogarithmic: Boolean = false) :
    DerivedUnit(name, symbol, prefix, "density", siUnitSet, convFac, exponent, isLogarithmic) {

    /**
     * generate UnitObjects and add them to the UnitList
     */
    object KilogramPerCubicMeter : Density("kilogram per cubic meter", "kg/m^3", NoPrefix, 1.0)

    override fun copy(): Density {
        return Density(name, symbol, prefix, convFac, exponent, isLogarithmic)
    }
}