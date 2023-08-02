package ImportedSysMDQuantities.derivedUnits

import ImportedSysMDQuantities.DerivedUnit
import ImportedSysMDQuantities.NoPrefix
import ImportedSysMDQuantities.Prefix
import ImportedSysMDQuantities.baseUnits.AmountOfSubstance
import ImportedSysMDQuantities.baseUnits.Time


private var siUnitSet = setOf(AmountOfSubstance.Mole.copy(), Time.Second.copy(-1))

open class CatalyticActivity(name: String, symbol: String, prefix: Prefix, convFac: Double, exponent: Int = 1, isLogarithmic: Boolean = false) :
    DerivedUnit(name, symbol, prefix, "catalytic activity", siUnitSet, convFac, exponent, isLogarithmic) {

    /**
     * generate UnitObjects and add them to the UnitList
     */
    object Katal : CatalyticActivity("katal", "kat", NoPrefix, 1.0)

    override fun copy(): CatalyticActivity {
        return CatalyticActivity(name, symbol, prefix, convFac, exponent, isLogarithmic)
    }
}