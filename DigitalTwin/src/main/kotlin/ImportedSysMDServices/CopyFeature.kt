package ImportedSysMDServices

import BaseEntities.*

/**
 * For inheritance: This function creates a deep copy of a feature.
 */
fun Feature.deepCloneWithInheritedFeature(addTo: Namespace): Feature {
    val klon = clone()  // Clone of this feature, but with new ID.
    klon.isTransient = true
    klon.ownedElements = mutableListOf()
    klon.owner = Identity(null, null, null)
    val createdKlon = model!!.createTransient(klon, addTo)
    if (klon == createdKlon) {
        ownedElements.forEach {
            when (it.ref) {
                is Multiplicity -> model?.createTransient(it.ref!!.clone() as Multiplicity, createdKlon)
                is Feature -> model?.createTransient((it.ref as Feature).deepCloneWithInheritedFeature(createdKlon), createdKlon)
                is Specialization -> model?.createTransient((it.ref!!.clone() as Specialization).clone(), createdKlon)
            }
        }
    }
    return createdKlon
}