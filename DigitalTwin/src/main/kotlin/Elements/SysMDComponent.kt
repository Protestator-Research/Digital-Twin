package Elements



class SysMDComponent : SysMDElement {
    override fun addProperty(name: String, type: SysMDType?) {
        properties[name]=type
    }

    override fun evaluateConnection() {

    }

    override val connectionType: ConnectionType
        get() = TODO("Not yet implemented")
    override val properties:HashMap<String,SysMDType?> = hashMapOf()
    override val consistsOfComponents: HashMap<String, SysMDElement?> = hashMapOf()
}