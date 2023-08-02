package ImportedSysMDQuantities.derivedUnits

import ImportedSysMDQuantities.DerivedUnit
import ImportedSysMDQuantities.NoPrefix
import ImportedSysMDQuantities.Prefix
import ImportedSysMDQuantities.baseUnits.*

private var siUnitSet = setOf(Length.Meter.copy(2), Mass.Kilogram.copy(), Time.Second.copy(-3))

open class Power(name: String, symbol: String, prefix: Prefix, convFac: Double, exponent: Int = 1, isLogarithmic: Boolean = false) :
    DerivedUnit(name, symbol, prefix, "power", siUnitSet, convFac, exponent, isLogarithmic) {

    /**
     * generate UnitObjects and add them to the UnitList
     */
    object Watt : Power("watt", "W", NoPrefix, 1.0)
    object Horsepower : Power("horsepower", "HP", NoPrefix, 735.499)

    override fun copy(): Power {
        return Power(name, symbol, prefix, convFac, exponent, isLogarithmic)
    }
}