package Elements

import java.util.UUID

class SysMDProperty<T>(
    var currentValue:T,
    var type:SysMDType
) {
    var measurable : Boolean = false
    var id:UUID?=null

}