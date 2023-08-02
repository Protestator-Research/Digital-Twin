package BaseEntities


interface Comment : AnnotatingElement {
    override fun clone(): Comment
    override fun toString(): String
    var locale: String?
    // override fun updateFrom(template: Element)
}
