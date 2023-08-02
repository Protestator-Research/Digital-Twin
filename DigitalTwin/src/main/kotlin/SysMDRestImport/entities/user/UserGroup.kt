package SysMDRestImport.entities.user

import SysMDRestImport.entities.user.security.SecurityGroup
import com.fasterxml.jackson.annotation.JsonIgnore
import com.fasterxml.jackson.annotation.JsonProperty

//@Getter
//@Setter
//@Document("User_Groups")
class UserGroup : SecurityGroup {
    //    @Id
    @JsonProperty("@id")
    internal val id: String? = null
    internal val name: String? = null
    private val description: String? = null

    @JsonIgnore //    @Relations(edges = HasMember.class, lazy = true, direction = Relations.Direction.OUTBOUND)

    private val members: Collection<User>? = null

    @JsonIgnore //    @Ref

    private val companyRef: Company? = null
    override val parentGroup: SecurityGroup? = null
    override fun toString(): String {
        return "UserGroup{" +
                "id='" + id + '\'' +
                ", name='" + name + '\'' +
                ", description='" + description + '\'' +
                ", members=" + members +
                ", companyRef=" + companyRef +
                '}'
    }

    override val securityIdentifier: String
        get() = (id + "_" + name + "_" + groupType).replace(' ', '-')
}
