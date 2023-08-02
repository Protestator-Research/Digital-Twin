package SysMDRestImport.entities

//import com.arangodb.springframework.annotation.Document
//import com.arangodb.springframework.annotation.Ref
//import com.arangodb.springframework.annotation.To
//import org.springframework.data.annotation.Id
import com.fasterxml.jackson.annotation.JsonIgnore
import com.fasterxml.jackson.annotation.JsonProperty
import java.time.ZonedDateTime
import java.util.*


/**
 * As a project we model a sequence of commits.
 * The project is the first commit and has additional properties:
 * - a collection of all commits
 * - access methods for finding head, branches, tags. (t.b.d.)
 */
//@Document("Projects")
open class Project(
//    @Id

//    override var alias: ArrayList<String> = ArrayList()
) : Record {

    @JsonProperty("@id")
    override var id: UUID = UUID.randomUUID()

    @JsonProperty("@type")
    var type:String="Project"

    @JsonProperty("name")
    override var name: String = ""

    @JsonProperty("description")
    override var description: String = ""

    @JsonProperty("created")
    var created: ZonedDateTime = ZonedDateTime.now()

    @Deprecated("legacy: represented the head of the default branch")
    val default: UUID?
        get() = defaultBranch?.referencedCommit?.id // ID of head of default branch

//    @Ref
    @JsonProperty("defaultBranch")
    var defaultBranch: Branch? = null

//    @Ref
    @JsonIgnore
    var branches: MutableCollection<Branch> = arrayListOf()

    // Ids of all commits
    @JsonIgnore
    var commits: MutableCollection<Commit> = arrayListOf()


    // Ids of all heads
    val heads: List<UUID>
        get() {
            return branches.groupBy { b -> b.referencedCommit?.id }.keys.filterNotNull()
        }


    override fun toString() =
        "Project { id = $id, name = $name, #commits = ${commits.size}, default = ${defaultBranch?.name} }"
}