package BaseEntities



/**
 * Following KerML Specification, 7.2.2.2.2 Relationships;
 * Represents the SysMD relationship source relation and inherits ElementImplementation
 */
interface Relationship: Element {
    var source: MutableList<Identity<Element>>
    var target: MutableList<Identity<Element>>

    /**
     * Initialize resolves the QualifiedNames and/or uid and adds references and uid to Elements.
     */
    override fun resolveNames(): Boolean
    override fun clone(): Relationship
    override fun updateFrom(template: Element)
}
