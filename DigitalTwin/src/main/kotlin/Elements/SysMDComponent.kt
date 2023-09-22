package Elements

import java.util.UUID


class SysMDComponent : SysMDElement() {
    override fun copyOfElement(id:UUID?): SysMDElement {
        val returnValue = SysMDComponent()

        for(elem in consistsOfComponents)
        {
            returnValue.consistsOfComponents[elem.key] = elem.value.copyOfElement(UUID.randomUUID())
        }

        for(elem in inputs){
            returnValue.inputs[elem.key]=elem.value.copyOfProperty()
        }

        for(elem in outputs){
            returnValue.outputs[elem.key]=elem.value.copyOfProperty()
        }

        for(elem in properties){
            returnValue.properties[elem.key]=elem.value.copyOfProperty()
        }

        returnValue.id = id

        return returnValue
    }

    override fun evaluateConnection() {

    }
}