package Elements

abstract class SysMDElement {
    fun addProperty(name:String, type:SysMDType?) {
        properties[name] = when (type) {
            SysMDType.ANY -> TODO()
            SysMDType.NUMBER -> SysMDPropertie<Double>(currentValue = 0.0,type = type)
            SysMDType.INTEGER -> SysMDPropertie<Int>(currentValue = 0,type = type)
            SysMDType.BOOLEAN -> SysMDPropertie<Boolean>(currentValue = false,type = type)
            SysMDType.REAL -> SysMDPropertie<Double>(currentValue = 0.0,type = type)
            SysMDType.QUANTITY -> TODO()
            SysMDType.SCALAR_VALUE -> TODO()
            SysMDType.STRING -> SysMDPropertie<String>(currentValue = "",type = type)
            SysMDType.REQUIREMENT -> TODO()
            SysMDType.PERFORMANCE -> SysMDPropertie<Double>(currentValue = 0.0,type = type)
            SysMDType.QUALITY -> TODO()
            SysMDType.WORK -> SysMDPropertie<Double>(currentValue = 0.0,type = type)
            SysMDType.MASS -> SysMDPropertie<Double>(currentValue = 0.0,type = type)
            SysMDType.TIME -> SysMDPropertie<Double>(currentValue = 0.0,type = type)
            SysMDType.POWER -> SysMDPropertie<Double>(currentValue = 0.0,type = type)
            SysMDType.LENGTH -> SysMDPropertie<Double>(currentValue = 0.0,type = type)
            SysMDType.SPEED -> SysMDPropertie<Double>(currentValue = 0.0,type = type)
            SysMDType.ACCELLERATION -> SysMDPropertie<Double>(currentValue = 0.0,type = type)
            SysMDType.AREA -> SysMDPropertie<Double>(currentValue = 0.0,type = type)
            SysMDType.VOLUME -> SysMDPropertie<Double>(currentValue = 0.0,type = type)
            SysMDType.CURRENT -> SysMDPropertie<Double>(currentValue = 0.0,type = type)
            SysMDType.CURRENCY -> SysMDPropertie<Double>(currentValue = 0.0,type = type)
            SysMDType.TEMPRETURE -> SysMDPropertie<Double>(currentValue = 0.0,type = type)
            SysMDType.VOLTAGE -> SysMDPropertie<Double>(currentValue = 0.0,type = type)
            SysMDType.CAPACITIY -> SysMDPropertie<Double>(currentValue = 0.0,type = type)
            SysMDType.RESISTANCE -> SysMDPropertie<Double>(currentValue = 0.0,type = type)
            SysMDType.ELEMENT -> TODO()
            SysMDType.FUNCTION -> TODO()
            SysMDType.COMPONENT -> TODO()
            SysMDType.SYSTEM -> TODO()
            SysMDType.PART -> TODO()
            SysMDType.SOFTWARE -> TODO()
            SysMDType.PROCESSOR -> TODO()
            SysMDType.DISPLAY -> TODO()
            SysMDType.COMMENT -> SysMDPropertie<String>(currentValue = "",type = type)
            SysMDType.shows -> TODO()
            SysMDType.implmenents -> TODO()
            SysMDType.implementedBy -> TODO()
            SysMDType.executes -> TODO()
            SysMDType.executedby -> TODO()
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



    abstract fun evaluateConnection()


    val connectionType:ConnectionType = ConnectionType.UNDEFINED
    protected val properties:HashMap<String,SysMDPropertie<*>> = hashMapOf()
    val inputs:HashMap<String,SysMDPropertie<*>> = hashMapOf()
    val outputs:HashMap<String,SysMDPropertie<*>> = hashMapOf()
    val consistsOfComponents:HashMap<String,SysMDElement> = hashMapOf()
}