package SysMDRestImport.entities;

import SysMDRestImport.entities.user.User
import SysMDRestImport.entities.user.UserGroup
import com.fasterxml.jackson.annotation.*

import java.time.ZonedDateTime
import java.util.*
import kotlin.collections.ArrayList

/**
 * A 'Commit' is a class that holds
 * - a set of elements,
 * - properties, and
 * - relationships.
 * To retrieve and search the elements properly, the global node is given.
 * The 'Any' node is there as well, but shared with all projects.
 * Project is a subclass with specific extensions.
 */
//@Document("CommitDataObject")
class CommitDataObject(
    @JsonProperty("@id")
    var id: UUID = UUID.randomUUID(),

    var payloadElementSnapshot: ElementSnapshot,

    var element: Element,
)

open class Commit {
    @JsonProperty("@id")
    var id: UUID = UUID.randomUUID()

    @JsonProperty("@type")
    var type:String="Commit"

    @JsonProperty("previousCommit")
    @JsonInclude(JsonInclude.Include.NON_NULL)
    var previousCommit: ArrayList<Commit> = arrayListOf<Commit>()

    @JsonProperty("owningProject")
    @JsonInclude(JsonInclude.Include.ALWAYS)
    var owningProject: Project? = null

    @JsonProperty("description")
    @JsonInclude(JsonInclude.Include.NON_NULL)
    var description: String? = null

    @JsonIgnore
    var payload: List<CommitDataObject> = arrayListOf<CommitDataObject>()

    @JsonIgnore
    var createdBy: User? = null

    @JsonIgnore
    var modifiedBy: User? = null

    @JsonProperty("created")
    var created: ZonedDateTime = ZonedDateTime.now()

    @JsonIgnore
    var modified: ZonedDateTime? = null

    //    @Ref
    @JsonIgnore
    var userGroups: List<UserGroup> = ArrayList()

    override fun toString() =
        "Commit { project = $owningProject, description = $description, #elements = ${payload.size} }f"
}