package Elements

import java.util.UUID

abstract class SysMDElement {
    fun addProperty(name:String, type:SysMDType?) {
        properties[name] = when (type) {
            SysMDType.ANY -> TODO()
            SysMDType.NUMBER -> SysMDProperty<Double>(currentValue = 0.0,type = type)
            SysMDType.INTEGER -> SysMDProperty<Int>(currentValue = 0,type = type)
            SysMDType.BOOLEAN -> SysMDProperty<Boolean>(currentValue = false,type = type)
            SysMDType.REAL -> SysMDProperty<Double>(currentValue = 0.0,type = type)
            SysMDType.QUANTITY -> TODO()
            SysMDType.SCALAR_VALUE -> TODO()
            SysMDType.STRING -> SysMDProperty<String>(currentValue = "",type = type)
            SysMDType.REQUIREMENT -> TODO()
            SysMDType.PERFORMANCE -> SysMDProperty<Double>(currentValue = 0.0,type = type)
            SysMDType.QUALITY -> TODO()
            SysMDType.WORK -> SysMDProperty<Double>(currentValue = 0.0,type = type)
            SysMDType.MASS -> SysMDProperty<Double>(currentValue = 0.0,type = type)
            SysMDType.TIME -> SysMDProperty<Double>(currentValue = 0.0,type = type)
            SysMDType.POWER -> SysMDProperty<Double>(currentValue = 0.0,type = type)
            SysMDType.LENGTH -> SysMDProperty<Double>(currentValue = 0.0,type = type)
            SysMDType.SPEED -> SysMDProperty<Double>(currentValue = 0.0,type = type)
            SysMDType.ACCELLERATION -> SysMDProperty<Double>(currentValue = 0.0,type = type)
            SysMDType.AREA -> SysMDProperty<Double>(currentValue = 0.0,type = type)
            SysMDType.VOLUME -> SysMDProperty<Double>(currentValue = 0.0,type = type)
            SysMDType.CURRENT -> SysMDProperty<Double>(currentValue = 0.0,type = type)
            SysMDType.CURRENCY -> SysMDProperty<Double>(currentValue = 0.0,type = type)
            SysMDType.TEMPRETURE -> SysMDProperty<Double>(currentValue = 0.0,type = type)
            SysMDType.VOLTAGE -> SysMDProperty<Double>(currentValue = 0.0,type = type)
            SysMDType.CAPACITIY -> SysMDProperty<Double>(currentValue = 0.0,type = type)
            SysMDType.RESISTANCE -> SysMDProperty<Double>(currentValue = 0.0,type = type)
            SysMDType.ELEMENT -> TODO()
            SysMDType.FUNCTION -> TODO()
            SysMDType.COMPONENT -> TODO()
            SysMDType.SYSTEM -> TODO()
            SysMDType.PART -> TODO()
            SysMDType.SOFTWARE -> TODO()
            SysMDType.PROCESSOR -> TODO()
            SysMDType.DISPLAY -> TODO()
            SysMDType.COMMENT -> SysMDProperty<String>(currentValue = "",type = type)
            SysMDType.shows -> TODO()
            SysMDType.implmenents -> TODO()
            SysMDType.implementedBy -> TODO()
            SysMDType.executes -> TODO()
            SysMDType.executedby -> TODO()
            SysMDType.ERROR -> SysMDProperty<Double>(currentValue = 1234567889.1235456789, type=type)
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


    val connectionType:ConnectionType = ConnectionType.UNDEFINED
    var properties:HashMap<String,SysMDProperty<*>> = hashMapOf()
    var inputs:HashMap<String,SysMDProperty<*>> = hashMapOf()
    var outputs:HashMap<String,SysMDProperty<*>> = hashMapOf()
    var consistsOfComponents:HashMap<String,SysMDElement> = hashMapOf()
    var id:UUID? = null
}