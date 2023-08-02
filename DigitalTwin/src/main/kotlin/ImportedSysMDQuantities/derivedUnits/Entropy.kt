package ImportedSysMDQuantities.derivedUnits

import ImportedSysMDQuantities.DerivedUnit
import ImportedSysMDQuantities.NoPrefix
import ImportedSysMDQuantities.Prefix
import ImportedSysMDQuantities.baseUnits.Length
import ImportedSysMDQuantities.baseUnits.Mass
import ImportedSysMDQuantities.baseUnits.Temperature
import ImportedSysMDQuantities.baseUnits.Time

private var siUnitSet = setOf(
    Length.Meter.copy(2), Mass.Kilogram.copy(),
    Time.Second.copy(-2), Temperature.Kelvin.copy(-1)
)

open class Entropy(name: String, symbol: String, prefix: Prefix, convFac: Double, exponent: Int = 1, isLogarithmic: Boolean = false) :
    DerivedUnit(name, symbol, prefix, "entropy", siUnitSet, convFac, exponent, isLogarithmic) {

    /**
     * generate UnitObjects and add them to the UnitList
     */
    object JoulePerKelvin : Entropy("joule per kelvin", "J/K", NoPrefix, 1.0)

    override fun copy(): Entropy {
        return Entropy(name, symbol, prefix, convFac, exponent, isLogarithmic)
    }
}