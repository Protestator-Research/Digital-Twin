package BaseEntities


/**
 * Connector / Binary link
 * connector c : A from f1 to f2;
 */
interface Connector: Relationship, Feature {
    var from: MutableList<Identity<Element>>
    var to: MutableList<Identity<Element>>
    override fun clone(): Connector
    override fun updateFrom(template: Element)
    override fun resolveNames(): Boolean
}