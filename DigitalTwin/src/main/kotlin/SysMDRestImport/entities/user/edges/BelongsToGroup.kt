package SysMDRestImport.entities.user.edges

import SysMDRestImport.entities.user.User
import SysMDRestImport.entities.user.UserGroup
import com.fasterxml.jackson.annotation.JsonProperty

class BelongsToGroup(//    @From
    private val user: User, //    @To
    private val userGroup: UserGroup
) {
    //    @Id
    @JsonProperty("@id")
    private val id: String? = null
    override fun toString(): String {
        return "BelongsToGroup{" +
                "id='" + id + '\'' +
                ", user=" + user +
                ", userGroup=" + userGroup +
                '}'
    }
}
