package Parser

import BaseEntities.Element


/**
 * An identification of an element that consists of
 * - an uid given by the implementation, automatically when persisted
 * - a globally unique id given by the user
 * - a name that is only unique the scope of the element
 */
class Identification(
    var shortName: String? = null,    // KerML: Abbreviation of name
    var name: String? = null          // KerML: Name
): Cloneable {

    constructor(element: Element): this(element.shortName, element.name)

    override fun toString(): String =
                (if (shortName!= null) "shortName: $shortName" else "")+
                (if (name != null) " name: $name" else "")


    fun toName(): String = shortName?:name?:"(no name)"

    override fun equals(other: Any?): Boolean {
        if (other == null) return false
        if (other !is Identification) return false
        if (other.shortName == this.shortName && this.shortName != null) return true
        return other.name == this.name && this.name != null
    }

    public override fun clone(): Identification {
        return Identification(shortName, name)
    }

    override fun hashCode(): Int {
        var result = shortName?.hashCode() ?: 0
        result = 31 * result + (name?.hashCode() ?: 0)
        return result
    }
}