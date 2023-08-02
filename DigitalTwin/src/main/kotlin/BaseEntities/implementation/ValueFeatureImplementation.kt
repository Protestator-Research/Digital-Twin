package BaseEntities.implementation

import BaseEntities.*
import ImportedSysMDExceptions.PropertyError
import ImportedSysMDQuantities.Quantity
import ImportedSysMDQuantities.VectorQuantity
import ImportedSysMDServices.report
import SysMDCopyAst.AstRoot
import com.github.tukcps.jaadd.*
import com.github.tukcps.jaadd.values.IntegerRange
import com.github.tukcps.jaadd.values.Range
import com.github.tukcps.jaadd.values.XBool


import java.util.*


@Suppress("UNCHECKED_CAST")
open class ValueFeatureImplementation (
    elementId: UUID = UUID.randomUUID(),
    name: String? = null,
    ownedElements: MutableList<Identity<Element>> = mutableListOf(),
    owner: Identity<Element> = Identity(),
    shortName: String? = null,
    direction: Feature.FeatureDirectionKind = Feature.FeatureDirectionKind.IN,
    isEnd: Boolean = false,
    /**
     * Holds either an IntegerRange, a Range, or an XBool, depending on the type.
     */
    override var valueSpecs: MutableList<Any?> = mutableListOf(),
    override var unitSpec:   String = "",        // Specified unit as string

    /*  For quantities, constraints, performances: an equation as text, right side of eqn for parser! */
    override var dependency:   String = "",      // equation as text, right side of eqn for parser!
): ValueFeature, FeatureImplementation(
    elementId = elementId, name = name, shortName = shortName,
    ownedElements = ownedElements, owner = owner,
    direction = direction, isEnd = isEnd) {

    /** access methods for the valueSpec field; returns different types */
    override val rangeSpecs: MutableList<Range>
        get() = if(valueSpecs.size>0 && valueSpecs[0]!=null) valueSpecs as MutableList<Range> else mutableListOf(Range.Reals)

    override val boolSpecs: MutableList<XBool>
        get() {
            if (valueSpecs[0] is XBool)
                return if(valueSpecs[0]!=null) valueSpecs as MutableList<XBool> else mutableListOf(XBool.X)
            else
                throw Exception("Invalid bool spec")
        }

    override val intSpecs: MutableList<IntegerRange>
        get() = if(valueSpecs.size>0 && valueSpecs[0]!=null) valueSpecs as MutableList<IntegerRange> else mutableListOf( IntegerRange.Integers)

    /** A getter for a string representation of the value, with field for serialization. */
    override var valueStr: String = ""
        get() {
            field = vectorQuantity.values.toString()
            return field
        }

    /** indicator for constraint propagation that shows stability in iterations. */
    override var stable: Boolean = false          // For use in numerical iterations

    /** value and unit for constraint propagation */
    final override lateinit var vectorQuantity : VectorQuantity   // actually possible values; intersection of up/downValue

    override val isVectorQuantityInitialized: Boolean by lazy { this::vectorQuantity.isInitialized }

    /** old value and unit for constraint propagation, to detect stability */
    final override var oldVectorQuantity: VectorQuantity? = null  // previous VectorQuantity for event detection

    /** The AstRoot for computation of the ValueFeature. */
    override var ast: AstRoot? = null

    override var textualRepresentation: TextualRepresentation? = null
    override var isMeasurable: Boolean = false
    override var isInput: Boolean = false
    override var isOutput: Boolean = false

    constructor(name : String, quantity : Quantity): this(name = name) {
        this.vectorQuantity = quantity
        this.oldVectorQuantity = quantity.clone()
    }

    /**
     * This method initializes the transient fields based on the specified value and unit
     */
    override fun initVectorQuantity(): ValueFeature {
        if (model == null)
            throw PropertyError("INTERNAL: Attempt to initialize Quantity before model")
        else
            when  {
                ofClass?.ref !is Classifier -> {
                    model!!.report(this,"expected subclass of ScalarValue, but got: '${ofClass?.str}'")
                    valueSpecs = mutableListOf(Range.Reals)
                    val values = mutableListOf<AADD>()
                    rangeSpecs.forEach{values.add(model!!.builder.range(it,elementId.toString()))}
                    vectorQuantity = VectorQuantity(values, unitSpec)
                }

                ofClass?.ref?.isSubclassOf(model!!.repo.realType) == true -> {
                    if (valueSpecs.size>0 && valueSpecs[0] is String?) {
                        valueSpecs = if (valueSpecs[0] == "Real" || valueSpecs[0] == null) {
                            mutableListOf(Range.Reals)
                        } else {
                            val ranges = mutableListOf<Any?>()
                            valueSpecs.forEach { ranges.add(Range(it as String)) }
                            ranges
                        }
                    }
                    val values = mutableListOf<AADD>()
                    rangeSpecs.forEach{values.add(model!!.builder.range(it,elementId.toString()))}
                    vectorQuantity = VectorQuantity(values, unitSpec)
                }

                ofClass?.ref?.isSubclassOf(model!!.repo.integerType) == true-> {
                    if (valueSpecs.size>0 && valueSpecs[0] is String) {
                        val ranges = mutableListOf<Any?>()
                        valueSpecs.forEach { ranges.add(IntegerRange(it as String)) }
                        valueSpecs = ranges
                    }
                    val values = mutableListOf<IDD>()
                    intSpecs.forEach{ values.add(model!!.builder.rangeIDD(it)) }
                    vectorQuantity = VectorQuantity(values)
                }

                ofClass?.ref?.isSubclassOf(model!!.repo.booleanType) == true -> {

                    val values = mutableListOf<BDD>()
                    val ranges = mutableListOf<Any?>()
                    if(valueSpecs.isEmpty()){
                        ranges.add(XBool.X)
                        values.add(model!!.builder.variable(elementId.toString()))
                    }
                    valueSpecs.forEach {
                        when (it.toString()) {
                            "True", "true" -> {
                                ranges.add(XBool.True)
                                values.add(model!!.builder.True)
                            }

                            "False", "false" -> {
                                ranges.add(XBool.False)
                                values.add(model!!.builder.False)
                            }

                            "null", "X", "Unknown" -> {
                                ranges.add(XBool.X)
                                values.add(model!!.builder.variable(elementId.toString()))
                            }

                            else ->  // throw Error("Forbidden boolSpec value in ValueFeature $id: $valueSpec")
                            {
                                ranges.add(XBool.X)
                                values.add(model!!.builder.variable(elementId.toString()))
                            }
                        }
                    }
                    valueSpecs = ranges
                    vectorQuantity = VectorQuantity(values)
                }
                ofClass?.ref?.isSubclassOf(model!!.repo.stringType) == true ->
                    vectorQuantity = Quantity(model!!.builder.string(dependency)) // For String there is no Vector available

                else -> {
                    model!!.report(this, "no type found for value $qualifiedName; assuming Real")
                    vectorQuantity = Quantity(model!!.builder.Reals, unitSpec)
                }
            }
        stable = false
        updated = true
        oldVectorQuantity = vectorQuantity.clone()
        return this
    }

    override fun resolveNames(): Boolean {
        updated = super.resolveNames() or updated
        return updated
    }


    /** Setter from a boolean string representation */
    override fun boolSpec(str: String?): ValueFeature {
        updated = true
        stable = false
        if (str == null) return this
        when(str.toString().trim().lowercase(Locale.US)) {
            "true"          -> valueSpecs = mutableListOf(XBool.True)
            "false"         -> valueSpecs = mutableListOf(XBool.False)
            "x", "unknown"  -> valueSpecs = mutableListOf(XBool.X)
            "nab"           -> valueSpecs = mutableListOf(XBool.NaB)
            ""              -> { /* no update */ }
            else -> throw PropertyError("boolean constraint must be true, false or x/unknown.")
        }
        return this
    }

    /** Setter for a real ValueFeature */
    override fun rangeSpec(lbs: String?, ubs: String?): ValueFeature {
        updated = true
        if (lbs == null || ubs == null) return this

        var lb = -Double.MAX_VALUE
        var ub = Double.MAX_VALUE

        if (lbs.isNotBlank()) lb = lbs.toDouble()
        if (ubs.isNotBlank()) ub = ubs.toDouble()

        if (lb > ub)
            throw PropertyError("in range subtype constraint, lower bound must be less or equal upper bound.")
        valueSpecs = mutableListOf(Range(lb, ub))
        return this
    }

    /** Setter for a real ValueFeature */
    override fun rangeSpec(lb: Double?, ub: Double?): ValueFeature {
        updated = true
        if (lb == null || ub == null) return this
        if (lb > ub)
            throw PropertyError("in range subtype constraint, lower bound must be less or equal upper bound.")
        valueSpecs = mutableListOf(Range(lb, ub))
        return this
    }

    /** Setter for a real ValueFeature */
    override fun rangeSpec(init : Range): ValueFeature {
        updated = true
        if (init.min > init.max)
            throw PropertyError("in range subtype constraint, lower bound must be less or equal upper bound.")
        valueSpecs = mutableListOf(Range(init.min, init.max))
        return this
    }


    /** Setter for range specification that also initializes the value */
    override fun intSpec(init: IntegerRange) : ValueFeature {
        updated = true
        if (init.min > init.max)
            throw PropertyError("in integer range subtype constraint, lower bound must be less or equal upper bound.")
        valueSpecs = mutableListOf(IntegerRange(init.min, init.max))
        return this
    }


    /** Setter for the unit specification that also initializes the unit */
    override fun unitSpec(str: String?): ValueFeature {
        updated = true
        if (str != null) {
            unitSpec = str
        }
        return this
    }

    override fun dependency(str: String?): ValueFeature {
        updated = true
        dependency = str ?: ""
        return this
    }


    /**
     * Clone creates a copy of all fields, but NOT of the owned elements;
     * only some well-understood owned elements from the metamodel are copied (e.g. Multiplicity, ...).
     * Hence, the consistency of the model is NOT by itself preserved.
     * Function should NOT be used to copy parts of the model;
     * ONLY to create a backup or in really internal logic when there are no or well-understood owned elements.
     * TAKE CARE!
     */
    override fun clone(): ValueFeature {
        return ValueFeatureImplementation(
            name = name,
            shortName = shortName,
            ownedElements = Identity.copyOfIdentityList(ownedElements),
            owner = Identity(owner),
            dependency = dependency.plus(""),
            valueSpecs = valueSpecs,
            unitSpec = unitSpec.plus(""),
        ).also { klon ->
            klon.model = model
            klon.updated = updated
            klon.isTransient = isTransient
        }
    }

    /**
     * Casts the quantity value to AADD and returns it.
     */
    override fun aadd(): AADD {
        if (vectorQuantity.values[0] is AADD) return vectorQuantity.valuesIn(unitSpec)[0] as AADD
        else throw DDException("ValueFeature value cannot be cast to AADD")
    }

    override fun bdd(): BDD {
        if (vectorQuantity.values[0] is BDD) return vectorQuantity.values[0] as BDD
        else
            throw DDException("ValueFeature value cannot be cast to BDD")
    }

    override fun idd(): IDD {
        if (vectorQuantity.values[0] is IDD) return vectorQuantity.values[0] as IDD
        else throw DDException("ValueFeature value cannot be cast to IDD")
    }


    /** Creates a compact string, skipping fields not relevant, incl. doc */
    override fun toString(): String =
        "ValueFeature { qualifiedName=$qualifiedName, ofClass=$ofClass" +
                when {
                    ofClass?.ref == null -> { }
                    ofClass?.ref!!.isSubclassOf(model?.repo?.realType) -> {
                        ", rangeSpec=$valueSpecs,  values=${vectorQuantity.values}" +
                                (if (unitSpec.isNotEmpty()) ", unitSpec=$unitSpec" else "")+
                                // (", quantity=${quantity.transformTo(Unit(unitSpec))}") +
                                if (dependency.isNotEmpty()) ", dependency=\"$dependency\"" else ""
                    }

                    ofClass?.ref!!.isSubclassOf(model?.repo?.booleanType) -> {
                        ", boolSpec=$valueSpecs" +
                                // ", value=$quantity.value, " +
                                if (dependency.isNotEmpty()) ", dependency=\"$dependency\"" else ""
                    }

                    ofClass?.ref!!.isSubclassOf(model?.repo?.integerType) -> {
                        ", intSpec=$valueSpecs" +
                                // (", quantity=$quantity") +
                                (if (dependency.isNotEmpty()) ", dependency=\"$dependency\"" else "")
                    }

                    ofClass?.ref!!.isSubclassOf(model?.repo?.stringType) -> {
                        ", dependency=\"$dependency\""
                    }
                    else -> "Unidentified type: ${ofClass?.str}"
                } + ", stable=$stable, updated=$updated, id=${elementId} }"

    /**
     * Returns min of first value of VectorQuantity
     */
    override fun min(): Double =
        when (vectorQuantity.values[0]) {
            is AADD -> (vectorQuantity.valuesIn(unitSpec)[0] as AADD).getRange().min
            is IDD -> (vectorQuantity.valuesIn(unitSpec)[0] as IDD).getRange().min.toDouble()
            else -> throw InternalError(".min can only be applied on properties of type Integer or Real")
        }

    /**
     * Returns max of first value of VectorQuantity
     */
    override fun max(): Double =
        when (vectorQuantity.values[0]) {
            is AADD -> (vectorQuantity.valuesIn(unitSpec)[0] as AADD).getRange().max
            is IDD -> (vectorQuantity.valuesIn(unitSpec)[0] as IDD).getRange().max.toDouble()
            else -> throw InternalError(".max can only be applied on properties of type Integer or Real")
        }

    override fun updateFrom(template: Element) {
        if (template is ValueFeature) {
            valueSpecs = template.valueSpecs
            unitSpec = template.unitSpec
            dependency = template.dependency
        }
        super.updateFrom(template)
    }
}
