package ImportedSysMDServices

import BaseEntities.*
import BaseEntities.implementation.SpecializationImplementation
import ImportedSysMDExceptions.InternalError
import ImportedSysMDExceptions.SemanticError
import ImportedSysMDQuantities.Quantity
import ImportedSysMDQuantities.VectorQuantity
import ImportedSysMDQuantities.ite
import SysMDCopyAst.AstRoot
import SysMDCopyAst.functions.AstByImplements
import SysMDCopyAst.functions.AstBySubclasses
import kotlin.math.abs
import ImportedSysMDServices.resolveName
import Parser.Identification
import Parser.QualifiedName
import Parser.parseDependency

/**
 * Schedules and initializes the properties and the elements.
 * This shall be done before calling propagate().
 */
fun AgilaSession.initialize(level: Int = 100) {
    if (settings.initialize) {
        try {
            if (level > 0) resolveNames()        // Calls initialize of all elements --> at least ownership should be resolved.
            repo.numberType = global.resolveName("ScalarValues::Number")
            repo.realType = global.resolveName("ScalarValues::Real")
            repo.integerType = global.resolveName("ScalarValues::Integer")
            repo.booleanType = global.resolveName("ScalarValues::Boolean")
            repo.stringType = global.resolveName("ScalarValues::String")
            if (level > 1) addInherited()     // Calls initialize of types that will add inherited properties.
            if (level > 2) resolveNames()        // Again, update name resolution considering types and inheritance
            if (level > 5) initRelationships()
            if (level > 4) initProperties()

            // Now, we only do checking and reporting of issues to the Agenda.
            if (level > 6) checkIdentifications()
            if (level > 7) get().filterIsInstance<Type>().forEach { checkConsistencyOfInheritance(it) }
        } catch (error: Exception) {
            report(SemanticError(message = "Initialization failed (${error.message}) ", cause = error))
            if (!settings.catchExceptions) throw error
        }
    }
}


/**
 * For inherited properties that are computed, computation can -- depending on the scope --
 * lead to different results. Hence, we create a volatile, inherited property that is
 * a clone of the superclass' property.
 */
private fun Type.addInheritedElements() {
    // "Clone" features of superclass iff not there!
    assert(model != null)
    val superclass = superclass?.ref ?: model!!.any
    val superclassFeatures = model!!.getAllInheritedFeatures(superclass).associateBy { Identification(it) }
    val localFeatures = getOwnedElementsOfType<Feature>().associateBy { Identification(it) }

    // For each inherited property, we create a clone; needed as changes in the owning class
    // to the property shall not change the original property of the superclass.
    superclassFeatures.values.forEach {
        if (localFeatures[Identification(it)] == null) {
            val klon = it.deepCloneWithInheritedFeature(this)
            model!!.createTransient(klon, this)
        }
    }
}


/**
 * finds all features in a namespace, while considering inheritance.
 * @param namespace the namespace in which features will be searched
 * @return list of all features in a namespace including inherited.
 */
fun AgilaSession.getAllInheritedFeatures(namespace: Namespace): Collection<Feature> =
    when (namespace) {
        is Anything -> emptyList()
        is Type -> mergeFeatures(namespace.getOwnedElementsOfType(), getAllInheritedFeatures(namespace.superclass?.ref as Namespace))
        else -> namespace.getOwnedElementsOfType()
    }


private fun AgilaSession.initRelationships() {
    get().forEach{
        if (it is Relationship)
            it.resolveNames()
    }
}


/**
 * Calls the initialize function all elements except ValueFeatures in random order.
 * No guarantee that name references can be resolved.
 * Ensures that at least an initial amount of elements is initialized.
 * No tests are made; they can only be done later.
 */
internal fun AgilaSession.resolveNames() {
    var stable: Boolean
    var iterations = 1000

    // First create ownership hierarchy
    do {
        stable = true
        iterations--

        // Check if owner of element can be identified and
        // create them in model.
        val elementsIdentified = mutableListOf<Element>()
        getUnownedElements().forEach {
            when {
                it.first.id != null -> {
                    val found = get(it.first.id!!)
                    if (found != null) {
                        create(it.second, found)
                        elementsIdentified.add(it.second)
                        stable = false
                    }
                }
                it.first.str != null -> {
                    val found = global.resolveName<Element>(it.first.str!!, searchInSuperClass = false)
                    if (found != null) {
                        create(it.second, found)
                        elementsIdentified.add(it.second)
                        stable = false
                    }
                }
            }
        }
        elementsIdentified.forEach { dropUnownedElement(it) }

    } while (!stable && iterations > 0)

    get().forEach {
        if(it != global && it.owningNamespace == null) {
            report("Unowned element: $it")
        }
    }

    // identify all names, but not yet Types w/ inheritance
    val elements = get().filter { it !is Type }
    for (element in elements) {
        element.resolveNames()
        if (element.updated)
            stable = false
        element.updated = false
    }

    // finally, inherited elements as well.
    val elements2 = get()
    for (element in elements2) {
        element.resolveNames()
        if (element.updated)
            stable = false
        element.updated = false
    }
}


/**
 * Initializes the classifiers by
 * 1) Checking that superclasses are ok and not cyclic.
 * 2) Creating a copy of the features of superclasses in subclasses.
 * We handle the ValueFeatures and Associations separately; hence, here we only handle
 * ClassifierImplementations.
 */
private fun AgilaSession.addInherited() {
    // TODO: Why do we have to (?) Classifiers here only, not all types?
    getAllOfClass<Classifier>().forEach {
        // First, we check some semantic issues, then we add inherited elements
        when {
            it is Anything -> {}
            it.superclass?.ref == null -> {
                if (it.specialization == null) { // Just in case there is a really faulty internal data structure ...
                    create(SpecializationImplementation(it, any), it)
                    reportInfo(it, "'${it.effectiveName}': specialization object missing; added it.")
                }
                it.superclass?.ref = any
                reportInfo(it, "'${it.effectiveName}': Could not identify superclass '${it.superclass?.str}'")
            }

            it.superclass?.str != "Any" && it.superclass?.str != null && it.superclass?.ref === any -> {
                reportInfo(it, "Superclass '${it.superclass?.str}' of '${it.qualifiedName}' could not be resolved")
            }

            it.superclass?.ref !is Classifier -> {
                it.superclass?.ref = any
                reportInfo(it, "Superclass '${it.superclass?.str}' of '${it.qualifiedName}' is not a Class, Association, or Datatype")
            }

            it.superclass?.ref === it -> {
                // it.superclass?.str = "Any"
                it.superclass?.ref = any
                report("Superclass ${it.superclass?.str} of '${it.qualifiedName} cannot be itself; stopping analysis")
            }

            (it.superclass?.ref as Type).isCyclic() -> {
                val cyclic = it.superclass?.str
                it.superclass?.str = "Any"
                it.superclass?.ref = any
                report( "Cyclic definition: '$cyclic' cannot be subclass of '${it.qualifiedName}; stopping analysis'")
            }

            // Only iff everything is OK we add it to our classifiers without issues
            else -> {
                try {
                    if (it.name == "c")
                        println()
                    it.addInheritedElements()
                } catch (error: Exception) {
                    report("Error processing the superclass '${it.superclass?.str} of ${it.qualifiedName}")
                }
            }
        }
    }
}

/**
 * Initializes the properties:
 * - Schedules the properties such that they are ordered following their data dependencies,
 * i.e. a property that depends on the computation of another property is schedule behind it.
 * - Does a single evaluation upwards to propagate types and units; this ensures that the quantity property of each
 * Property is set correctly.
 */
private fun AgilaSession.initProperties() {
    repo.schedule.clear()
    // First, we initialize all properties in the symbol table, such that the values are equal to the specified ranges.
    // This guarantees that if we later infer types, the declared properties are already initialized.
    //FIXME: Temporary workaround, to preserve order before sorting
    val properties: List<ValueFeature> =
        get().filterIsInstance(ValueFeature::class.java).sortedBy { it.effectiveName.hashCode() }

    // Then, we set up a list of computed and not-yet-computed properties.
    val computed = mutableSetOf<ValueFeature>()
    val notComputed = mutableListOf<ValueFeature>()
    properties.forEach {
        notComputed += it
        it.initVectorQuantity()
        // reportError(it, "Feature '${it.qualifiedName}' has a type '${it.superclass.str}' that could not be resolved")
    }

    // Check if there is a cyclic dependency in a single expression ... should be better at
    // overall level -> todo.
    properties.forEach { property ->
        val leaveNames = mutableSetOf<String>()
        if ((property.ast != null) && ((property.ast as AstRoot).dependency !is AstBySubclasses)
            && (property.ast as AstRoot).dependency !is AstByImplements
        ) {
            property.ast!!.getLeaves().forEach {
                if (it.qualifiedName != null)
                    leaveNames.add(it.qualifiedName!!)
            }
            if (property.name in leaveNames || property.shortName in leaveNames)
                throw SemanticError("Cyclic Dependency in ${property.effectiveName}")
        }
    }

    // order the properties by their dependencies into the repo.schedule. This schedule is used for initialization of the properties.
    var iterations = 0
    while (notComputed.isNotEmpty() && (iterations < 10000)) {
        iterations++
        val it = notComputed.first()
        try {
            if (it.ast != null) {   // For not constants, literals, ...
                //test if there is a dependency in notComputed
                val dependentProperties = it.ast?.getDependencyStrings()!!
                val notComputedElements = notComputed.map { it.effectiveName }
                if (!dependentProperties.any { it in notComputedElements }) {
                    computed += it
                    notComputed -= it
                    repo.schedule += it
                } else {
                    notComputed.removeFirst()
                    notComputed.add(it)
                }
            } else {  // For constants, literals, ...
                notComputed -= it
                computed += it
                repo.schedule += it
            }
        } catch (exception: Exception) {
            report(InternalError(message = exception.message?:"Error during scheduling of constraints", cause = exception))
            computed += it
            notComputed -= it
            // schedule+=it --- we do not schedule an erroneous dependency.
        }
    }
    // add all remaining elements to schedule
    if (iterations >= 10000)
        while (notComputed.isNotEmpty()) {
            val it = notComputed.first()
            computed += it
            notComputed -= it
            repo.schedule += it
        }

    // Initialize internal AST nodes, starting from leaves upwards.
    repo.schedule.forEach {
        if (it.dependency.isNotBlank())
            it.parseDependency()
        it.ast?.runDepthFirst { initialize() }
    }

    repo.schedule.forEach {
        try {
            it.ast?.evalUpRec()
        } catch (exception: Exception) {
            report(exception)
        }
    }

//    if (!dSolver.isInitialized())
//        dSolver.initialize(this)
//    repo.schedule.forEach { dSolver.update(it) }
}


/**
 * Searches for a property in an element and, if not found in its subclasses.
 * The value of the property is computed using the ITE function.
 */
fun AgilaSession.estimateProperty(namespace: Namespace, qualifiedName: QualifiedName): VectorQuantity {
    // It is already known from this or a superclass.
    val p = namespace.resolveName<ValueFeature>(qualifiedName)

    if (p != null) return p.vectorQuantity

    val subclasses = getSubclasses(namespace)
    var quantity: VectorQuantity? = null
    for (subclass in subclasses) {
        quantity = if (subclass == subclasses.first())
            estimateProperty(subclass, qualifiedName)
        else {
            val alternative = builder.variable("select_" + subclass.elementId)
            alternative.ite(estimateProperty(subclass, qualifiedName), quantity!!)
        }
    }
    return quantity!!
}


/**
 * @return the joined set of properties in case of inheritance, where
 * - properties are added if present in a single properties sets
 * - properties are intersected if present in both properties sets
 * - an exception is thrown if a property is present in both, but the inherited is not a
 *   specified value is not a subset of the superclass.
 *   This is considered as an inconsistency in the model.
 */
private fun AgilaSession.mergeFeatures(own: Collection<Feature>, inherited: Collection<Feature>): Collection<Feature> {
    val merged = mutableSetOf<Feature>()
    merged.addAll(own)
    for (i in inherited) {
        var overridden = false
        for (o in own) {
            // If there is already a property with the same name, compute subset of it.

            if ((o.name == i.name && i.name != null) || (o.shortName == i.shortName) && i.shortName != null) {
                // Basic requirement for inheritance, must hold in all cases otherwise something went wrong before ...
                // TODO: Limit this to subclass which is sufficient.
                // if (o.ofClass.str != i.ofClass.str)
                if (o.ofClass?.ref == null) return merged
                //    reportError(o, "INTERNAL ERROR: ${o.ofClass.str} was not resolved property." )

                if ( o.ofClass?.ref?.isSubclassOf(i)!! )
                    reportInconsistency(o, "INCONSISTENCY: subclass type  of '${o.qualifiedName}' must be subclass of '${i.qualifiedName}'")

                if(o::class != i::class )
                    reportInconsistency(i,"Value Feature may not override Part")

                if (o.multiplicity !in i.multiplicity)
                    reportInconsistency(o, "INCONSISTENCY: multiplicity of subclass '${i.qualifiedName}' must be subset of superclass '${o.qualifiedName}'.")

                /*
                when (o.quantity.value) {
                    is AADD -> o.quantity = o.quantity.intersect(i.quantity)
                    is IDD  -> o.quantity = o.quantity.intersect(i.quantity)
                    is BDD  -> o.quantity = o.quantity.intersect(i.quantity)
                    is StrDD -> { } // TODO()
                } */
                overridden = true
            }
        }
        if (!overridden) {
            merged.add(i)
        }
    }
    return merged
}

/**
 * We look at an element and its superclass(es).
 * - The subclass properties must be a subset of the superclass properties with same name.
 * - Maybe additional needs for other types; t.b.d.
 */
fun AgilaSession.checkConsistencyOfInheritance(element: Type) {
    val superclass = element.superclass?.ref
    if (superclass != null) {
        element.ownedElements.forEach { ownedElement ->
            val owned = ownedElement.ref!!
            if (owned is Feature) {
                val superclassFeature = superclass.getOwnedElement(owned.name, owned.shortName)
                if  (owned is ValueFeature && superclassFeature is ValueFeature) {
                    // Checks for superclass and subclass property
                    // Basic requirement for inheritance, must hold in all cases otherwise something went wrong before ...
                    if (owned.ofClass?.ref != superclassFeature.ofClass?.ref)
                        report(owned, "INCONSISTENCY: subclass type of ${owned.effectiveName} must be subclass of ${superclass.effectiveName}")
                    when  {
                        owned.ofClass?.ref?.isSubclassOf(repo.realType) == true -> {
                            //Convert Ranges od owned and superclass to SI
                            owned.vectorQuantity.values.indices.forEach {
                                val ownedRange = Quantity(builder.range(owned.rangeSpecs[it]),owned.unitSpec).getRange().intersect(owned.vectorQuantity.values[it].asAadd().getRange())//constrain to quantity of owned
                                //extend range to solve problems with accuracy
                                val extendedRangeSuperclass = builder.range(superclassFeature.rangeSpecs[it].min-abs(superclassFeature.rangeSpecs[it].min*0.000001),superclassFeature.rangeSpecs[it].max+abs(superclassFeature.rangeSpecs[it].max*0.000001))
                                val superClassRange = Quantity(extendedRangeSuperclass,superclassFeature.unitSpec).getRange()
                                if (ownedRange !in superClassRange)
                                    reportInconsistency(owned, "INCONSISTENCY: subclass value ${owned.rangeSpecs} of ${superclassFeature.effectiveName} must be refinement of superclass value ${superclassFeature.rangeSpecs}")
                                if (owned.vectorQuantity.unit != superclassFeature.vectorQuantity.unit)
                                    reportInconsistency(owned, "INCONSISTENCY: subclass unit ${owned.unitSpec} of ${superclassFeature.effectiveName} must be the same as ${superclassFeature.unitSpec}")
                            }

                        }
                        owned.ofClass?.ref?.isSubclassOf(repo.integerType) == true ->
                            owned.vectorQuantity.values.indices.forEach {
                                if (owned.intSpecs[it] !in superclassFeature.intSpecs[it])
                                    reportInconsistency(owned, "INCONSISTENCY: subclass value ${owned.intSpecs} of ${owned.effectiveName} must be refinement of superclass value ${superclassFeature.intSpecs}")
                            }

                        owned.ofClass?.ref?.isSubclassOf(repo.booleanType) == true ->
                            { // if (owned.boolSpec !in superclassFeature.boolSpec) {
                                // TODO: Agree with Axel & Sebastian how to handle digital inconsistencies.
                                // reportError(get(it),
                                //    "INCONSISTENCY: subclass value ${owned.boolSpec} of ${owned.effectiveName} must be refinement of superclass value ${superclassProperty.boolSpec}"
                                //)
                            }
                    }
                }
                if (superclassFeature is Feature && owned.multiplicity !in superclassFeature.multiplicity)
                    report("INCONSISTENCY: multiplicity of subclass must be subset of superclass multiplicity.")
            }
        }
    }
}
