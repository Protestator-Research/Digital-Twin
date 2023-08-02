package ImportedSysMDQuantities.derivedUnits

import ImportedSysMDQuantities.DerivedUnit
import ImportedSysMDQuantities.NoPrefix
import ImportedSysMDQuantities.Prefix
import ImportedSysMDQuantities.baseUnits.LuminousIntensity
import ImportedSysMDQuantities.baseUnits.Time


private var siUnitSet = setOf(LuminousIntensity.Candela.copy(), Time.Second.copy())

open class LuminousEnergy(name: String, symbol: String, prefix: Prefix, convFac: Double, exponent: Int = 1, isLogarithmic: Boolean = false) :
    DerivedUnit(name, symbol, prefix, "luminous energy", siUnitSet, convFac, exponent, isLogarithmic) {

    /**
     * generate UnitObjects and add them to the UnitList
     */
    object LumenSecond : LuminousEnergy("luminous energy", "lms", NoPrefix, 1.0)

    override fun copy(): LuminousEnergy {
        return LuminousEnergy(name, symbol, prefix, convFac, exponent, isLogarithmic)
    }
}