package ImportedSysMDQuantities.derivedUnits

import ImportedSysMDQuantities.DerivedUnit
import ImportedSysMDQuantities.NoPrefix
import ImportedSysMDQuantities.Prefix
import ImportedSysMDQuantities.baseUnits.*

private var siUnitSet = setOf(Length.Meter.copy(2), Mass.Kilogram.copy(), Time.Second.copy(-2))

open class MomentOfForce(name: String, symbol: String, prefix: Prefix, convFac: Double, exponent: Int = 1, isLogarithmic: Boolean = false) :
    DerivedUnit(name, symbol, prefix, "moment of force", siUnitSet, convFac, exponent, isLogarithmic) {

    /**
     * generate UnitObjects and add them to the UnitList
     */
    object NewtonMeter : MomentOfForce("newton meter", "Nm", NoPrefix, 1.0)

    override fun copy(): MomentOfForce {
        return MomentOfForce(name, symbol, prefix, convFac, exponent, isLogarithmic)
    }
}