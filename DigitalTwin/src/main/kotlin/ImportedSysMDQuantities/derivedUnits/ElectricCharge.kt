package ImportedSysMDQuantities.derivedUnits

import ImportedSysMDQuantities.DerivedUnit
import ImportedSysMDQuantities.NoPrefix
import ImportedSysMDQuantities.Prefix
import ImportedSysMDQuantities.baseUnits.ElectricCurrent
import ImportedSysMDQuantities.baseUnits.Time

private var siUnitSet = setOf(Time.Second.copy(), ElectricCurrent.Ampere.copy())

open class ElectricCharge(name: String, symbol: String, prefix: Prefix, convFac: Double, exponent: Int = 1, isLogarithmic: Boolean = false) :
    DerivedUnit(name, symbol, prefix, "electric charge", siUnitSet, convFac, exponent, isLogarithmic) {

    /**
     * generate UnitObjects and add them to the UnitList
     */
    object Coulomb : ElectricCharge("coulomb", "C", NoPrefix, 1.0)
    object AmpereSecond : ElectricCharge("ampere second", "As", NoPrefix, 1.0)
    object AmpereHours : ElectricCharge("ampere hour", "Ah", NoPrefix, 3600.0)

    override fun copy(): ElectricCharge {
        return ElectricCharge(name, symbol, prefix, convFac, exponent, isLogarithmic)
    }
}