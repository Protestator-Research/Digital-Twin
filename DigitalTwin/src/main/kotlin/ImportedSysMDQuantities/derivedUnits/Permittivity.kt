package ImportedSysMDQuantities.derivedUnits

import ImportedSysMDQuantities.DerivedUnit
import ImportedSysMDQuantities.NoPrefix
import ImportedSysMDQuantities.Prefix
import ImportedSysMDQuantities.baseUnits.*

private var siUnitSet = setOf(
    Time.Second.copy(4), ElectricCurrent.Ampere.copy(2),
    Length.Meter.copy(-3), Mass.Kilogram.copy(-1)
)

open class Permittivity(name: String, symbol: String, prefix: Prefix, convFac: Double, exponent: Int = 1, isLogarithmic: Boolean = false) :
    DerivedUnit(name, symbol, prefix, "permittivity", siUnitSet, convFac, exponent, isLogarithmic) {

    /**
     * generate UnitObjects and add them to the UnitList
     */
    object FaradPerSecond : Permittivity("farad per meter", "F/m", NoPrefix, 1.0)

    override fun copy(): Permittivity {
        return Permittivity(name, symbol, prefix, convFac, exponent, isLogarithmic)
    }
}