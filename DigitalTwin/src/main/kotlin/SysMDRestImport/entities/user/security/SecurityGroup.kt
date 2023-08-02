package SysMDRestImport.entities.user.security

/** @author Philipp Lersch
 * Used to determain contexts for authorization
 * This can be a person, a group or an more abstract party
 */
interface SecurityGroup {
    val groupType: String?
        /** Gets the groups type
         * @return 's the type as String. Often it can be getClass().getSimpleName()
         */
        get() = this.javaClass.simpleName

    /** A unique identifier for all possible SecurityGroups
     * @return 's am String with only the 'return getGroupType()' or it beeing appended with 'return ..+"_"+getGroupType()'.
     * Also if present an id as prefix 'getID()+"_"+..'.
     * The middle-part is for extra info. To prevent spaces wrapping in (..).replace(' ','-') could be useful
     */
    val securityIdentifier: String?

    /** Returns the parent security group.
     * The root group is defined as SecurityConstants.rootGroup
     * @return The parent security group of this security group.
     */
    val parentGroup: SecurityGroup?
}
