package ImportedSysMDServices

import BaseEntities.*
import java.util.*


/**
 * Checks invariants for debugging and robustness.
 * @param elements Collection of all elements
 * @param info Info string for error message
 */
fun AgilaSession.checkConsistency(
    elements: Collection<Element>,
    info: String?="",
    checkForNoTransients: Boolean = true
) {
    // val roots = elements.filter { it.owner.id == null }
    val map = elements.associateBy( {it.elementId}, {it} )

    map.values.forEach {

        // No transient element shall be persisted.
        if (it.isTransient && checkForNoTransients)
            throw Exception("($info) Attempt to persist element that is transient: $it.id")

        // Every element with an id is in the map.
        if ( it.owner.id != null && (it.owner.id !in map.keys) && it.name != "Global") {
            throw Exception("($info) Inconsistency in model: element '${it.qualifiedName}' has unknown owner id.")
        }
        if ( it is Specialization ) {
            if (it.superclass.id != null && map[it.superclass.id] == null)
                throw  Exception("($info) Type '${it.qualifiedName}' has unknown superclass id.")
            if (map[it.subclass.id] == null)
                throw  Exception("($info) Type '${it.qualifiedName}' has unknown subclass id.")
        }
    }
}


/**
 * Checks for debugging and robustness.
 */
fun AgilaSession.checkConsistency(elements: HashMap<UUID, Element>, global: UUID, info: String?= "") {
    if (elements[global] == null)
        throw Exception("($info) Inconsistent elements: no Global.")
    elements.forEach {
        if (it.value.elementId != global && it.value.owner.ref?.elementId !in elements.keys)
            throw Exception("($info) Inconsistent element detected (owner not in elements): $it")
    }
}


/**
 * Function that does the semantic checks on the KerML model data.
 * Errors and issues are reported to the agenda.
 */
fun AgilaSession.checkIdentifications() {

    // Checks whether superclass was resolved (e.g. x isA y, with unknown y).
    // Also checks Feature types.
    val elements = get()
    elements.forEach {
        if (it is Type) {
            if (it.name != "Any")
                if (it.superclass?.ref == null) {
                    if (it.superclass == null ) {
                        reportInfo(it, "The type  of '${it.qualifiedName}' is not given -- give a definition!")
                    } else {
                        reportInfo(it, "The class '${it.superclass?.str}' is not defined -- give a definition!")
                    }
                }
        }
    }

    // Checks whether definitions were resolved (e.g. x hasA y, with unknown x).
    getUnownedElements().forEach {
        if (it.first.str != "Global") {
            report(global, "Could not resolve owner '${it.first}' of '${it.second.qualifiedName}'")
        }
    }
}


fun AgilaSession.checkConsistencyOfBuilders() {
    get().forEach {element ->
        if (element.model?.builder != this.builder)
            throw Exception("Inconsistent model: reference to other model.")
        if (element.model?.builder != this.builder)
            throw Exception("Inconsistent model: reference to other builder.")
        if (element is Relationship) {
            element.target.forEach {
                if(it.ref != null && it.ref?.model != this)
                    throw Exception("Inconsistent model: reference in targets of ${it}.")
            }
            element.source.forEach {
                if (it.ref != null && it.ref?.model != this)
                    throw Exception("Inconsistent model: reference in sources of ${it}.")
            }
        }
        if (element is Specialization)
            if (element.superclass.ref != null && element.superclass.ref?.model?.builder != builder)
                report(element, "Inconsistent builder: in Specialization $element")

        if (element is ValueFeature) {
            if (element.superclass?.ref?.model != null && element.superclass?.ref?.model?.builder != builder)
                report(element, "Inconsistent builder: in Value $element")
        }
    }
}


fun AgilaSession.checkOwnership() {
    get().forEach {  element ->
        if (element.owner.ref != null) {
            val ownedByOwner = element.owner.ref?.ownedElements?.associateBy { it.id }?.keys
            if (element.owner.ref != null && element.owner.id != null && element.owner.ref != get(element.owner.id!!))
                report(element, "owner id and ref not consistent.")
            if (ownedByOwner != null && element.elementId !in ownedByOwner)
                report(element, "owner does not refer correctly to owned element.")
        }
    }
}
