package BaseEntities

/**
 * An Import that imports a specific member of an imported namesapce.
 */
interface MembershipImport: Import {
    var importedMemberName: Identity<Element>
    override var importedNamespace: Identity<Namespace>
}