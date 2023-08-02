package SysMDRestImport.entities.user.edges

import SysMDRestImport.entities.user.User
import SysMDRestImport.entities.user.UserGroup
import com.fasterxml.jackson.annotation.JsonProperty

//@Getter
//@Setter
//@Edge
class HasMember(//    @From
    private val userGroup: UserGroup, //    @To
    private val user: User
) {
    //    @Id
    @JsonProperty("@id")
    private val id: String? = null
    override fun toString(): String {
        return "HasMember{" +
                "id='" + id + '\'' +
                ", userGroup=" + userGroup +
                ", user=" + user +
                '}'
    }
}
