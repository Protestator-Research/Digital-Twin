package ImportedSysMDServices

import Parser.loadSysMDFromResources
import SysMDRestImport.ElementDAO
import java.util.*


/**
 * Basic Interface between SysMD Notebook and Agila.
 * It can be implemented by the simple mock repository that is integrated in SysMD Notebook,
 * or by the Agila backend that is accessed via REST.
 */
interface BasicRepository {

    /**
     * Retrieves all information of a commit that is selected by its name
     * - Elements, Properties, Relations.
     * The name has the format "[project:]path/filename[:versionNr]
     * @param projectName a string that uniquely identifies a revision.
     */
    @Deprecated("Use getAllElements instead")
    fun getCommitPayload(projectName: String): List<ElementDAO>
    @Deprecated("Use getAllElements instead")
    fun getCommitPayload(projectId: UUID, commitId: UUID): List<ElementDAO>

    /**
     * Gets all elements of the default branch heda of a project by name.
     */
    fun getAllElements(projectName: String): List<ElementDAO>

    /**
     * Gets all elements of a project by project and commit id.
     */
    fun getAllElements(projectId: UUID, commitId: UUID?): List<ElementDAO>

    /**
     * Writes all information from a commit into the repository, and returns an id of the commit.
     * When writing to the repository, the packages among the elements will be tagged with the revision tag that will also be returned in response.
     * @param name a user-given name
     * @param description a brief description of the commit
     * @param changes, the committed data, consisting of a data class CommitData with
     * - all elements defined in the commit,
     * - all properties defined in the commit
     * - all relations defined in the document
     * - globalUId of the root of decomposition
     * @param previousCommit the id of the previous version of the document
     * @return An id that uniquely identifies the commit
     */
    fun commit(
        owningProject: UUID,
        name: String?,
        description: String?,
        changes: List<ElementDAO>,
        previousCommit: UUID? = null): UUID
}


/**
 * Just a dummy for test and simple independent client.
 * Other implementation is the Backend via REST.
 */
object AgilaRepositorySysMD: BasicRepository {
    private var connectedWithBackend:  Boolean = false

    /**
     * We keep projects in the simple standalone client as a map Name -> Pair of Elements and the Root-Namespace.
     */
    val projectsLoaded = hashMapOf<String, List<ElementDAO>>()


    fun reset() {
        projectsLoaded.clear()
    }


    @Deprecated("Replace with getAllElements")
    override fun getCommitPayload(projectName: String): List<ElementDAO> = getAllElements(projectName)
    @Deprecated("Replace with getAllElements")
    override fun getCommitPayload(projectId: UUID, commitId: UUID): List<ElementDAO> {
        TODO("Not yet implemented")
    }

    override fun getAllElements(projectId: UUID, commitId: UUID?): List<ElementDAO> {
        TODO("Not implemented; just dummy for standalone.")
    }


    /**
     * Tries to get a document from the repository; if there is
     * no connection to the backend repository, it will be retrieved from
     * 1) local repository used as cache;
     * 2) the local SysMD folder, if not in the cache.
     */
    override fun getAllElements(projectName: String): List<ElementDAO>  {
        if (!connectedWithBackend) {
            // If already loaded, just return existing payload
            if (projectsLoaded[projectName] != null)
                return projectsLoaded[projectName]!!

            println("Loading: $projectName.md")
            val model = AgilaSessionImpl()
            try {
                model.loadSysMDFromResources("$projectName.md", false)
            } catch (error: Exception) {
                model.report(error)
            }
            if (model.status.errors.size > 0)
                println(model.status.errors)
            projectsLoaded[projectName] = model.export()
            return projectsLoaded[projectName]!!
        } else {
            TODO("Connected with Backend -- but using SysMD standalone repository. Weird.")
        }
    }

    /**
     * Simple dummy that writes a commit to the default branch of a project.
     */
    override fun commit(
        owningProject: UUID,
        name: String?,
        description: String?,
        changes: List<ElementDAO>,
        previousCommit: UUID?
    ): UUID {
        projectsLoaded[name!!] = changes
        return UUID.randomUUID() // NOTE: Now way to get it back in this dummy.
    }
}