package ImportedSysMDQuantities.derivedUnits

import ImportedSysMDQuantities.DerivedUnit
import ImportedSysMDQuantities.NoPrefix
import ImportedSysMDQuantities.Prefix
import ImportedSysMDQuantities.baseUnits.*

private var siUnitSet = setOf(Mass.Kilogram.copy(), Length.Meter.copy(-1), Time.Second.copy(-2))

open class Pressure(name: String, symbol: String, prefix: Prefix, convFac: Double, exponent: Int = 1, isLogarithmic: Boolean = false) :
    DerivedUnit(name, symbol, prefix, "pressure", siUnitSet, convFac, exponent, isLogarithmic) {

    /**
     * generate UnitObjects and add them to the UnitList
     */
    object Pascal : Pressure("pascal", "Pa", NoPrefix, 1.0)
    object Bar : Pressure("bar", "bar", NoPrefix, 100000.0)
    object PoundPerSquareInch : Pressure("psi", "bar", NoPrefix, 6894.757293)

    override fun copy(): Pressure {
        return Pressure(name, symbol, prefix, convFac, exponent, isLogarithmic)
    }
}