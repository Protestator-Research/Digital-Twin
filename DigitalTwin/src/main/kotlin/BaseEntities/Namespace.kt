package BaseEntities

import BaseEntities.implementation.NamespaceImportImplementation
import ImportedSysMDServices.AgilaSession
import Parser.QualifiedName


/**
 * KerML foresees a namespace that is an element.
 * "Namespaces can assign unique names to Namespace members, but support multiple aliases per Element.
 * They also support Import of Elements from other Namespaces, enabling an Element to have
 * a different name when imported."
 */
interface Namespace: Element {

    val imports: List<Identity<Namespace>>
        get() {
            val result: MutableList<Identity<Namespace>> = mutableListOf()
            ownedElements.forEach {
                if (it.ref is Import) {
                    result.add( (it.ref as Import).importedNamespace )
                }
            }
            return result
        }

    fun importNamespace(model: AgilaSession, importedNamespace: QualifiedName, importedNamespaceRef: Namespace?=null) {
        val import = NamespaceImportImplementation(importedNamespace = Identity(id=null, str=importedNamespace, ref=importedNamespaceRef), owner=Identity(this))
        model.create(import, this)
    }
}