package BaseEntities

interface Specialization: Relationship {

    var superclass: Identity<Type>
    var subclass: Identity<Type>

    override fun clone(): Specialization
    override fun updateFrom(template: Element)
}