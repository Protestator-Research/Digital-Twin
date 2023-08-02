package ImportedSysMDQuantities.derivedUnits

import ImportedSysMDQuantities.BaseUnit
import ImportedSysMDQuantities.DerivedUnit
import ImportedSysMDQuantities.NoPrefix
import ImportedSysMDQuantities.Prefix
import ImportedSysMDQuantities.baseUnits.*

private var siUnitSet = emptySet<BaseUnit>()

open class SolidAngle(name: String, symbol: String, prefix: Prefix, convFac: Double, exponent: Int = 1, isLogarithmic: Boolean = false) :
    DerivedUnit(name, symbol, prefix, "solid angle", siUnitSet, convFac, exponent, isLogarithmic) {

    /**
     * generate UnitObjects and add them to the UnitList
     */
    object Steradian : Pressure("steradian", "sr", NoPrefix, 1.0)

    override fun copy(): SolidAngle {
        return SolidAngle(name, symbol, prefix, convFac, exponent, isLogarithmic)
    }
}