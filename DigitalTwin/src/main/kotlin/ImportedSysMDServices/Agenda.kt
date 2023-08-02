package ImportedSysMDServices



import BaseEntities.TextualRepresentation
import ImportedSysMDExceptions.SysMDException
import kotlin.reflect.KClass

/**
 * The object Agenda stores and provides access to the agenda storing errors
 * which occurred on elements defined in SysMD cells
 */
class Agenda(
    private var kerMlModel: AgilaSession
) {
    /**
     * The status of the overall model; includes among others a mutable set of infos, errors, etc. .
     */
    val status: SessionStatus
        get() = kerMlModel.status


    /**
     * Determines whether the error agenda window shall be active or not
     */
    /*val active: MutableState<Boolean> = mutableStateOf(false)*/


    /**
     * Stores qualified names of all undefined elements
     */
    private val agenda: ArrayList<AgendaElement> = arrayListOf()

    /**
     * Adds elements to [agenda]
     */
    private fun addElement(
        qualifiedName: String,
        exceptionClass: KClass<out SysMDException>,
        errorMessage: String = "",
        textualRepresentation: TextualRepresentation?,
        line: Int = -1
    ) {
        agenda.add(
            AgendaElement(
                qualifiedName = qualifiedName,
                exceptionClass = exceptionClass,
                errorMessage = errorMessage,
                textualRepresentation = textualRepresentation,
                line = line
            )
        )
    }

    /**
     * Clears [agenda]
     */
    fun clear() {
        agenda.clear()
    }

    /**
     * Returns true if element found
     */
    fun contains(new: AgendaElement): Boolean {
        return agenda.contains(new)
    }

    /**
     * Returns all elements from [agenda][com.github.tukcps.sysmd.services.Agenda.agenda]
     */
    fun getIssues(): List<AgendaElement> {
        return agenda
    }

    /**
     * Returns true if agenda is empty
     */
    fun isEmpty(): Boolean {
        return agenda.isEmpty()
    }

    /**
     * Removes elements from [agenda]
     * IMPORTANT:
     * 1) Lines always refer to a TextualRepresentation ... I would also add textual representation in Agenda.
     * 2) I wonder if agenda might be positioned best as a "View Model".
     */
    fun removeElement(
        qualifiedName: String,
        textualRepresentation: TextualRepresentation,
        line: Int
    ) {
        agenda.removeIf { it.qualifiedName == qualifiedName && it.textualRepresentation == textualRepresentation && it.line == line }
    }

    /**
     * Returns the number of all agenda elements of all types
     */
    fun size(): Int {
        return agenda.size
    }

    /**
     * Analyzes, sorts, manages error and status message.
     */
    fun update() {
        status.exceptions.forEach {
            addElement(
                qualifiedName = it.element?.qualifiedName ?: "",
                errorMessage = it.message,
                exceptionClass = it::class,
                textualRepresentation = it.textualRepresentation,
                line = it.token?.lineNo ?: -1
            )
        }
    }
}