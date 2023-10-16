package Elements

import java.time.LocalDateTime
import java.time.format.DateTimeFormatter
import java.util.UUID
import kotlin.reflect.typeOf

abstract class SysMDProperty<T>(
    var currentValue:T,
    var type:SysMDType
) {
    val formatter = DateTimeFormatter.ofPattern("dd-MM-yyyy HH-mm-ss.SSS")
    var measurable : Boolean = false
    var controllable : Boolean = false
    var id:UUID?=UUID.randomUUID()
    var topic:String=""

    fun <V:T>setNewValue(newValue:V){
        currentValue = newValue
        if(topic != ""){
            val payload = "{\r\n\"current_time\": \"${LocalDateTime.now().format(formatter)}\",\r\n\"value\": $currentValue\r\n}"
            MQTT.Broker.publishToTopic(topic, payload)
        }
    }

    abstract fun copyOfProperty() : SysMDProperty<T>
}

class SysMDPropertyNumber(currentValue: Number, type: SysMDType):SysMDProperty<Number>(currentValue, type ) {
    override fun copyOfProperty(): SysMDProperty<Number> {
        val returnValue = SysMDPropertyNumber(currentValue, type)
        returnValue.measurable = measurable
        returnValue.controllable = controllable
        return returnValue
    }
}

class SysMDPropertyInteger(currentValue: Int, type: SysMDType):SysMDProperty<Int>(currentValue, type) {
    override fun copyOfProperty(): SysMDProperty<Int> {
        val returnValue = SysMDPropertyInteger(currentValue, type)
        returnValue.measurable = measurable
        returnValue.controllable = controllable
        return returnValue
    }

}

class SysMDPropertyString(currentValue: String, type: SysMDType):SysMDProperty<String>(currentValue, type) {
    override fun copyOfProperty(): SysMDProperty<String> {
        val returnValue = SysMDPropertyString(currentValue, type)
        returnValue.measurable = measurable
        returnValue.controllable = controllable
        return returnValue
    }
}

class SysMDPropertyDouble(currentValue: Double, type: SysMDType):SysMDProperty<Double>(currentValue, type) {
    override fun copyOfProperty(): SysMDProperty<Double> {
        val returnValue = SysMDPropertyDouble(currentValue, type)
        returnValue.measurable = measurable
        returnValue.controllable = controllable
        return returnValue
    }
}

class SysMDPropertyBoolean(currentValue: Boolean, type: SysMDType):SysMDProperty<Boolean>(currentValue, type) {
    override fun copyOfProperty(): SysMDProperty<Boolean> {
        val returnValue = SysMDPropertyBoolean(currentValue, type)
        returnValue.measurable = measurable
        returnValue.controllable = controllable
        return returnValue
    }
}
