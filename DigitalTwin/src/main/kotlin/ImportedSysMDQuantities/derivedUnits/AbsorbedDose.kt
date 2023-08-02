package ImportedSysMDQuantities.derivedUnits

import ImportedSysMDQuantities.DerivedUnit
import ImportedSysMDQuantities.NoPrefix
import ImportedSysMDQuantities.Prefix
import ImportedSysMDQuantities.baseUnits.Length
import ImportedSysMDQuantities.baseUnits.Time


private var siUnitSet = setOf(Length.Meter.copy(2), Time.Second.copy(-2))

open class AbsorbedDose(name: String, symbol: String, prefix: Prefix, convFac: Double, exponent: Int = 1, isLogarithmic: Boolean = false) :
    DerivedUnit(name, symbol, prefix, "absorbed dose", siUnitSet, convFac, exponent, isLogarithmic) {

    /**
     * generate UnitObjects and add them to the UnitList
     */
    object Gray : AbsorbedDose("gray", "Gy", NoPrefix, 1.0)

    override fun copy(): AbsorbedDose {
        return AbsorbedDose(name, symbol, prefix, convFac, exponent, isLogarithmic)
    }
}