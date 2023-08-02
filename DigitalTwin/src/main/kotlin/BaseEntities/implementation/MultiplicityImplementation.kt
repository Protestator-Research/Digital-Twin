package BaseEntities.implementation


import BaseEntities.*
import ImportedSysMDQuantities.Quantity
import com.github.tukcps.jaadd.values.IntegerRange

import java.util.*


/**
 * The multiplicity is a Feature that is an integer range or set.
 */
class MultiplicityImplementation(
    elementId: UUID = UUID.randomUUID(),
    name: String? = "multiplicity",
    owner: Identity<Element> = Identity(),
    valueSpec: IntegerRange = IntegerRange(1, 1)
): Multiplicity, ValueFeatureImplementation(
    elementId = elementId,
    name = name,
    owner = owner,
    ownedElements = mutableListOf(),
    valueSpecs = mutableListOf(valueSpec),
    unitSpec = "",
    dependency = ""
) {
    override fun resolveNames(): Boolean {
        super.resolveNames()
        return updated
    }

    override val superclass: Identity<Type>
        get() = Identity(str="ScalarValues::Integer", ref = model?.repo?.integerType, id=model?.repo?.integerType?.elementId)

    override val specialization: Specialization?
        get() = null

    /**
     * InitQuantity uses the model given as parameter to initialize the quantity
     * used for computations.
     */
    override fun initVectorQuantity(): ValueFeature {
        this.vectorQuantity = Quantity(model!!.builder.rangeIDD(intSpecs[0]))
        return this
    }

    override fun toString(): String {
        return "Multiplicity { valueSpec = $intSpecs, owner = ${owner.ref?.qualifiedName} }"
    }

    override fun updateFrom(template: Element) {
        require (template is Multiplicity)
        this.valueSpecs = template.valueSpecs
    }

    /**
     * Clone creates a copy of all fields, but NOT of the owned elements;
     * only some well-understood owned elements from the metamodel are copied (e.g. Multiplicity, ...).
     * Hence, the consistency of the model is NOT by itself preserved.
     * Function should NOT be used to copy parts of the model;
     * ONLY to create a backup or in really internal logic when there are no or well-understood owned elements.
     * TAKE CARE!
     */
    override fun clone(): Multiplicity {
        return MultiplicityImplementation(
            name=name,
            owner=Identity(owner),
            valueSpec=this.intSpecs[0]).also {
            it.model = model
            it.isTransient = this.isTransient
        }
    }
}
