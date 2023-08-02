package BaseEntities


interface Dependency : Relationship {
    var clientDependency: MutableList<Identity<Element>>
    var supplierDependency: MutableList<Identity<Element>>
}