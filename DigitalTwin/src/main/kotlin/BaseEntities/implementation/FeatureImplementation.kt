package BaseEntities.implementation

import BaseEntities.*
import Parser.SimpleName
import com.github.tukcps.jaadd.values.IntegerRange
import java.util.*


/**
 * A feature definition incl. multiplicity, as in KerML (mostly).
 * The relationship can refer to an element, or to a property.
 * - multiplicity
 * - instances
 * - ofClass
 * - direction (in, out, inout)
 */
open class FeatureImplementation(
    elementId: UUID = UUID.randomUUID(),
    name: SimpleName? = null,
    shortName: String? = null,
    ownedElements: MutableList<Identity<Element>> = mutableListOf(),
    owner: Identity<Element> = Identity(),
    override var direction: Feature.FeatureDirectionKind = Feature.FeatureDirectionKind.IN,
    override var isEnd: Boolean = false
): Feature, TypeImplementation(
    elementId=elementId, name=name, shortName=shortName, ownedElements, owner
){
    override val ofClass: Identity<Type>?
        get() = superclass

    /** Getter and setter for the owned Multiplicity (if any). */
    override val multiplicityProperty: Multiplicity?
        get() = getOwnedElementOfType()


    /** Getter and setter for the specified multiplicity. */
    override var multiplicity: IntegerRange
        get() = multiplicityProperty?.intSpecs?.get(0) ?: IntegerRange(1, 1 )
        set(value) { multiplicityProperty?.valueSpecs = mutableListOf(value)}

    /**
     * Initialize searches for (qualified) names in the element and
     * adds UId where the search was successful or reports an error where not.
     */
    override fun resolveNames(): Boolean {
        //
        // We also need to add a multiplicity as property.
        // Add multiplicity as owned property is done in create-function of AgilaSession ! ! !
        //
        updated = super.resolveNames() or updated
        return updated
    }

    /**
     * Just string representation for debugging.
     */
    override fun toString() =
        "Feature(name=${name}, ofClass=${ofClass}, multiplicity=${multiplicity}, id='$elementId'}"

    override fun clone(): Feature {
        return FeatureImplementation(
            name = name,
            shortName = shortName,
            ownedElements = Identity.copyOfIdentityList(ownedElements),
            owner = Identity(owner)
        ).also { klon ->
            klon.model = model
            klon.direction = direction
            klon.updated = updated
        }
    }

    override var expression: Expression? = null

    override fun updateFrom(template: Element) {
        if (template is Feature) {
            direction = template.direction
            if (template.getOwnedElementOfType<Multiplicity>() != null)
                multiplicity = template.multiplicity
            super.updateFrom(template)
        }
    }
}
