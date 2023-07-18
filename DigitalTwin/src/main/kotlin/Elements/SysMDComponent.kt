package Elements

import java.util.UUID


class SysMDComponent : SysMDElement() {
    override fun copyOfElement(id:UUID?): SysMDElement {
        val returnValue = SysMDComponent()

        returnValue.consistsOfComponents = this.consistsOfComponents.clone() as HashMap<String, SysMDElement>
        returnValue.inputs = this.inputs.clone() as HashMap<String, SysMDProperty<*>>
        returnValue.outputs = this.outputs.clone() as HashMap<String, SysMDProperty<*>>
        returnValue.properties = this.properties.clone() as HashMap<String, SysMDProperty<*>>

        returnValue.id = id

        return returnValue
    }

    override fun evaluateConnection() {

    }
}