package ImportedSysMDServices


import BaseEntities.*
import BaseEntities.implementation.PackageImplementation
import ImportedSysMDExceptions.ElementNotFoundException
import ImportedSysMDExceptions.SemanticError
import Parser.QualifiedName
import Parser.loadSysMD
import SysMDRestImport.ElementDAO
import SysMDRestImport.toDAO
import SysMDRestImport.toElement
//import SysMDImportSolver.DiscreteSolver
import com.github.tukcps.jaadd.DDBuilder
import com.github.tukcps.jaadd.values.XBool
import com.github.tukcps.jaadd.values.XBoolImpl

import java.util.*
import kotlin.collections.ArrayList

/**
 * A mock repository that can be used like the AGILA Backend -- just, in memory.
 */
class Repository {

    /** The in-memory representation uses hash-maps for efficient search. */
    val elements:  HashMap <UUID, Element> = hashMapOf()

    /** Map that permits finding incoming relationships of an element */
    val targetOfRelationship: HashMap<Element, ArrayList<Relationship>> = hashMapOf()

    /** Map that permits finding outgoing relationships of an element */
    val sourceOfRelationship: HashMap<Element, ArrayList<Relationship>> = hashMapOf()

    /** List that stores the features that are part of constraint propagation */
    val schedule: ArrayList<BaseEntities.ValueFeature> = arrayListOf()

    /** Caches of important types */
    var numberType: Type? = null
    var realType: Type? = null
    var booleanType: Type? = null
    var integerType: Type? = null
    var stringType: Type? = null

    /** Map for saving not-yet-identified Relationships */
    val unownedElements: ArrayList<Pair<Identity<Element>, Element>> = arrayListOf()
}


/**
 * The AgilaSession is a container for a SysMD project.
 * It represents data by elements, properties, and relations thereof.
 * Its main organization consists of two interwoven trees:
 * - The isA - tree that represents the inheritance of all class and instance definitions.
 * - the hasA - tree that represents the decomposition of all classes, occurrences and instances.
 *
 * At the level of an AgilaSession, the entities are accessed by its UId.
 */
interface AgilaSession {
    val builder: DDBuilder
    val id: UUID

    // Error messages and settings
    val status: SessionStatus
    val settings: SessionSettings

    /** global is an imaginary package that holds all root elements */
    val global: PackageImplementation

    /** any is the superclass of all non-classified things */
    val any: Anything

    /** data contains data structures that represent the model and support efficient access, i.e. caches */
    val repo: Repository

    /** specific information from the discrete solver */
//    var dSolver: DiscreteSolver<Any?>

    val loadedProjects: ArrayList<UUID>

    /**
     * Adds new elements to the session. The elements are organized under the
     * root given as second parameter. The existing root is maintained,
     * the root passed as given parameter is just for easier analysis.
     * @param newElements collection of elements that will be added.
     */
    fun import(newElements: Collection<ElementDAO>)

    /**
     * Returns the element of the session, and the root of the ownership tree (global)
     * as a Pair, but not the temporary elements.
     */
    fun export(): List<ElementDAO>

    /**
     * Returns all elements in the session, including temporary elements, Any, Global.
     */
    fun get(): Collection<Element>

    operator fun get(uid: UUID): Element?

    /**
     * Creates a new element in a namespace that becomes owner of the element.
     * The new element can be of arbitrary subtype of ElementBase; i.e. ValueFeature, Namespace, Feature, etc.
     * If an element with the same id or name in namespace exists, its fields will be updated.
     * @param element The property to be created.
     * @param owner The element in which the property will be created.
     * @return the created element with the id field set. Note, that it is not necessarily the same as the
     * element passed as argument.
     */
    fun <T: Element> create(element: T, owner: Element): T

    /**
     * Like create, but the created element is marked as transient.
     * The new element can be of arbitrary subtype of ElementBase; i.e. ValueFeature, Namespace, Feature, etc.
     * If an element with the same id or name in namespace exists, its fields will be updated.
     * @param element The property to be created.
     * @param owner The element in which the property will be created.
     * @return the created element with the id field set. Note, that it is not necessarily the same as the
     * element passed as argument.
     */
    fun <T: Element> createTransient(element: T, owner: Namespace): T


    /**
     * Creates a new element in a namespace that becomes owner of the element.
     * The new element can be of arbitrary subtype of ElementBase; i.e. ValueFeature, Namespace, Feature, etc.
     * If an element with the same id or name in namespace exists, the existing one will
     * be deleted, including all its owned elements, and be replaced with the element given as argument.
     * @param element The property to be created.
     * @param owner The element in which the property will be created.
     * @return the created element with the id field set. Note, that it is not necessarily the same as the
     * element passed as argument.
     */
    fun <T: Element> createOrReplace(element: T, owner: Element): T


    /**
     * Gets an element by its database id.
     * Throws an ElementNotFound exception, if an element with that id does not exist.
     * @param id the id of the element
     * @return the Element.
     */
    fun getElement(id: UUID): Element

    /**
     * Gets a property by id, directly and without further search or initialisation.
     * In particular the quantity field is not initialized.
     * @param id id of the property
     */
    fun getProperty(id: UUID): ValueFeature

    /**
     * Deletes an element by reference. It also removes the hasA relationship in the according
     * parent element, and also all owned elements.
     * @param element the element to be deleted.
     */
    fun delete(element: Element): Element?

    /**
     * Gets the subclasses of an element. If the element is an instance, it will also consider
     * subclasses of the class of the instance.
     */
    fun getSubclasses(element: Element): Collection<Classifier>

    /**
     * Gets the subtypes of an element. If the element is an instance, it will also consider
     * subclasses of the class of the instance.
     */
    fun getSubtypes(element: Type): Collection<Type>

    /**
     * Ends a session without saving it.
     */
    fun endSession()

    /**
     * Removes all elements that are not related to textual models.
     */
    fun reset()

    /**
     * Adds an element where the ownership is not yet identified by an ID.
     * It is identified by a qualified name and/or feature chain.
     * These can in some cases only be identified correctly after all features and
     * inheritance information is initialized.
     * @param element the element to be added to an owner
     * @param ownerQualifiedName the qualified name that MUST be unique
     */
    fun addUnownedElement(element: Element, ownerQualifiedName: QualifiedName? = null, ownerId: UUID? = null)
    fun addUnownedElement(element: Element, ownerIdentity: Identity<Element>)
    fun getUnownedElements(): Set<Pair<Identity<Element>, Element>>
    fun dropUnownedElement(element: Element)

    /**
     * A shortcut that reads a SysMD-Model into the model.
     */
    operator fun String.unaryPlus()
}


/**
 * This class implements a model representation for use in the frontend SysMD compiler.
 * The model class holds all information that is read from a SysMD file.
 * Should be a more or less subset of the backend repository and lower-level service functionality.
 */
open class AgilaSessionImpl(
    override val id: UUID = UUID.randomUUID(),
    override val status: SessionStatus = SessionStatus(),
    override var settings: SessionSettings = SessionSettings(),
    override var builder: DDBuilder = DDBuilder()
): AgilaSession {

//    override var dSolver = DiscreteSolver<Any?>(this)
    override val loadedProjects = arrayListOf<UUID>()

    /**
     * We have one special package "Global" that is the highest level package.
     * It can be accessed by the reference global.
     */
    override val global = PackageImplementation(shortName = "Global", name="Global").also {
        it.owner.ref = null
        it.model = this
    }

    /**
     * We have a single element "Any" that is root of the inheritance tree.
     * It can be accessed by the reference anyElement.
     */
    override val any = Anything(
        owner = Identity(global),
        model = this
    )

    override val repo = Repository()

    /** We set up initial libraries, i.e. the math library with constants */
    init {
        // These are packages that are always available and there is a circular dependency among them.
        // Hence, one cannot create them via the regular API.
        repo.elements[any.elementId] = any
        repo.elements[global.elementId] = global

        // We put Any and Package now into the Package Global ...
        global.ownedElements.add(Identity(any))
        require(global === this.getElement(global.elementId))
    }

    override fun get(): Collection<Element> = repo.elements.values
    override fun get(uid: UUID): Element? = repo.elements[uid]

    /**
     * Adds new elements to the session. The elements are organized under the
     * root given as second parameter. The existing root is maintained,
     * the root passed as given parameter is just for easier analysis.
     * @param newElements collection of elements that will be cloned and added.
     */
    override fun import(newElements: Collection<ElementDAO>) {

        // we first separate root and other elements
        val import = arrayListOf<ElementDAO>()
        val roots = arrayListOf<ElementDAO>()
        newElements.forEach{newElement ->
            if (newElement.owner == null)
                roots += newElement
            else
                import += newElement
        }

        // Add root elements of import to global of session; if there, overwrite.
        roots.forEach { root ->
            create(root.toElement(), global)
        }

        // Add all imported elements to model, unless already there.
        import.forEach { element ->
            if (repo.elements[element.elementId] == null)
                repo.elements[element.elementId] = element.toElement()
        }

        // resolve uid -> ref, superclass null -> any, add model ref,
        resolveIDs()
    }


    private fun resolveIDs() {
        // resolve uid -> ref, superclass null -> any, add model ref,
        repo.elements.values.forEach {
            // set model to this
            it.model = this

            // resolve ID to reference of owned elements.
            it.ownedElements.forEach { owned ->
                getElement(owned.id!!).owner.ref = it
                owned.ref = getElement(owned.id!!)
            }

            // resolve ID to reference of sources/targets of relationships.
            when(it) {
                is Relationship -> {
                    it.source.forEach { source ->
                        source.ref = if (source.id != null) get(source.id!!) else null
                    }
                    it.target.forEach { target ->
                        target.ref = if (target.id != null) get(target.id!!) else null
                    }
                    if (it is Specialization && it.superclass.id == null) {
                        it.superclass.id = any.elementId
                        it.superclass.ref = any
                    }
                    if (it is Import) {
                        if (it.importedNamespace.id != null)
                            it.importedNamespace.ref = get(it.importedNamespace.id!!) as Namespace?
                    }
                }

                // TODO --> This is ugly!!!
                is ValueFeature -> {
                    it.ast = null
                    if (it.valueSpecs.firstOrNull() is XBoolImpl) {
                        val valueSpecs = arrayListOf<Any?>()
                        it.valueSpecs.forEach { valueSpec ->
                            valueSpecs.add(
                                when (valueSpec.toString()) {
                                    "True" -> XBool.True
                                    "False" -> XBool.False
                                    "X" -> XBool.X
                                    "Contradiction" -> XBool.NaB
                                    else -> {}
                                }
                            )
                        }
                        it.valueSpecs = valueSpecs
                    }
                }
            }
        }
    }

    /**
     * The function export creates a list of elements where
     * 1) Any and Global are not included.
     * 2) owners or superclass are set to null for global resp. any.
     * 3) To be sure, all elements are cloned. No reference shall leave the session;
     *    just to prevent side effects.
     */
    override fun export(): List<ElementDAO>  {
        checkConsistency(repo.elements.values, checkForNoTransients = false)

        val exportCollection = arrayListOf<ElementDAO>()

        // create collection for export, leaving out global and any
        repo.elements.values.forEach {
            if (it != global && it != any)
                exportCollection.add(it.toDAO())
        }

        // remove transient elements, from both list of elements *and* list of owned elements.
        val transient = arrayListOf<UUID>()
        repo.elements.values.forEach { if (it.isTransient) { transient.add(it.elementId) } }
        exportCollection.removeIf { it.elementId in transient }
        for (element in exportCollection) {
            element.ownedElements?.removeIf { it in transient }
            element.source?.removeIf { it in transient}
            element.target?.removeIf { it in transient}
            if (element.owner == global.elementId) element.owner = null
        }

        return exportCollection
    }

    /**
     * Creates an element owned by an element that is a namespace;
     * if an element with the same elementId is already in the model, the existing model is returned.
     * @param element The element to be created. It must have a valid identification.
     * @param owner The element in which the property will be created.
     * @return the created property with UId field set.
     */
    @Suppress("UNCHECKED_CAST")
    override fun <T : Element> create(element: T, owner: Element): T {

        // If Element with same elementId already exists in model --> update.
        val existingElement = get(element.elementId)
        if (existingElement != null) {
            existingElement.updateFrom(element)
            if (existingElement.updated)
                status.updates[existingElement.elementId] = "updated: '${existingElement.qualifiedName}'"
            return existingElement as T
        }


        // If identification by name exists, it must be unique in namespace
        if (element.name != null || element.shortName != null) {
            var existingWithSameName: Element? = null
            if (element.name != null)
                existingWithSameName = owner.getOwnedElement(element.name!!)
            if (existingWithSameName == null && element.shortName != null)
                existingWithSameName = owner.getOwnedElement(element.shortName!!)

            if (existingWithSameName != null) {
                // Quite annoying ... -- should be optional for debugging. Not by default.
                // reportInfo(found, "Overwritten '${element.identification.toName()}' with new information.")
                if (existingWithSameName.javaClass != element.javaClass) {
                    report(element, "'${element.qualifiedName}' changes the type of the element '${existingWithSameName.qualifiedName}; can lead to problems.'; suggestion: reset for complete update.")
                    delete(existingWithSameName)
                    return create(element, owner)
                }
                existingWithSameName.updateFrom(element)
                if (existingWithSameName.updated)
                    status.updates[existingWithSameName.elementId] = "updated: '${existingWithSameName.qualifiedName}'"
                return existingWithSameName as T
            }
        }

        // Import must not be duplicate
        if (element is Import) {
            element.model = this
            val imports = owner.getOwnedElementsOfType<Import>()
            val duplicate = imports.find {
                it.importedNamespace.str == element.importedNamespace.str // && it.importedMemberName == it.importedMemberName
            }
            if (duplicate != null ) return element
        }

        // Multiplicity must not be duplicate; just update
        if (element is Multiplicity) {
            element.model = this
            val found = owner.getOwnedElementOfType<Multiplicity>()
            if (found != null) {
                found.updateFrom(element)
                return found as T
            }
        }

        if (element is Specialization) {
            element.model = this
            val found = owner.getOwnedElementOfType<Specialization>()
            if (found != null) {
                found.updateFrom(found)
                return found as T
            }
        }

        // Create relationship with owning namespace
        element.model = this
        element.owner.id = owner.elementId
        element.owner.ref = owner
        owner.ownedElements.add(Identity(element))
        repo.elements[element.elementId] = element

        // In case there are owned elements - add them to the model as well.
        // element.ownedElements.forEach {
        //     if (it.ref != null) repo.elements[it.id!!] = it.ref!!
        // }

        return element
    }

    /**
     * Creates or, if an element with the same *name* exists, replaces an element.
     * If an element with the same *elementId* exists, it will be updated.
     * This changes the elementId of the element.
     * @param element the element to be created.
     * @param owner the element that shall become owner.
     * @return the element created.
     */
    override fun <T : Element> createOrReplace(element: T, owner: Element): T {
        val existingElement = owner.getOwnedElement(element.name, element.shortName)
        if (existingElement == null)
            return create(element, owner)
        else {
            delete(existingElement)
            val created = create(element, owner)
            resolveIDs()
            return created
        }
    }

    override fun getUnownedElements(): Set<Pair<Identity<Element>, Element>> {
        return repo.unownedElements.toSet()
    }

    override fun dropUnownedElement(element: Element) {
        repo.unownedElements.removeIf { it.second === element }
    }

    /** Just removes the session from the existing sessions */
    override fun endSession() {
        AgilaSessionMgr.kill(this.id)
    }


    override fun getElement(id: UUID): Element = get(id)
            ?: throw ElementNotFoundException(id.toString())


    /** Gets a property with given id */
    override fun getProperty(id: UUID): ValueFeature =
        get(id) as ValueFeature?
            ?: throw  SemanticError("Not found: $id")


    /**
     * Deletes an element with a given reference and all owned elements.
     * Also updates ownedElements of owner.
     * @param element reference to element to be deleted
     */
    override fun delete(element: Element): Element? {
        if (element in setOf(global, any)) return null
        val owner = element.owner.ref
        val toDelete = arrayListOf<Identity<Element>>()
        element.ownedElements.forEach { toDelete.add(it) }
        toDelete.forEach {
            delete(it.ref!!)
        }
        owner?.ownedElements?.removeIf {it.id == element.elementId }
        val result = repo.elements.remove(element.elementId)
        checkConsistencyOfBuilders()
        checkConsistency(repo.elements, global.elementId, "remove")
        return  result
    }


    /** Gets all subclasses of a class; requires initialized model. */
    override fun getSubclasses(element: Element): Collection<Classifier> {
        val result = arrayListOf<Classifier>()

        val relations = getRelationshipsTo(element, "*")
        relations.forEach {
            if (it is Specialization) {
                if (it.target[0].ref === element && it.source[0].ref is Classifier)
                    result.add(it.source[0].ref as Classifier)
            }
        }
        return result
    }

    /**
     * Gets all subtypes of a type; requires an initialized model.
     * @param element type from which subtypes will be searched
     * @return collection of subtypes
     */
    override fun getSubtypes(element: Type): Collection<Type> {
        val result = arrayListOf<Type>()

        val relations = getRelationshipsTo(element, "*")
        relations.forEach {
            if (it is Specialization) {
                if (it.target[0].ref === element && it.source[0].ref is Type)
                    result.add(it.source[0].ref as Type)
            }
        }
        return result
    }


    /**
     * Creates a transient element that will NOT be persisted.
     * It nevertheless must have an id.
     * @param element A property with a user-defined and unique id that will be added to the session cache.
     * @param owner The element to which the property belongs.
     */
    override fun <T: Element> createTransient(element: T, owner: Namespace): T {
        element.isTransient = true

        // If identification by name exists, it must be unique in namespace
        if (element.name != null || element.shortName != null) {
            var found: Element? = null
            if (element.name != null)
                found = owner.getOwnedElement(element.name!!)
            if (found == null && element.shortName != null)
                found = owner.getOwnedElement(element.shortName!!)

            if (found != null) {
                // Quite annoying ... -- should be optional for debugging. Not by default.
                // reportInfo(found, "Overwritten '${element.identification.toName()}' with new information.")
                if (found.javaClass != element.javaClass) {
                    report(element, "'${element.qualifiedName}' cannot change the type of the element '${found.qualifiedName}'; suggestion: reset for complete update.")
                }
                found.updateFrom(element)
                if (found.updated)
                    status.updates[found.elementId] = "updated: '${found.qualifiedName}'"
                @Suppress("UNCHECKED_CAST")
                return found as T
            }
        }

        // Import must not be duplicate
        if (element is Import) {
            element.model = this
            val imports = owner.getOwnedElementsOfType<Import>()
            val duplicate = imports.find {
                it.importedNamespace.str == element.importedNamespace.str // && it.importedMemberName == it.importedMemberName
            }
            @Suppress("UNCHECKED_CAST")
            if (duplicate != null ) return duplicate as T
        }

        // Multiplicity must not be duplicate; just update
        if (element is Multiplicity) {
            element.model = this
            val found = owner.getOwnedElementOfType<Multiplicity>()
            if (found != null) {
                found.updateFrom(found)
                @Suppress("UNCHECKED_CAST")
                return found as T
            }
        }

        if (element is Specialization) {
            element.model = this
            val found = owner.getOwnedElementOfType<Specialization>()
            if (found != null) {
                found.updateFrom(found)
                @Suppress("UNCHECKED_CAST")
                return found as T
            }
        }

        element.model = this

        repo.elements[element.elementId] = element
        element.owner.ref = owner
        element.owner.id = owner.elementId
        owner.ownedElements.add(Identity(element))
        element.isTransient = true

        return element
    }

    /**
     * Adds an element to the unowned elements, without checks.
     * @param element element for which the owner-relationship is only defined by a qualified name
     * @param ownerQualifiedName the qualified name of the owner
     */
    override fun addUnownedElement(element: Element, ownerQualifiedName: QualifiedName?, ownerId: UUID?) {
        val ownerIdentity: Identity<Element> = if (ownerId != null)
            Identity(ownerId)
        else if (ownerQualifiedName != null)
            Identity(ownerQualifiedName)
        else
            throw Exception("Wrong call of addUnownedElement: at least id or name of owner required")
        repo.unownedElements.add(Pair(ownerIdentity, element))
    }


    /**
     * Adds an element to the unowned elements, without checks.
     * @param element element for which the owner-relationship is only defined by a qualified name
     * @param ownerIdentity the identity of the owner
     */
    override fun addUnownedElement(element: Element, ownerIdentity: Identity<Element>) {
        repo.unownedElements.add(Pair(ownerIdentity, element))
    }

    
    /**
     * Remove all elements except any or global, and reset all internal data structures and states.
     */
    override fun reset() {
        val toDelete = arrayListOf<UUID>()
        repo.elements.values.forEach { element ->
            if (element != any && element != global) {
                toDelete.add(element.elementId)
            }
        }
        toDelete.forEach { repo.elements.remove(it) }
        global.ownedElements = mutableListOf(Identity(any))
        repo.unownedElements.clear()
        repo.sourceOfRelationship.clear()
        repo.targetOfRelationship.clear()
        builder = DDBuilder()
//        dSolver = DiscreteSolver<Any?>(this)
        status.reset()

        /** Caches of important types */
        repo.numberType = null
        repo.realType = null
        repo.booleanType = null
        repo.integerType = null
        repo.stringType = null
        repo.schedule.clear()
    }

    /** parses a line on which the unary operator + is applied; DSL for testing purpose */
    override operator fun String.unaryPlus() =
        loadSysMD(this)

    override fun toString(): String {
        return status.toString()
    }
}

/**
 * Returns a list of all elements of Type T
 * @param T: type for which elements will be filtered.
 */
inline fun <reified T: Element> AgilaSession.getAllOfClass(): List<T> =
     get().filterIsInstance<T>()
