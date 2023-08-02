package SysMDRestImport.entities.user.security

import java.util.*

/**
 * @author Philipp Lersch
 */
enum class Role {
    SUPER_ADMIN,
    ADMIN,
    GUEST,
    USER,
    MANAGER,
    KNOWLEDGE_CREATOR,
    DESIGN_CREATOR;

    override fun toString(): String {
        return name
    }

    fun equals(role: Role): Boolean {
        return this == role
    }

    companion object {
        /** Returns a role if it can be matched.
         * @param role A String which should be equal the name of a role. Accepts corrects upper, lower case & trailing, leading spaces
         * @return returns the matched role, if unsuccessful null
         */
        fun getRoleByString(role: String?): Role? {
            return if (role == null) null else when (role.uppercase(Locale.getDefault()).strip()) {
                "SUPERADMIN", "SUPER ADMIN", "SUPER-ADMIN", "SUPER_ADMIN" -> SUPER_ADMIN
                "ADMIN" -> ADMIN
                "USER" -> USER
                "GUEST" -> GUEST
                "MANAGER" -> MANAGER
                "KNOWLEDGECREATOR", "KNOWLEDGE CREATOR", "KNOWLEDGE-CREATOR", "KNOWLEDGE_CREATOR" -> KNOWLEDGE_CREATOR
                "DESIGNCREATOR", "DESIGN CREATOR", "DESIGN-CREATOR", "DESIGN_CREATOR" -> DESIGN_CREATOR
                else -> null
            }
        }

        /** Returns a role if it can be matched.
         * @param role role A String which should be equal the name of a role. Accepts corrects upper, lower case & trailing, leading spaces
         * @return the matched role, if unsuccessful GUEST
         */
        fun getRoleByStringWithUnknownAsGuest(role: String?): Role {
            val tmp = getRoleByString(role)
            return tmp ?: GUEST
        }
    }
}
