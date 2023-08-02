package SysMDRestImport.entities;

import SysMDRestImport.entities.user.User
import com.fasterxml.jackson.annotation.*
import java.time.LocalDateTime
import java.time.ZonedDateTime
import java.util.*

open class Branch : CommitReference {

    @JsonProperty("@id")
    override var id: UUID  = UUID.randomUUID()

    @JsonProperty("@type")
    var type:String="Branch"

    @JsonProperty("name")
    override var name: String = "name"

    @JsonProperty("description")
    override var description: String = ""

    @JsonProperty("referencedCommit")
    @JsonInclude(JsonInclude.Include.NON_NULL)
    override var referencedCommit: Commit? = null
//    var alias: ArrayList<String> = ArrayList()

//    @JsonIdentityInfo(generator = ObjectIdGenerators.PropertyGenerator::class, property = "id")
//    @JsonIdentityReference(alwaysAsId = true)
    @JsonProperty("owningProject")
    var owningProject: Project? = null

    @JsonProperty("head")
    @JsonInclude(JsonInclude.Include.NON_NULL)
    var head: Commit? = null

    @JsonIgnore
    var createdBy: User? = null

    @JsonIgnore
    var modifiedBy: User? = null

    @JsonIgnore
    var created: ZonedDateTime? = ZonedDateTime.now()

    @JsonIgnore
    var modified: LocalDateTime? = null

    override fun toString() =
        "Branch { id = $id, name = $name, owningProject = $owningProject, referencedCommit = $referencedCommit, created = $created }"

}
