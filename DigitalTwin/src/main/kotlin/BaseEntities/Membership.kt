package BaseEntities

interface Membership: Relationship {
    val memberElement: Identity<Element>
        get() = target.first()


}