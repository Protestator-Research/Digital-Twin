package ImportedSysMDQuantities.derivedUnits

import ImportedSysMDQuantities.DerivedUnit
import ImportedSysMDQuantities.NoPrefix
import ImportedSysMDQuantities.Prefix
import ImportedSysMDQuantities.baseUnits.ElectricCurrent
import ImportedSysMDQuantities.baseUnits.Length
import ImportedSysMDQuantities.baseUnits.Mass
import ImportedSysMDQuantities.baseUnits.Time

private var siUnitSet = setOf(
    Length.Meter.copy(2), Mass.Kilogram.copy(),
    Time.Second.copy(-3), ElectricCurrent.Ampere.copy(-1)
)

open class ElectricPotential(name: String, symbol: String, prefix: Prefix, convFac: Double, exponent: Int = 1, isDifference: Boolean = false) :
    DerivedUnit(name, symbol, prefix, "electric potential", siUnitSet, convFac, exponent, isDifference = isDifference) {

    /**
     * generate UnitObjects and add them to the UnitList
     */
    object Volt : ElectricPotential("volt", "V", NoPrefix, 1.0, isDifference = true)

    override fun copy(): ElectricPotential {
        return ElectricPotential(name, symbol, prefix, convFac, exponent, isLogarithmic)
    }
}