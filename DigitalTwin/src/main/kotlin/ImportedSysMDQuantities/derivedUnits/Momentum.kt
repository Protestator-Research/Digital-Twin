package ImportedSysMDQuantities.derivedUnits

import ImportedSysMDQuantities.DerivedUnit
import ImportedSysMDQuantities.NoPrefix
import ImportedSysMDQuantities.Prefix
import ImportedSysMDQuantities.baseUnits.*

private var siUnitSet = setOf(Mass.Kilogram.copy(), Length.Meter.copy(), Time.Second.copy(-1))

open class Momentum(name: String, symbol: String, prefix: Prefix, convFac: Double, exponent: Int = 1, isLogarithmic: Boolean = false) :
    DerivedUnit(name, symbol, prefix, "momentum", siUnitSet, convFac, exponent, isLogarithmic) {

    /**
     * generate UnitObjects and add them to the UnitList
     */
    object KilogramMeterPerSecond : Momentum("kilogram meter per second", "kg m/s", NoPrefix, 1.0)

    override fun copy(): Momentum {
        return Momentum(name, symbol, prefix, convFac, exponent, isLogarithmic)
    }
}