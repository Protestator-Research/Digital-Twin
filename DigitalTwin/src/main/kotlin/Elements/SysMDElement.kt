package Elements

interface SysMDElement {
    fun addProperty(name:String, type:SysMDType?)


    fun evaluateConnection()


    val connectionType:ConnectionType
    val properties:HashMap<String,SysMDType?>
    val consistsOfComponents:HashMap<String,SysMDElement?>
}