package Elements

/**
 * Represents the definition of the twin. Has all the Components and their connection.
 */
class SysMDTwin : SysMDElement{
    override fun addProperty(name: String, type: SysMDType?) {
        TODO("Not yet implemented")
    }

    override fun evaluateConnection() {

    }

    override val connectionType: ConnectionType
        get() = TODO("Not yet implemented")
    override val properties: HashMap<String, SysMDType?>
        get() = TODO("Not yet implemented")
    override val consistsOfComponents: HashMap<String, SysMDElement?>
        get() = TODO("Not yet implemented")

}
