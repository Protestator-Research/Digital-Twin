package ImportedSysMDServices

import BaseEntities.*
import Parser.*


/**
 * Resolves a qualified name in a namespace.
 * Furthermore, the method has a template parameter to which it, if possible, casts the result.
 * @param T Type for which will be searched.
 * @param qualifiedName QualifiedName that will be searched.
 */
inline fun <reified T: Element> Namespace.resolveName(qualifiedName: QualifiedName, searchInSuperClass: Boolean = true): T?  {
    val found = findRecursive(qualifiedName, emptySet(), emptySet(),true, searchInSuperClass)
    if (found is T?)
        return found
    model?.report(found!!,"'$qualifiedName' could be resolved, but is of wrong type")
    return null
}


/**
 * Searches recursively for a qualified name, starting from the namespace.
 * @param qualifiedName the qualified name that is searched
 * @param searchedImports optionally, a set of Namespaces that have already been searched; needed to detect cyclic includes.
 * @param searchedSuperClasses optionally, the set of searched superclasses; needed to tetect cyclic definitions
 * @param searchInOwner optionally, whether the given qualified name is initially given as simple name, or just
 * became a simple name via recursion that cuts qualified name down.
 * @param searchInSuperClass optionally, whether to recurse into superclasses.
 * @return An element of or null, if the name cannot be resolved.
 */
fun Namespace.findRecursive(
    qualifiedName: QualifiedName,
    searchedImports: Set<Namespace>,
    searchedSuperClasses: Set<Type>,
    searchInOwner: Boolean = true,
    searchInSuperClass: Boolean = true
): Element? {

    // Stop search if we search in Any, or if the name has been shortened to an empty string.
    if (this is Anything) return null
    if (qualifiedName.hasNoName()) return null

    // If we have a simple name, we can search in owned elements that are identified by simple names.
    if (qualifiedName.isSimpleName()) {
        if (this == model?.global && qualifiedName == "Global") return model?.global
        val found = getOwnedElement(qualifiedName)
        if (found != null) return found
    } else {
        // Check for Self and Global name that are just pre-defined alias names.
        val newName: QualifiedName = qualifiedName.dropFirstName()

        // Search downwards in matching owned namespaces, shortening the qualified name by 1st name.
        // SysMD: We allow "Global" as explicit entry to start of global search.
        val matchingNamespace =
            if (qualifiedName.firstName() == "Global") model?.global
            else getOwnedElement(qualifiedName.firstName())
        if (matchingNamespace != null && matchingNamespace is Namespace) {
            val found = matchingNamespace.findRecursive(newName, emptySet(), emptySet(),false, searchInSuperClass)
            if (found != null) return found
        }
    }

    // Search in imports, and track searched paths to avoid cyclic search.
    imports.forEach {
        if (this !in searchedImports) {
            if (it.ref != null && it.ref is Namespace) {
                val found = it.ref!!.findRecursive(qualifiedName, searchedImports+this, searchedSuperClasses, searchInOwner, searchInSuperClass)
                if (found != null) return found
            }
        }
    }

    // Special case: Relationship?
    if (this is Relationship) {
        if (qualifiedName == "source") return source.firstOrNull()?.ref
        else if (qualifiedName == "target") return  target.firstOrNull()?.ref
    }

    // Search in supertypes
    if (this is Type && searchInSuperClass && this !in searchedSuperClasses) {
        if (this.superclass != null && this.superclass!!.ref is Type && this.superclass!!.ref is Namespace) {
            val found = this.superclass!!.ref!!.findRecursive(qualifiedName, searchedImports+this, searchedSuperClasses+this, false, true)
            if (found != null) return found
        }
    }

    // Search in owning namespace
    return if (searchInOwner) owningNamespace?.findRecursive(qualifiedName, searchedImports, searchedSuperClasses, true) else null
}



/**
 * Recursive search upwards to superclasses until global; first occurrence is returned.
 * @param name simple name that is searched in owned elements only
 */
tailrec fun Namespace.findOwnedElement(name: String): Element? {
    return getOwnedElement(name)
        ?: if (this is Type && superclass?.ref != null)
            superclass?.ref?.findOwnedElement(name)
        else
            null
}


/**
 * Recursive collection of all owned features, upwards to superclass until Anything.
 */
fun Namespace.findAllOwnedElements(z: Int=0): Collection<Element> {
    val owned = mutableListOf<Element>()
    ownedElements.forEach { owned.add(model!!.getElement(it.id!!)) }
    if (this is Type && superclass?.ref != null) {
        val superclass = superclass?.ref as Namespace
        if(this == superclass)
            model?.report(this, "Cyclic superclass: ${this.qualifiedName}")
        else{
            val inherited = if (superclass !is Anything) superclass.findAllOwnedElements(z+1)
            else emptySet()
            return mergeOwnedElements(owned, inherited)
        }
    }
    return owned
}



/**
 * Merges the owned elements specified by the hasA relationships from a class with
 * inherited relations that are potentially overridden.
 * The merge method merges features of a class with those of its superclass
 * such that the Liskov principle holds.
 * @param own Features of subclass
 * @param inherited Features of superclass
 */
private fun mergeOwnedElements(own: Collection<Element>, inherited: Collection<Element>): Collection<Element> {
    val merged = mutableSetOf<Element>()
    merged.addAll(own)

    for (i in inherited) {
        var overridden = false
        for (o in own) { overridden = true }
        if (!overridden)
            merged.add(i)
    }
    return merged
}
