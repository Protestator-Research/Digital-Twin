package SysMDRestImport.entities.user.edges

import SysMDRestImport.entities.user.Company
import SysMDRestImport.entities.user.UserGroup
import com.fasterxml.jackson.annotation.JsonProperty


//@Getter
//@Setter
//@Edge
class HasGroup(//    @From
    private val company: Company, //    @To
    private val userGroup: UserGroup
) {
    //    @Id
    @JsonProperty("@id")
    private val id: String? = null
    override fun toString(): String {
        return "HasGroup{" +
                "id='" + id + '\'' +
                ", company=" + company +
                ", userGroup=" + userGroup +
                '}'
    }
}
