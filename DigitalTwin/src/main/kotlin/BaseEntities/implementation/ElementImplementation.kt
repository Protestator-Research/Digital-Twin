package BaseEntities.implementation


import BaseEntities.Element
import BaseEntities.Identity
import BaseEntities.Namespace
import ImportedSysMDServices.AgilaSession
import Parser.QualifiedName
import Parser.SimpleName
import java.util.*


/**
 * Base class for the repository entities.
 * Elements can be of the special kind Package.
 * Elements are
 * - identified by a unique id field,
 * - have a user-given identification following the KerML v2 rules,
 * - a list of imports following SysML rules,
 * - a collection of owned elements that may not be identified by its names.
 *   (then we would need a namespace)
 */
open class ElementImplementation(
    override val elementId: UUID = UUID.randomUUID(),
    override var name: SimpleName? = null,
    override var shortName: SimpleName? = null,
    override var aliasIds: Collection<String> = emptyList(),
    override var ownedElements: MutableList<Identity<Element>> = mutableListOf(),
    override var owner: Identity<Element> = Identity(null, null, null)
) : Element {

    /** A dependency injected by call of model.create() */
    override var model: AgilaSession? = null


    /**
     * Field-less property that gets the owning namespace.
     */
    override val owningNamespace: Namespace?
        get() = if (owner.ref is Namespace) owner.ref as Namespace else owner.ref?.owningNamespace

    final override var isTransient: Boolean = false

    /**
     * Flag that is set if some non-transient information was changed.
     * Then, a new id is needed prior to persisting the element in the backend.
     */
    override var hasBeenChanged: Boolean = false

    /**
     * whether value has been changed by constraint propagation.
     * As this affects only non-transient information, no new id is needed.
     */
    override var updated: Boolean = true


    /** Field-less property; qualified name is determined by name + owner */
    override val qualifiedName: QualifiedName
        get() = if (name == null)
            shortName?:"(unnamed element)"
        else if (owner.ref != null && owner.ref?.shortName != "Global")
            "${owner.ref!!.qualifiedName}::${name}"
        else
            name!!

    /**
     * The method searches for (qualified) names in the element and
     * adds the id and reference to identifications, where the search
     * was successful or reports an error where not.
     */
    override fun resolveNames(): Boolean {
        // Update owner only by ID. Owner doesn't have a name.
        // Following not needed as long
        // as 1) in import the ownership is set. 2) new elements are added by create
        /*
        if (owner.id != null) {
            val old = owner.ref
            owner.ref = model?.get(owner.id!!)
            if (owner.ref != old)
                updated = true
        }

        ownedElements.forEach {
            updated = it.resolveIdentity(if (this is Namespace) this else owningNamespace!!) or updated
        }
        return updated */
        return false
    }

    /**
     * Establishes correct owner relationship between two elements, even if unowned.
     * A call of create of the model must add then all owned elements to Collection/Map of elements & give the ID.
     */
    override fun addOwnedElement(element: Element) {
        ownedElements.add(Identity(element))
        element.owner.ref = this
        element.owner.id = this.elementId
    }

    override fun toString(): String =
        "Element { name='$name', shortName = '$shortName', #owned = ${ownedElements.size}, id='$elementId'}"

    /**
     * Clone creates a copy of all fields, but NOT of the owned elements;
     * only some well-understood owned elements from the metamodel are copied (e.g. Multiplicity, ...).
     * Hence, the consistency of the model is NOT by itself preserved.
     * Function should NOT be used to copy parts of the model;
     * ONLY to create a backup or in really internal logic when there are no or well-understood owned elements.
     * !!! TAKE CARE !!!
     * Also, the clone is given a new id.
     */
    override fun clone(): Element {
        return ElementImplementation(
            name = name,
            shortName = shortName,
            ownedElements = Identity.copyOfIdentityList(ownedElements),
            owner = Identity(owner)
        ).also {
            it.updated = updated
            it.isTransient = isTransient
        }
    }

    /**
     * Simply copies all fields from a template into the current element.
     * Note, that only a shallow copy is made, no deep copy.
     * Also note, that the id of the original element is NOT copied.
     * @param template the template whose fields will be copied.
     */
    override fun updateFrom(template: Element) {
        // id = template.id?: AgilaSessionImpl.newUId(template.identification).id
        if (name != template.name) {
            name = template.name
            updated = true
        }
        if (shortName != template.shortName) {
            shortName= template.shortName
            updated = true
        }
        if (aliasIds != template.aliasIds) {
            aliasIds = template.aliasIds
            updated = true
        }
        // owner = template.owner?:owner
        // ownedElements = if (template.ownedElements.isEmpty()) ownedElements else template.ownedElements
        isTransient = template.isTransient
        hasBeenChanged = template.hasBeenChanged
    }

    override val effectiveName: String?
        get() = name?:shortName

}
