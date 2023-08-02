package BaseEntities.implementation

import BaseEntities.*
import ImportedSysMDExceptions.SemanticError
import ImportedSysMDExceptions.SysMDError
import ImportedSysMDServices.report

import java.util.*


/**
 * An Import is a Relationship between an importOwningNamespace in which one or more of the visible
 * Memberships of the importedNamespace become importedMemberships of the importOwningNamespace.
 * @param isImportAll If isImportAll = false (the default), then only public Memberships are considered "visible". If isImportAll =
 * true, then all Memberships are considered "visible", regardless of their declared visibility.
 * If no importedMemberName is given, then all visible Memberships are imported from the importedNamespace.
 * If isRecursive = true, then visible Memberships are also recursively imported from all visible ownedMembers
 * of the Namespace that are also Namespaces.
 * @param importedMemberName If an importedMemberName is given, then the Membership whose effectiveMemberName is that name is
 * imported from the importedNamespace, if it is visible.
 * @param isRecursive If isRecursive = true and the imported
 * memberElement is a Namespace, then visible Memberships are also recursively imported from that Namespace and
 * its owned sub-Namespaces.
 */
class MembershipImportImplementation(
    id: UUID = UUID.randomUUID(),
    owner: Identity<Element> = Identity(),
    importedNamespace: Identity<Namespace> = Identity(),
    override var importedMemberName: Identity<Element> = Identity(),
    override var visibility: Import.VisibilityKind = Import.VisibilityKind.Public,
    override var isRecursive: Boolean = true,                            // False by default in SysMLv2
    override var isImportAll: Boolean = false
): MembershipImport, RelationshipImplementation(
    elementId = id, ownedElements = mutableListOf(), owner = owner,
    source = mutableListOf(owner),
    target = mutableListOf(Identity(importedNamespace))
) {

    override val importOwningNamespace: Namespace?
        get() = owningNamespace

    @Suppress("UNCHECKED_CAST")
    override var importedNamespace: Identity<Namespace>
        get() = try { target[0] as Identity<Namespace> } catch (e: Exception) { model?.report(SysMDError("Problem with import"))
            Identity(model!!.global) }
        set(value) { target[0].ref = value.ref; target[0].str = value.str; target[0].id = value.id }

    override fun toString(): String = "MembershipImport { $importedNamespace $importedMemberName}"

    override fun clone() = MembershipImportImplementation(
        owner = Identity(owner),
        importedNamespace = Identity(importedNamespace),
        visibility = visibility,
        isRecursive = isRecursive,
        isImportAll = isImportAll
    ).also {
        it.model = model
    }

    override fun resolveNames(): Boolean {
        updated = super.resolveNames() or updated
        if (source.size > 1)
            model?.report( SemanticError("imports can have only a single source"))
        if (importOwningNamespace !is Namespace)
            model?.report(SemanticError( "only Packages and Namespaces can import"))
        if (target.size < 1)
            model?.report(SemanticError( "import: nothing imported"))
        target.forEach {
            if(it.ref !is Namespace?)
                model?.report(SemanticError( "only Packages and Namespaces can be imported"))
        }

        updated = importedNamespace.resolveIdentity(owningNamespace!!)
        if (importedNamespace.ref != null) {
            target[0] = importedNamespace
        }
        return updated
    }

    override fun updateFrom(template: Element) {
        super.updateFrom(template)
        if (template is Import) {
            importedNamespace = template.importedNamespace
            visibility = template.visibility
            isImportAll = template.isImportAll
            isRecursive = template.isRecursive
        }
    }
}