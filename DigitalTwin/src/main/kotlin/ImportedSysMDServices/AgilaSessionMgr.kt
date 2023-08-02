package ImportedSysMDServices

import Parser.loadProjectFromRepository
import java.util.*

/**
 * The session manager keeps a list of all open sessions.
 * Each session allows to exclusively edit a commit; no other sessions can be opened with this commit.
 * A session can be persisted or retrieved in a repository (commit).
 */
object AgilaSessionMgr {

    /**
     * The repository in which data will be persisted.
     * Dependency injection e.g. in Spring can create own repository;
     * then, this reference is overwritten by a more sophisticated service.
     */
    var repository: BasicRepository = AgilaRepositorySysMD

    /**
     * A map with all active sessions; hashmap of UId of commits and sessions working with it.
     */
    private val sessions = hashMapOf<UUID, AgilaSession>()

    @Suppress("unused")
    fun getSession(id: UUID) = sessions[id]

    /**
     * Gets all collections and puts its ids in a collection.
     * @return a collection of the ids of all sessions.
     */
    @Suppress("unused")
    fun getAllSessions(): Collection<UUID> {
        val response = mutableListOf<UUID>()
        sessions.forEach {
            response.add(it.key)
        }
        return response
    }

    /**
     * Starts a session; a session is for exclusive use by a single client.
     * @return AgilaService object
     */
    fun startSession(): AgilaSession = AgilaSessionImpl().run {
        sessions[id] = this
        initialize()
        return this
    }


    /**
     * Executes a lambda 'block' in the context of a session.
     * @param id UUID of the session
     * @param block lambda that is executed in the context of the session
     */
    @Suppress("unused")
    fun<T> runInSession(id: UUID, block: AgilaSession.() -> T): T {
        val session = sessions[id] ?: throw Exception("No such session")
        session.run { return block() }
    }


    /**
     * Starts a session, executes a lambda for testing purposes.
     * Just a shortcut for testing.
     * The session is terminated after the test has run.
     * The parameters permit to set up test cases:
     * @param load Documents that are loaded into the repository and test session prior to the test.
     * @param catchExceptions allows to disable catching exceptions in the parser and other methods.
     * @param initialize whether to run initialize or not.
     * @param test a lambda with the test.
     */
    fun testSession(vararg load: String,
                    catchExceptions: Boolean = true,
                    initialize: Boolean = true,
                    test: AgilaSession.() -> Unit) = AgilaSessionImpl().run {
        sessions[id] = this
        settings.catchExceptions = catchExceptions
        settings.initialize = initialize
        load.forEach { loadProjectFromRepository(it) }
        test()
        sessions.remove(id)
        settings.initialize = initialize
    }


    /**
     * runs a lambda in a session; no id, no persistence after the session ends.
     */
    fun runInSession(toRun: AgilaSession.() -> Unit) = AgilaSessionImpl().run {
        toRun()
    }


    /**
     * writes the session data into the DB and thereby creates a new revision
     * @param session an AgilaSession
     * @param projectId the id of the owning project
     * @param description the description of the commit
     * @return the UUID of the created commit
     */
    fun commitSession(session: AgilaSession, projectId: UUID, name: String = "", description: String = ""): UUID {
        return repository.commit(
            name = name,
            owningProject = projectId,
            description = description,
            changes = session.export(),
            previousCommit = null,
        )
    }

    /**
     * Removes a mapping from the sessions and returns the entry or null
     * if the id was not in the keys.
     * @param id UId of the session.
     */
    fun kill(id: UUID) = sessions.remove(id)
}
