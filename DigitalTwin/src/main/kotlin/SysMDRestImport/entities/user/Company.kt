package SysMDRestImport.entities.user

import SysMDRestImport.entities.user.security.Constants
import SysMDRestImport.entities.user.security.SecurityGroup
import com.fasterxml.jackson.annotation.JsonIgnore
import com.fasterxml.jackson.annotation.JsonProperty

import java.util.function.Function
import java.util.stream.Collectors

//@Getter
//@Setter
//@Document("Companies")
class Company : SecurityGroup {
    //    @Id
    @JsonProperty("@id")
    internal val id: String? = null
    var name: String? = null
    var description: String? = null

    @JsonIgnore //    @Relations(edges = HasGroup.class, lazy = true, direction = Relations.Direction.OUTBOUND)

    private val userGroups: Collection<UserGroup>? = null
    override fun toString(): String {
        return "Company:" +
                "id='" + id + '\'' +
                ", name='" + name + '\'' +
                ", description='" + description + '\'' +
                ", userGroups=" + if (userGroups == null) "null" else userGroups.stream()
            .map<String>(Function<UserGroup, String> { userGroup: UserGroup -> (userGroup.name + ":" + userGroup.id).toString() + ";" })
            .collect(
                Collectors.joining()
            )
    }

    override val securityIdentifier: String
        get() = (id + "_" + name + "_" + groupType).replace(' ', '-')
    override val parentGroup: SecurityGroup
        get() = Constants.ROOT_GROUP
}
