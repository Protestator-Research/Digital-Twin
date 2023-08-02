package ImportedSysMDQuantities.derivedUnits

import ImportedSysMDQuantities.DerivedUnit
import ImportedSysMDQuantities.NoPrefix
import ImportedSysMDQuantities.Prefix
import ImportedSysMDQuantities.baseUnits.*

private var siUnitSet = setOf(Mass.Kilogram.copy(), Time.Second.copy(-2), ElectricCurrent.Ampere.copy(-1))

open class MagneticFluxDensity(name: String, symbol: String, prefix: Prefix, convFac: Double, exponent: Int = 1, isLogarithmic: Boolean = false) :
    DerivedUnit(name, symbol, prefix, "magnetic flux density", siUnitSet, convFac, exponent, isLogarithmic) {

    /**
     * generate UnitObjects and add them to the UnitList
     */
    object Tesla : MagneticFluxDensity("tesla", "T", NoPrefix, 1.0)

    override fun copy(): MagneticFluxDensity {
        return MagneticFluxDensity(name, symbol, prefix, convFac, exponent, isLogarithmic)
    }
}