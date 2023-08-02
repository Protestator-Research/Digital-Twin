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
    Time.Second.copy(-3), ElectricCurrent.Ampere.copy(-2)
)

open class ElectricalResistance(name: String, symbol: String, prefix: Prefix, convFac: Double, exponent: Int = 1, isLogarithmic: Boolean = false) :
    DerivedUnit(name, symbol, prefix, "electrical resistance", siUnitSet, convFac, exponent, isLogarithmic) {

    /**
     * generate UnitObjects and add them to the UnitList
     */
    object Ohm : ElectricalResistance("ohm", "Ω", NoPrefix, 1.0)

    override fun copy(): ElectricalResistance {
        return ElectricalResistance(name, symbol, prefix, convFac, exponent, isLogarithmic)
    }
}