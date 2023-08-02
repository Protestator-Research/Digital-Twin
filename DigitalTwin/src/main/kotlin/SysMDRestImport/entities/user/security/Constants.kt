package SysMDRestImport.entities.user.security

object Constants {
    /** A context group meant only for root/super admins  */
    @JvmField
    val ROOT_GROUP: SecurityGroup = object : SecurityGroup {
        override val groupType: String
            get() = "Root-Group"

        override val securityIdentifier: String
            get() = groupType
        override val parentGroup: SecurityGroup?
            get() = null

        override fun equals(other: Any?): Boolean {
            if (this === other) return true
            if (other == null || javaClass != other.javaClass) return false
            val group = other as SecurityGroup
            return securityIdentifier == group.securityIdentifier
        }
    }

    /** A group meant for public access. Can be used with Role. GUEST as seen in publicContext  */
    @JvmField
    val PUBLIC_GROUP: SecurityGroup = object : SecurityGroup {
        override val groupType: String
            get() = "Public-Group"
        override val securityIdentifier: String
            get() = groupType
        override val parentGroup: SecurityGroup
            get() = ROOT_GROUP

        override fun equals(other: Any?): Boolean {
            if (this === other) return true
            if (other == null || javaClass != other.javaClass) return false
            val group = other as SecurityGroup
            return securityIdentifier == group.securityIdentifier
        }
    }

    /** Represents the highest possible securityContext: the root/super Admin context. Also used to deny access on buggy user authorization  */
    val ROOT_CONTEXT = UserSecurityContext(Role.SUPER_ADMIN, ROOT_GROUP)

    /** Represents a more public securityContext, which each user has upon creation. Can be removed/added like any other context  */
    @JvmField
    val PUBLIC_CONTEXT = UserSecurityContext(Role.GUEST, PUBLIC_GROUP)
}
