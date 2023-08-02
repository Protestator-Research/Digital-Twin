package ImportedSysMDServices


import ImportedSysMDExceptions.SysMDError
import ImportedSysMDExceptions.SysMDException
import ImportedSysMDExceptions.SysMDInfo
import java.util.*

/**
 * In this class we save the status of the current analysis
 */
class SessionStatus {
    var source: String = ""

    /** The number of iterations used in the constraint propagation */
    var numberOfPropagateIterations: Int = 0

    // Hashmap of error messages, property id is key, string (error message).
    val exceptions = mutableSetOf<SysMDException>()

    val numberOfErrors: Int
        get() = exceptions.filterIsInstance<SysMDError>().size

    val numberOfInfos: Int
        get() = exceptions.filterIsInstance<SysMDInfo>().size

    val infos: List<SysMDInfo>
        get() = exceptions.filterIsInstance<SysMDInfo>()

    val errors: List<SysMDError>
        get() = exceptions.filterIsInstance<SysMDError>()

    // Hashmap of updated properties, property id is key, and string (updated result).
    val updates: HashMap<UUID, String> = hashMapOf()

    /**
     * Resets all internal values: source, lineNo, columnNo, and the mapx errors, errorsByLine,
     * and updates.
     */
    fun reset() {
        source = ""
        exceptions.clear()
        updates.clear()
    }

    override fun toString(): String =
        "Status: $numberOfErrors errors,${numberOfInfos} infos, ${updates.size} values updated."

}