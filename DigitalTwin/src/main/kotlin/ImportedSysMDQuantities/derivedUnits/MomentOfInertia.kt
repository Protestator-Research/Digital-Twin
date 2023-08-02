package ImportedSysMDQuantities.derivedUnits

import ImportedSysMDQuantities.DerivedUnit
import ImportedSysMDQuantities.NoPrefix
import ImportedSysMDQuantities.Prefix
import ImportedSysMDQuantities.baseUnits.*

private var siUnitSet = setOf(Length.Meter.copy(2), Mass.Kilogram.copy())

open class MomentOfInertia(name: String, symbol: String, prefix: Prefix, convFac: Double, exponent: Int = 1, isLogarithmic: Boolean = false) :
    DerivedUnit(name, symbol, prefix, "moment of inertia", siUnitSet, convFac, exponent, isLogarithmic) {

    /**
     * generate UnitObjects and add them to the UnitList
     */
    object KilogramSquareMeter : MomentOfInertia("kilogram square meter", "kg m^2", NoPrefix, 1.0)

    override fun copy(): MomentOfInertia {
        return MomentOfInertia(name, symbol, prefix, convFac, exponent, isLogarithmic)
    }
}