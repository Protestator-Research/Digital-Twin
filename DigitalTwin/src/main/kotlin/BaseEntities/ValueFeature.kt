@file:Suppress("UNCHECKED_CAST")

package BaseEntities


import ImportedSysMDQuantities.VectorQuantity
import Parser.size
import SysMDCopyAst.AstRoot
import com.github.tukcps.jaadd.AADD
import com.github.tukcps.jaadd.BDD
import com.github.tukcps.jaadd.IDD
import com.github.tukcps.jaadd.values.IntegerRange
import com.github.tukcps.jaadd.values.Range
import com.github.tukcps.jaadd.values.XBool

/**
 * The ValueFeature is a replacement for the ValueFeature class (Versions before 2.6.15).
 * ValueFeature combines the KerML metamodel classes in a single class:
 * - Feature that models a value.
 * - Expression that models some expressions.
 * - ValueFeature, a relationship that links the value of a Feature with an Expression.
 */
interface ValueFeature: Expression {

    /** Holds either an IntegerRange, a Range, or an XBool, depending on the type. */
    var valueSpecs:  MutableList<Any?>
    var unitSpec:   String       // Specified unit as string

    /*  For quantities, constraints, performances: an equation as text, right side of eqn for parser! */
    var dependency:   String     // equation as text, right side of eqn for parser!

    /** access methods for the valueSpec field; returns different types */
    val rangeSpecs: MutableList<Range>
        get() = if(valueStr.size()>0) valueSpecs as MutableList<Range> else mutableListOf(Range.Reals)

    val boolSpecs: MutableList<XBool>
        get() = if(valueStr.size()>0) valueSpecs as MutableList<XBool> else mutableListOf(XBool.X)

    val intSpecs: MutableList<IntegerRange>
        get() = if(valueStr.size()>0) valueSpecs as MutableList<IntegerRange> else mutableListOf(IntegerRange.Integers)

    /** A getter for a string representation of the value, with field for serialization. */
    var valueStr: String

    /** indicator for constraint propagation that shows stability in iterations. */
    var stable: Boolean                 // For use in numerical iterations

    /** value and unit for constraint propagation */
    var vectorQuantity : VectorQuantity   // actually possible values; intersection of up/downValue

    val isVectorQuantityInitialized: Boolean

    /** old value and unit for constraint propagation, to detect stability */
    var oldVectorQuantity: VectorQuantity?   // previous quantity for event detection

    /** The AstRoot for computation of the ValueFeature. */
    var ast: AstRoot?

    var textualRepresentation: TextualRepresentation?

    var isMeasurable:Boolean

    var isInput:Boolean

    var isOutput:Boolean

    /**
     * This method initializes the transient fields based on the specified value and unit
     */
    fun initVectorQuantity(): ValueFeature
    override fun resolveNames(): Boolean

    /** Setter from a boolean string representation */
    fun boolSpec(str: String?): ValueFeature

    /** Setter for a real ValueFeature */
    fun rangeSpec(lbs: String?, ubs: String?): ValueFeature

    /** Setter for a real ValueFeature */
    fun rangeSpec(lb: Double?, ub: Double?): ValueFeature

    /** Setter for a real ValueFeature */
    fun rangeSpec(init : Range): ValueFeature

    /** Setter for range specification that also initializes the value */
    fun intSpec(init: IntegerRange) : ValueFeature

    /** Setter for the unit specification that also initializes the unit */
    fun unitSpec(str: String?): ValueFeature

    fun dependency(str: String?): ValueFeature

    /**
     * Clone creates a copy of all fields, but NOT of the owned elements;
     * only some well-understood owned elements from the metamodel are copied (e.g. Multiplicity, Specialization).
     * Hence, the consistency of the model is NOT by itself preserved.
     * Function should NOT be used to copy parts of the model;
     * ONLY to create a backup or in really internal logic when there are no or well-understood owned elements.
     * TAKE CARE!
     */
    override fun clone(): ValueFeature

    /** Casts the quantity value to AADD and returns it. */
    fun aadd(): AADD
    fun bdd(): BDD
    fun idd(): IDD
    fun min(): Double
    fun max(): Double
    override fun updateFrom(template: Element)
}
