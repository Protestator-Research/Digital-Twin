package Elements

import java.util.UUID
import kotlin.reflect.typeOf

open class SysMDProperty<T>(
    var currentValue:T,
    var type:SysMDType
) {
    var measurable : Boolean = false
    var id:UUID?=null

    fun <V:T>setNewValue(newValue:V){
        currentValue = newValue
    }
}

class SysMDPropertyNumber(currentValue: Number, type: SysMDType):SysMDProperty<Number>(currentValue, type )

class SysMDPropertyInteger(currentValue: Int, type: SysMDType):SysMDProperty<Int>(currentValue, type)

class SysMDPropertyString(currentValue: String, type: SysMDType):SysMDProperty<String>(currentValue, type)

class SysMDPropertyDouble(currentValue: Double, type: SysMDType):SysMDProperty<Double>(currentValue, type)

class SysMDPropertyBoolean(currentValue: Boolean, type: SysMDType):SysMDProperty<Boolean>(currentValue, type)
