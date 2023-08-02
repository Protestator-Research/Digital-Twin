package ImportedSysMDQuantities.derivedUnits

import ImportedSysMDQuantities.DerivedUnit
import ImportedSysMDQuantities.NoPrefix
import ImportedSysMDQuantities.Prefix
import ImportedSysMDQuantities.baseUnits.*

private var siUnitSet = setOf(
    Length.Meter.copy(2), Mass.Kilogram.copy(),
    Time.Second.copy(-2), ElectricCurrent.Ampere.copy(-2)
)

open class Inductance(name: String, symbol: String, prefix: Prefix, convFac: Double, exponent: Int = 1, isLogarithmic: Boolean = false) :
    DerivedUnit(name, symbol, prefix, "inductance", siUnitSet, convFac, exponent, isLogarithmic) {

    /**
     * generate UnitObjects and add them to the UnitList
     */
    object Henry : Inductance("henry", "H", NoPrefix, 1.0)

    override fun copy(): Inductance {
        return Inductance(name, symbol, prefix, convFac, exponent, isLogarithmic)
    }
}