package BaseEntities

import com.github.tukcps.jaadd.values.IntegerRange


/**
 * A feature definition incl. multiplicity, as in KerML (mostly).
 * The relationship can refer to an element, or to a property.
 * - multiplicity
 * - instances
 * - ofClass
 * - direction (in, out, inout)
 */
interface Feature: Type {

    // via owned Specialization Element
    val ofClass: Identity<Type>?

    enum class FeatureDirectionKind { IN, OUT, INOUT}
    var direction: FeatureDirectionKind
    
    /** Getter and setter for the owned Multiplicity (if any). */
    val multiplicityProperty: Multiplicity?

    /**
     * Getter and setter for the specified multiplicity; via
     * the owned Multiplicity element.
     */
    val multiplicity: IntegerRange

    /**
     * Variable that is true if the feature constrains source/target of
     * a relationship.
     */
    var isEnd: Boolean

    /**
     * Initialize searches for (qualified) names in the element and
     * adds UId where the search was successful or reports an error where not.
     */
    override fun resolveNames(): Boolean
    override fun clone(): Feature

    /**
     * Features can 'have' a value which is determined by an owned
     * expression.
     */
    var expression: Expression? // TODO: Model properly as owned Feature
}
