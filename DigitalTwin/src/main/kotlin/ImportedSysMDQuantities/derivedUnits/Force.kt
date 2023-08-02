package ImportedSysMDQuantities.derivedUnits

import ImportedSysMDQuantities.DerivedUnit
import ImportedSysMDQuantities.NoPrefix
import ImportedSysMDQuantities.Prefix
import ImportedSysMDQuantities.baseUnits.Length
import ImportedSysMDQuantities.baseUnits.Mass
import ImportedSysMDQuantities.baseUnits.Time

private var siUnitSet = setOf(Length.Meter.copy(), Mass.Kilogram.copy(), Time.Second.copy(-2))

open class Force(name: String, symbol: String, prefix: Prefix, convFac: Double, exponent: Int = 1, isLogarithmic: Boolean = false) :
    DerivedUnit(name, symbol, prefix, "force", siUnitSet, convFac, exponent, isLogarithmic) {

    /**
     * generate UnitObjects and add them to the UnitList
     */
    object Newton : Force("newton", "N", NoPrefix, 1.0)

    override fun copy(): Force {
        return Force(name, symbol, prefix, convFac, exponent, isLogarithmic)
    }
}