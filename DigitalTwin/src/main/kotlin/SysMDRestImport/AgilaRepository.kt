package SysMDRestImport

import BaseEntities.Relationship
import BaseEntities.ValueFeature
import BaseEntities.implementation.TextualRepresentationImplementation
import ImportedSysMDServices.BasicRepository
import SysMDRestImport.entities.*
import SysMDRestImport.entities.requests.*
import SysMDRestImport.entities.requests.commitData.ElementCommitData
import com.fasterxml.jackson.databind.ObjectMapper
import com.fasterxml.jackson.datatype.jsr310.JavaTimeModule
import com.fasterxml.jackson.module.kotlin.readValue

import java.util.*
import kotlin.collections.ArrayList


var OnlineState = false

/**
 * An object that handle communication with the agila backend project repository via REST.
 */
object AgilaRepository: BasicRepository {
    private val objectMapper = ObjectMapper()
    private var writer = objectMapper.writer().withDefaultPrettyPrinter()
    private const val admin_password = "admin"
    private const val admin_unsername = "admin@cps.de"
    private const val user_key = "email"
    private const val password_key = "password"

    init {
        objectMapper.registerModule(JavaTimeModule())
    }
    /**
     * Id of the session working with the REST API;
     * Must be set by the Session Manager at client side prior first use of the Agila backend.
     */
    var internalSessionId: UUID = UUID.randomUUID()
    var serverSessionId:UUID? = null

    var projectsState: ArrayList<Project> = getProjects()


    /**
     * @param projectId projectId of the project
     * @return the Project
     */
    fun getProjectById(projectId: UUID): Project {
        Rest.login("/users/login", user_key, admin_unsername, password_key, admin_password)
        val response =
            if(serverSessionId!=null)
                Rest.get("/projects/${projectId.toString()}", serverSessionId.toString())
            else
                Rest.get("/projects/${projectId.toString()}", internalSessionId.toString())

        return objectMapper.readValue(response.body, Project::class.java)
    }

    /**
     * Gets a list of projects.
     */
    fun getProjects(): ArrayList<Project> =
        try {
            serverSessionId = postSession()
            Rest.login("/users/login", user_key, admin_unsername, password_key, admin_password)
            val response = if(serverSessionId!=null)
                    Rest.get("/projects", serverSessionId.toString())
                else
                    Rest.get("/projects", internalSessionId.toString())


            when (response.statusCodeValue) {
                200 -> {
                    OnlineState=true
                }
                201 -> {
                    OnlineState=true
                }
                400 -> OnlineState=false
                403 -> OnlineState=false
            }


            println(response.body)

            objectMapper.readValue<ArrayList<Project>>(response.body!!)
        } catch (io: Exception) {
            println("Exception: " + io.stackTraceToString())
            arrayListOf<Project>()
        }


    /**
     * Returns all the commits of a project
     */
    fun getCommits(projectId: UUID?): ArrayList<Commit> =
        try {
            Rest.login("/users/login", user_key, admin_unsername, password_key, admin_password)
            val commitsResponse = if(serverSessionId!=null)
                Rest.get("/projects/$projectId/commits", serverSessionId.toString())
                else
                Rest.get("/projects/$projectId/commits", internalSessionId.toString())

            val commits = commitsResponse.body
            objectMapper.readValue<ArrayList<Commit>>(commits!!)
        } catch (io: Exception) {
            println(io.stackTraceToString())
            arrayListOf()
        }


    fun getCommit(projectId: UUID, commitId: UUID?): Commit {
        Rest.login("/users/login", user_key, admin_unsername, password_key, admin_password)
        val projectResponse = if(serverSessionId!=null) Rest.get("/projects/$projectId", serverSessionId.toString()) else Rest.get("/projects/$projectId", internalSessionId.toString())
        val project = objectMapper.readValue(projectResponse.body, Project().javaClass)

        project.defaultBranch = getBranchById(projectId, project.defaultBranch?.id ?: UUID.randomUUID())

        val commitId2 = commitId ?: project.defaultBranch!!.head
        val commitResponse = Rest.get("/projects/$projectId/commits/$commitId2", internalSessionId.toString())

        println(commitResponse.body)

        return objectMapper.readValue<Commit>(commitResponse.body!!)
    }


    fun postBranch(projectID:UUID, name: String?, headOfBranch:UUID): Branch? {
        var branch : Branch? = null
        val branchId : String

        try {
            Rest.login("/users/login", user_key, admin_unsername, password_key, admin_password)
            val branchReq= BranchRequest(name=name, id = headOfBranch)

            println(writer.writeValueAsString(branchReq))

            val postProjectResponse = if(serverSessionId!=null)
                Rest.post("/projects/${projectID}/branches", writer.writeValueAsString(branchReq), serverSessionId.toString())
            else
                Rest.post("/projects/${projectID}/branches", writer.writeValueAsString(branchReq), internalSessionId.toString())

            println(postProjectResponse)

            val resultBody: String = postProjectResponse.body ?: throw Exception("empty result body")

            when (postProjectResponse.statusCodeValue) {
                200 -> {
                    println("--- " + postProjectResponse.statusCodeValue + " ---" + resultBody)
                    branchId = Rest.extractEntityIdFromBody(resultBody).toString()
                    branch = getBranchById(projectID,UUID.fromString(branchId))
                }
                201 -> {
                    println("---" + postProjectResponse.statusCodeValue + " ---" + resultBody)
                    branchId = Rest.extractEntityIdFromBody(resultBody).toString()
                    branch = getBranchById(projectID,UUID.fromString(branchId))

                    // Update the projectState variable with a new list of projects
                    val newProjectsState= getProjects()
                    projectsState = newProjectsState
                }
                400 -> println("---" + postProjectResponse.statusCodeValue + "---" + resultBody)
                403 -> println("---" + postProjectResponse.statusCodeValue + "---" + resultBody)
            }
            return branch

        } catch (io: Exception) {
            println(io.stackTraceToString())
            return branch
        }
    }

    fun getBranchById(projectId: UUID,branchId:UUID) : Branch {
        Rest.login("/users/login", user_key, admin_unsername, password_key, admin_password)
        val branchResponse = if(serverSessionId!=null)
            Rest.get("/projects/${projectId}/branches/${branchId}", serverSessionId.toString())
        else
            Rest.get("/projects/${projectId}/branches/${branchId}", internalSessionId.toString())

        return objectMapper.readValue<Branch>(branchResponse.body!!)
    }

    fun deleteBranch(projectID: UUID, branchId: UUID): Boolean {
        var deleted = false

        try {
            Rest.login("/users/login", user_key, admin_unsername, password_key, admin_password)
            // TODO(Check the Value of payload?)

            val payload ="""
                {
              "@id": "$branchId"
                }
            """.trimIndent()

            val response = if(serverSessionId!=null)
                Rest.delete("/projects/$projectID/branches/$branchId", payload, serverSessionId.toString())
            else
                Rest.delete("/projects/$projectID/branches/$branchId", payload, internalSessionId.toString())

            println(response.body)

            when (response.statusCodeValue) {
                200 -> {
                    println("--- " + response.statusCodeValue + " ---" + response.body)
                    deleted = true
                }
                400 -> println("---" + response.statusCodeValue + "---" + response.body)
                401 -> println("---" + response.statusCodeValue + "---" + response.body)
                403 -> println("---" + response.statusCodeValue + "---" + response.body)
                500 -> println("---" + response.statusCodeValue + "---" + response.body)
            }
            return deleted

        } catch (io: Exception) {
            io.printStackTrace()
            throw Exception("No project with the id: $projectID")
        }
    }


    fun getBranchesForProject(projectID: UUID):MutableList<Branch> {
        return try {
            Rest.login("/users/login", user_key, admin_unsername, password_key, admin_password)
            val branchResponse = if(serverSessionId!=null)
                Rest.get("/projects/${projectID}/branches", serverSessionId.toString())
            else
                Rest.get("/projects/${projectID}/branches", internalSessionId.toString())
            val branchesParser = objectMapper.typeFactory?.constructCollectionType(
                mutableListOf<Branch>().javaClass,
                Branch().javaClass
            )
            objectMapper.readValue(branchResponse.body, branchesParser)
        } catch (io: Exception) {
            mutableListOf<Branch>()
        }
    }
    override fun getAllElements(projectId: UUID, commitId: UUID?): MutableList<ElementDAO> {
        Rest.login("/users/login", user_key, admin_unsername, password_key, admin_password)

        println(projectId)
        println(commitId)

        val elementResponse = if(serverSessionId!=null)
                Rest.get("/projects/$projectId/commits/$commitId/elements", "")
            else
                Rest.get("/projects/$projectId/commits/$commitId/elements", internalSessionId.toString())

        print(elementResponse.body)

        val elements = objectMapper.readValue<MutableList<Element>>(elementResponse.body)
        val returnValue = mutableListOf<ElementDAO>()

        for(element in elements)
            returnValue.add(element.toElementDAO())

        return returnValue
    }

    /** For later use ?
     * @param projectId ID of the project
     * @param commitId ID of the commit
     * @param elementId ID of the element
     * @return An element by project, commit and its Id
     */
    fun getElementById(projectId: UUID, commitId: UUID?, elementId: UUID?): ElementDAO {
        Rest.login("/users/login", user_key, admin_unsername, password_key, admin_password)
        val elementResponse = if(serverSessionId!=null)
                Rest.get("/projects/$projectId/commits/$commitId/elements/$elementId", serverSessionId.toString())
            else
                Rest.get("/projects/$projectId/commits/$commitId/elements/$elementId", internalSessionId.toString())
        val element = objectMapper.readValue<Element>(elementResponse.body)
        return element.toElementDAO()
//        return objectMapper.readValue(elementResponse.body, ElementDAO(elementId=UUID.randomUUID()).javaClass)
    }

    /**
     * Used to display Elements of a Project in an EditorTab
     * @param projectId ID of the project
     * @param commitId ID of the commit
     * @return All elements in a from the current commit in a project as a MutableList<Element>
     */
    fun getElementsForUi(projectId: UUID?, commitId: UUID? = null): MutableList<TextualRepresentationImplementation> {
        val textualRepresentations : MutableList<TextualRepresentationImplementation>  = mutableListOf()
        val elementsDAO = getAllElements(projectId!!,commitId!!)
        for (elementDAO in elementsDAO){
            if (elementDAO.type == "TextualRepresentation"){
                val textualRepresentation = elementDAO.toElement()
                textualRepresentations.add(textualRepresentation as TextualRepresentationImplementation)
            }
        }
        return textualRepresentations
    }

    fun getElementsForUi2(projectId: UUID?, commitId: UUID? = null): MutableList<UUID> {

        val cellList : MutableList<UUID> = mutableListOf()
        val elementsDAO = getAllElements(projectId!!,commitId!!)

        for (elementDAO in elementsDAO){
            if (elementDAO.type == "AnnotatingElement"){
                for (elementUUID in elementDAO.ownedElements!!) {
                    cellList.add(elementUUID)
                }
            }
        }
        return cellList
    }

    @Suppress("unused_parameter")
    fun getProperties(projectId: UUID, commitId: UUID?=null): MutableList<ValueFeature> {
        Rest.login("/users/login", user_key, admin_unsername, password_key, admin_password)
        TODO()
    }

    @Suppress("unused", "unused_parameter")
    fun getRelationships(projectId: UUID, commitId: UUID? = null): MutableList<Relationship> {
        Rest.login("/users/login", user_key, admin_unsername, password_key, admin_password)
        TODO()
    }

    /**
     * @param projectName name of the project
     * @param description description of the project
     * @return The project created
     */
    fun postProject(projectName: String, description: String?, defaultBranchName:String = "Main"): Project?{
        var project : Project? = null
        val projectId : String

        try {
            Rest.login("/users/login", user_key, admin_unsername, password_key, admin_password)
            val projectReq = ProjectRequest(name=projectName,description = description,defaultBranchName = defaultBranchName)

            val postProjectResponse = if(serverSessionId!=null)
                Rest.post("/projects/", writer.writeValueAsString(projectReq), serverSessionId.toString())
                else
                Rest.post("/projects/", writer.writeValueAsString(projectReq), internalSessionId.toString())
            val resultBody: String = postProjectResponse.body ?: throw Exception("empty result body")

            when (postProjectResponse.statusCodeValue) {
                200 -> {
                    println("--- " + postProjectResponse.statusCodeValue + " ---" + resultBody)
                    projectId = Rest.extractEntityIdFromBody(resultBody).toString()
                    project = getProjectById(UUID.fromString(projectId))
                }
                201 -> {
                    println("---" + postProjectResponse.statusCodeValue + " ---" + resultBody)
                    projectId = Rest.extractEntityIdFromBody(resultBody).toString()
                    project = getProjectById(UUID.fromString(projectId))

                    // Update the projectState variable with a new list of projects
                    val newProjectsState= getProjects()
                    projectsState = newProjectsState
                }
                400 -> println("---" + postProjectResponse.statusCodeValue + "---" + resultBody)
                403 -> println("---" + postProjectResponse.statusCodeValue + "---" + resultBody)
            }
            return project

        } catch (io: Exception) {
            throw Exception("No project: $projectName")
        }
    }

    /** For the moment just Used in Test to deleted posted projects in test
     * @param projectID
     * @return true or false, if the deletion was successful or not
     */
    fun deleteProject(projectID: UUID): Boolean {
        var deleted = false

        try {
            Rest.login("/users/login", user_key, admin_unsername, password_key, admin_password)

            val payload ="""
                {
              "@id": "$projectID"
                }
            """.trimIndent()

            val response = if(serverSessionId!=null)
                    Rest.delete("/projects/$projectID", payload, serverSessionId.toString())
                else
                    Rest.delete("/projects/$projectID", payload, internalSessionId.toString())

            when (response.statusCodeValue) {
                200 -> {
                    println("--- " + response.statusCodeValue + " ---" + response.body)
                    deleted = true
                }
                400 -> println("---" + response.statusCodeValue + "---" + response.body)
                401 -> println("---" + response.statusCodeValue + "---" + response.body)
                403 -> println("---" + response.statusCodeValue + "---" + response.body)
                500 -> println("---" + response.statusCodeValue + "---" + response.body)
            }
            return deleted

        } catch (io: Exception) {
            throw Exception("No project with the id: $projectID")
        }
    }

    /**
     * @param commitName name of the commit
     * @param commitDescription description of the commit
     * @param  projectId
     * @param  elementsDAOList MutableList<ElementDAO>?
     * @return the created commit or null
     */
    fun postCommit(
        commitName: String,
        commitDescription: String,
        projectId: UUID,
        elementsDAOList: MutableList<ElementDAO>? = mutableListOf()
    ): Commit {

        Rest.login("/users/login", user_key, admin_unsername, password_key, admin_password)

        val changeList = arrayListOf<DataVersionRequest>()

        if (elementsDAOList != null) {
            if (elementsDAOList.isNotEmpty()) {
                for (element in elementsDAOList)
                {

                    val commitData = ElementCommitData(
                        id = element.elementId,
                        type = element.type !!,
                        name = element.name,
                        shortName = element.shortName,
                        ownedElement = element.ownedElements?.map { Identity(id=it) }?.toMutableList() ?: arrayListOf(),
                        owner = Identity(id = element.owner),
    //                    language = element.language,
    //                    textualRepresentation = element.elementId
                    )
                    commitData.language = element.language
                    commitData.body = element.body
                    val dataVersionRequest = DataVersionRequest(payload = commitData)
                    changeList.add(dataVersionRequest)
                }
            }
        }

        val commitRequest = CommitRequest(description = commitDescription, change = changeList)

        val dataString = writer.writeValueAsString(commitRequest)

        println(dataString)


        val postCommitResponse = if(serverSessionId!=null)
                Rest.post("/projects/$projectId/commits", dataString, "")
            else
                Rest.post("/projects/$projectId/commits", dataString, "")
        val resultBody: String = postCommitResponse.body ?: throw Exception("empty result body")
        val commitId : UUID?
        var commit : Commit? = null

        when (postCommitResponse.statusCodeValue) {
            200 -> {
                println("--- " + postCommitResponse.statusCodeValue + " ---" + resultBody)
                 commitId = UUID.fromString(Rest.extractEntityIdFromBody(resultBody))
                 commit = getCommit(projectId, commitId)
            }
            201 -> {
                println("---" + postCommitResponse.statusCodeValue + " ---" + resultBody)
                commitId = UUID.fromString(Rest.extractEntityIdFromBody(resultBody))
                commit = getCommit(projectId, commitId)

            }
            400 -> println("---" + postCommitResponse.statusCodeValue + "---" + resultBody)
            403 -> println("---" + postCommitResponse.statusCodeValue + "---" + resultBody)
        }

        if (commit == null){
           throw Exception("Commit Unsuccessful: $commitName")
        }

        return commit
    }

    /**
     * Implements method from interface!
     */
    override fun getAllElements(projectName: String): List<ElementDAO> {
        try {
            val project = projectsState.first { it.name == projectName }
            val projectId = project.id
            //TODO commits müssen anders behandelt werden
//            val commitId = project.headIDs.first()
//            val elementDaoList = getAllElements(projectId, commitId)
//            val elements = mutableListOf<ElementDAO>()
//
//            for (elementDAO in elementDaoList){
//                elements.add(elementDAO)
//            }
//
//            return elements
            return arrayListOf<ElementDAO>()

        } catch (notLoaded: Exception) {
            println("Could not connect to backend via REST.")
            return emptyList()
        }
    }


    fun postSession() : UUID? {
        Rest.login("/users/login", user_key, admin_unsername, password_key, admin_password)

        val postCommitResponse = Rest.post("/sessions", "", internalSessionId.toString())
        val resultBody: String = postCommitResponse.body ?: throw Exception("empty result body")

        print(resultBody)

        when (postCommitResponse.statusCodeValue) {
            200 -> {
                println("--- " + postCommitResponse.statusCodeValue + " ---" + resultBody)
//                commitId = UUID.fromString(Rest.extractEntityIdFromBody(resultBody))
//                commit = getCommit(projectId, commitId)
            }
            201 -> {
                println("---" + postCommitResponse.statusCodeValue + " ---" + resultBody)
                val uuidString = Rest.extractEntityUUIDFromBody(resultBody)
                if(uuidString!=null)
                    return UUID.fromString(uuidString)


//                commitId = UUID.fromString(Rest.extractEntityIdFromBody(resultBody))
//                commit = getCommit(projectId, commitId)
//
//                if (elementsDAOList != null) {
//                    for(elementDAO in elementsDAOList){
//                        commit.change.add(DataVersion( payload = elementDAO ))
//                    }
//                }
//                //add the new commit to the ProjectCommitIds List
//                getProjectById(projectId).commitIDs.add(commitId)
            }
            400 -> println("---" + postCommitResponse.statusCodeValue + "---" + resultBody)
            403 -> println("---" + postCommitResponse.statusCodeValue + "---" + resultBody)
        }
        return null
    }

    @Deprecated("Use getCommit instead")
    override fun getCommitPayload(projectName: String): List<ElementDAO> =
        getAllElements(projectName)


    @Deprecated("Use getAllElements")
    override fun getCommitPayload(projectId: UUID, commitId: UUID): List<ElementDAO> =
        getAllElements(projectId, commitId)

    override fun commit(
        owningProject: UUID,
        name: String?,
        description: String?,
        changes: List<ElementDAO>,
        previousCommit: UUID?
    ): UUID {
        val project = projectsState.forEach {
            // search for project
        }
        //
        // use updated endpoint /projects/id/commits/id to commit payload
        //
        TODO("Not yet implemented")
    }

    fun postDigitalTwin(projectId: UUID, request: DigitalTwinRequest):DigitalTwin ?{
        var dt:DigitalTwin? = null
        try {
            Rest.login("/users/login", user_key, admin_unsername, password_key, admin_password)

            print(writer.writeValueAsString(request))

            val postProjectResponse = if (serverSessionId != null)
                Rest.post(
                    "/projects/$projectId/digital-twin",
                    writer.writeValueAsString(request),
                    serverSessionId.toString()
                )
            else
                Rest.post(
                    "/projects/$projectId/digital-twin",
                    writer.writeValueAsString(request),
                    internalSessionId.toString()
                )
            val resultBody: String = postProjectResponse.body ?: throw Exception("empty result body")

            when (postProjectResponse.statusCodeValue) {
                200 -> {
                    println("--- " + postProjectResponse.statusCodeValue + " ---" + resultBody)
                    dt = objectMapper.readValue<DigitalTwin>(resultBody)
                }

                201 -> {
                    println("---" + postProjectResponse.statusCodeValue + " ---" + resultBody)
                    dt = objectMapper.readValue<DigitalTwin>(resultBody)

                    // Update the projectState variable with a new list of projects
                }

                400 -> println("---" + postProjectResponse.statusCodeValue + "---" + resultBody)
                403 -> println("---" + postProjectResponse.statusCodeValue + "---" + resultBody)
            }
        }catch (ex:Exception){
            ex.printStackTrace()

        }
        return dt
    }

    fun getDigitalTwinsFromProject(projectId: UUID) : MutableList<DigitalTwin> {
        return try {
            Rest.login("/users/login", user_key, admin_unsername, password_key, admin_password)

            val branchResponse = if(serverSessionId!=null)
                Rest.get("/projects/${projectId}/digital-twin", serverSessionId.toString())
            else
                Rest.get("/projects/${projectId}/digital-twin", internalSessionId.toString())

            print(branchResponse.body)

            objectMapper.readValue(branchResponse.body)
        } catch (io: Exception) {
            mutableListOf<DigitalTwin>()
        }
    }

    fun postRealTwinData(projectId: UUID, digitalTwinId:UUID, data:RealTwinDataRequest){
        Rest.post("/projects/${projectId}/real-twin-data/${digitalTwinId}", writer.writeValueAsString(data), "")
    }
}
