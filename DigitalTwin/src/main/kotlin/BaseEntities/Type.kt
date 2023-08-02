package BaseEntities


/**
 * Interface of a Type and implementation of functions on this interface.
 */
interface Type: Namespace {

    /**
     * The superclass of an element.
     * It is persisted as an owned element of type Specialization.
     */
    val superclass: Identity<Type>?
        get() = specialization?.superclass

    val specialization: Specialization?
        get() = getOwnedElementOfType()


    override fun resolveNames(): Boolean {
        updated = if (specialization == null)
            false
        else
            superclass?.resolveIdentity(owningNamespace!!) == true or updated
        return updated
    }


    /**
     * Checks if a classifier this is a subclass of another class.
     * @param other the other class
     * @return true, if this is a subclass of other, else false.
     */
    fun isSubclassOf(other: Type?): Boolean {
        if (other === this) return true
        if (other is Anything) return false
        if (this is Anything) return false
        if (superclass?.ref === null) return false
        return (superclass!!.ref!!).isSubclassOf(other)
    }


    /**
     * Checks if a classifier this is a superclass of another class.
     * @param other the other class
     * @return true, if this is a superclass of other, else false.
     */
    fun isSuperclassOf(other: Type?): Boolean {
        if (other === this) return true
        if (other is Anything) return false
        if (this is Anything) return false
        return if (other?.superclass?.ref is Type)
            isSuperclassOf(other.superclass?.ref as Type)
        else
            false
    }


    /**
     * Returns a list of all superclasses of a type, including Anything
     * @return A list of the ids of all its superclasses
     */
    fun allSupertypes(): List<Type> {
        val all = mutableListOf<Type>()
        var where = this
        while(where.superclass?.ref !is Anything) {
            all.add(where.superclass?.ref as Type)
            where = where.superclass?.ref as Type
        }
        return all+where
    }

    /**
     * Returns a list of all subclasses of a type, not transitive.
     * @return list of subclasses of the Type.
     */
    fun subclasses(): Collection<Classifier> =
        model!!.getSubclasses(this)


    fun subtypes(): Collection<Type> =
        model!!.getSubtypes(this)


    /**
     * Checks if the superclass has a cycle.
     * @return true, if there is a cycle.
     */
    fun isCyclic(visited: MutableSet<Element> = mutableSetOf()) : Boolean {
        visited += this
        return when {
            superclass?.ref   == null     -> false
            superclass?.ref   is Anything -> false
            superclass?.ref!! in visited  -> true
            else        -> superclass?.ref!!.isCyclic(visited)
        }
    }
}
