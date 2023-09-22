package Elements

import MQTT.Broker
import java.util.UUID

abstract class SysMDElement {
    fun addProperty(name:String, type:SysMDType?) {
        properties[name] = when (type) {
            SysMDType.ANY -> TODO()
            SysMDType.NUMBER -> SysMDPropertyNumber(currentValue = 0.0,type = type)
            SysMDType.INTEGER -> SysMDPropertyInteger(currentValue = 0,type = type)
            SysMDType.BOOLEAN -> SysMDPropertyBoolean(currentValue = false,type = type)
            SysMDType.REAL -> SysMDPropertyDouble(currentValue = 0.0,type = type)
            SysMDType.QUANTITY -> TODO()
            SysMDType.SCALAR_VALUE -> TODO()
            SysMDType.STRING -> SysMDPropertyString(currentValue = "",type = type)
            SysMDType.REQUIREMENT -> TODO()
            SysMDType.PERFORMANCE -> SysMDPropertyDouble(currentValue = 0.0,type = type)
            SysMDType.QUALITY -> TODO()
            SysMDType.WORK -> SysMDPropertyDouble(currentValue = 0.0,type = type)
            SysMDType.MASS -> SysMDPropertyDouble(currentValue = 0.0,type = type)
            SysMDType.TIME -> SysMDPropertyDouble(currentValue = 0.0,type = type)
            SysMDType.POWER -> SysMDPropertyDouble(currentValue = 0.0,type = type)
            SysMDType.LENGTH -> SysMDPropertyDouble(currentValue = 0.0,type = type)
            SysMDType.SPEED -> SysMDPropertyDouble(currentValue = 0.0,type = type)
            SysMDType.ACCELLERATION -> SysMDPropertyDouble(currentValue = 0.0,type = type)
            SysMDType.AREA -> SysMDPropertyDouble(currentValue = 0.0,type = type)
            SysMDType.VOLUME -> SysMDPropertyDouble(currentValue = 0.0,type = type)
            SysMDType.CURRENT -> SysMDPropertyDouble(currentValue = 0.0,type = type)
            SysMDType.CURRENCY -> SysMDPropertyDouble(currentValue = 0.0,type = type)
            SysMDType.TEMPRETURE -> SysMDPropertyDouble(currentValue = 0.0,type = type)
            SysMDType.VOLTAGE -> SysMDPropertyDouble(currentValue = 0.0,type = type)
            SysMDType.CAPACITIY -> SysMDPropertyDouble(currentValue = 0.0,type = type)
            SysMDType.RESISTANCE -> SysMDPropertyDouble(currentValue = 0.0,type = type)
            SysMDType.ELEMENT -> TODO()
            SysMDType.FUNCTION -> TODO()
            SysMDType.COMPONENT -> TODO()
            SysMDType.SYSTEM -> TODO()
            SysMDType.PART -> TODO()
            SysMDType.SOFTWARE -> TODO()
            SysMDType.PROCESSOR -> TODO()
            SysMDType.DISPLAY -> TODO()
            SysMDType.COMMENT -> SysMDPropertyString(currentValue = "",type = type)
            SysMDType.shows -> TODO()
            SysMDType.implmenents -> TODO()
            SysMDType.implementedBy -> TODO()
            SysMDType.executes -> TODO()
            SysMDType.executedby -> TODO()
            SysMDType.ERROR -> SysMDPropertyDouble(currentValue = 1234567889.1235456789, type=type)
            null -> throw Exception("Expected a Type --> Not Valid")
        }
    }

    fun setMeasurable(name:String) {
        if(properties.containsKey(name))
        {
            properties[name]?.measurable=true
        }
    }

    fun setInput(name:String) {
        if(properties.containsKey(name)) {
            inputs[name] = properties[name] !!
        }
    }
    fun setOutput(name:String) {
        if(properties.containsKey(name)) {
            outputs[name] = properties[name] !!
        }
    }

    fun getProperty(name:String):SysMDProperty<*> {
        return  properties[name] !!
    }

    abstract fun copyOfElement(id:UUID?):SysMDElement

    abstract fun evaluateConnection()

    fun addRecursiveComponentsToBroker(currentPath:String)
    {
        for(prop_keys in properties) {
            if((prop_keys.value.measurable)||(prop_keys.value.controllable)) {
                Broker.pushTopic("$currentPath/${prop_keys.key}")
//                if (properties[prop_keys.key]?.topic != "") {
                    println("Current Toptic: ${properties[prop_keys.key]?.topic} Set To Topic $currentPath/${prop_keys.key}")
                    properties[prop_keys.key]?.topic = "$currentPath/${prop_keys.key}"
            }
        }


        for (component in consistsOfComponents) {
            var newTopicName = "$currentPath/${component.key}"
            component.value.addRecursiveComponentsToBroker(newTopicName)
        }
    }

    fun getPropertyRecursive(values:List<String>, index:Int) : SysMDProperty<*>? {
        if(properties.containsKey(values[index])) {
            return properties[values[index]]
        } else if(consistsOfComponents.containsKey(values[index])) {
            return consistsOfComponents[values[index]]?.getPropertyRecursive(values, index+1)
        }
        return null
    }

    val connectionType:ConnectionType = ConnectionType.UNDEFINED
    var properties:HashMap<String,SysMDProperty<*>> = hashMapOf()
    var inputs:HashMap<String,SysMDProperty<*>> = hashMapOf()
    var outputs:HashMap<String,SysMDProperty<*>> = hashMapOf()
    var consistsOfComponents:HashMap<String,SysMDElement> = hashMapOf()
    var id:UUID? = null
}