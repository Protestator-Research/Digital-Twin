package SysMDRestImport.entities.user.security

import java.util.*

class UserSecurityContext {
    @JvmField
    val role: Role
    private val groups: Set<SecurityGroup>

    constructor(role: Role, group: SecurityGroup) {
        this.role = role
        groups = setOf(group)
    }

    constructor(role: Role, groups: Set<SecurityGroup>) {
        this.role = role
        this.groups = groups
    }

    fun getGroups(): Set<SecurityGroup> {
        return HashSet(groups)
    }

    override fun equals(o: Any?): Boolean {
        if (this === o) return true
        if (o == null || javaClass != o.javaClass) return false
        val that = o as UserSecurityContext
        return role === that.role && groups.stream().allMatch { group: SecurityGroup ->
            that.groups.stream()
                .anyMatch { tg: SecurityGroup -> group.securityIdentifier.equals(tg.securityIdentifier) }
        }
    }

    override fun hashCode(): Int {
        return Objects.hash(role, groups)
    }

    override fun toString(): String {
        return "role: " + groups.joinToString { it.securityIdentifier.toString() }
    }
}
