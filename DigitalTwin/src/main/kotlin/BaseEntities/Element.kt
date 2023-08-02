package BaseEntities


import ImportedSysMDServices.AgilaSession
import Parser.QualifiedName
import Parser.SimpleName
import java.util.*

interface Element: Cloneable {
    /**
     * The model to which the element belongs.
//     */
    var model: AgilaSession?

    /**
     * A unique id of the Element that remains unchained over the whole lifecycle of the element.
     * We use UUIDv4 as suggested in SysMLv2 Std.
     */
    val elementId: UUID
    var aliasIds: Collection<String>

    /**
     * fields for identification by name, shortName.
     */
    var name: SimpleName?
    var shortName: SimpleName?
    val declaredName: SimpleName? get() = name
    val declaredShortName: SimpleName? get() = shortName
    val effectiveName: String?

    /**
     * The ownership is modeled by a set of owned elements.
     */
    var ownedElements: MutableList<Identity<Element>> // --> to be replaced by derived property

    /**
     * The owning element; can be an Identity with "null" entries in case of the root element.
     */
    var owner: Identity<Element>                      // --> to be replaced by derived property
    val owningNamespace: Namespace?
    val qualifiedName: QualifiedName

    /**
     * Is true for elements that are added, but are not subject to persistence and/or exchange of
     * entities.
     */
    var isTransient: Boolean

    /**
     * Is true if the element has been changed and needs a new commit
     */
    var hasBeenChanged: Boolean

    /**
     * is true if the element has been updated in evaluate/update cycles of constraint propagation
     * mechanisms etc.; prior to its use it shall be set to false.
     */
    var updated: Boolean

    /**
     * Method that updates fields of this element from another element.
     */
    fun updateFrom(template: Element)
    fun addOwnedElement(element: Element)

    /**
     * Resolves the names and UUID used in Identity to references and UUID.
     * It returns 'true' if a field was updated; the function is overridden
     * by other classes and used during initialization.
     * @return true if a field was updated
     */
    fun resolveNames(): Boolean

    /**
     * Avoid use of clone - it creates a NEW, independent element that has the same class and
     * fields as this, but a different elementId.
     */
    public override fun clone(): Element
}


/**
 * Returns the first element of type T from the owned elements.
 * @param T The subclass of Element for which we search.
 */
inline fun <reified T: Element> Element.getOwnedElementOfType(): T? {
    val found = ownedElements.find { (it.ref != null) && it.ref is T }
    return found?.ref as T?
}



/**
 * Returns a mutable list of all owned elements of type T.
 * @param T The subclass of Element for which we search.
 */
inline fun <reified T: Element> Element.getOwnedElementsOfType(): MutableCollection<T>  {
    val result = mutableListOf<T>()
    ownedElements.forEach { if (it.ref is T) result.add(it.ref as T) }
    return result
}


/**
 * Returns the owned element with a given name.
 * @param name A SimpleName that is searched for
 */
fun Element.getOwnedElement(name: SimpleName?, shortName: SimpleName? = null): Element? {
    ownedElements.forEach {
        if (name != null && it.ref?.name == name || (shortName == null) && it.ref?.shortName == name || (shortName!= null) && it.ref?.shortName == shortName)
            return it.ref
    }
    return null
}

/**
 * Returns the owned element with a given name.
 * @param name A SimpleName that is searched for
 */
inline fun <reified T> Element.getOwned(name: SimpleName): T? {
    ownedElements.forEach {
        if (it.ref?.name == name)
            return if (it.ref is T) it.ref as T else null
        if (it.ref?.shortName == name)
            return if (it.ref is T) it.ref as T else null
    }
    return null
}

/**
 * Returns the owned element with an index.
 * @param i index of the element list
 */
inline fun <reified T> Element.getOwnedByIndex(i: Int): T? {
    if (i in ownedElements.indices) {
        val element = ownedElements.get(i).ref
        return if (element is T) element else null
    }
    else
        return null
}
