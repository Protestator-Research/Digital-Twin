import Elements.*
import com.github.tukcps.jaadd.DDBuilder
import com.github.tukcps.jaadd.values.IntegerRange
import com.github.tukcps.sysmd.entities.*
import com.github.tukcps.sysmd.services.*
import java.util.*

class DigitalTwinSession(
    id: UUID = UUID.randomUUID(),
    status: SessionStatus = SessionStatus(),
    settings: SessionSettings = SessionSettings(),
    builder: DDBuilder = DDBuilder()
) : AgilaSessionImpl(id, status, settings, builder) {
    operator fun set(elementId: UUID, value: Element) {
        repo.elements.set(elementId,value)
    }

    fun remodelRepresentation(){
        for (element in getUnownedElements())
        {
            if((element.first !is Association)&&(element.second is Specialization)) {
                if ((element.second as Specialization).target[0].toString() == "Component?") {
                    componentsMap[(element.second as Specialization).source[0].toString()] = SysMDComponent()
                } else {
                    val qualifiedName = (element.second as Specialization).target[0].toString().removeSuffix("?")
                    val completeName = element.first.toString().removeSuffix("?").split("::").first()
                    val partName = element.first.toString().removeSuffix("?").split("::").last()
                    val type = getTypeOfElement(element.first.toString().removeSuffix("?"))
                    val number = getNumberOfElements(element.first.toString().removeSuffix("?"))
                    if (type == null)
                        if(number>1) {
                            for(i in 1..number){
                                if (componentsMap[qualifiedName] != null) {
                                    componentsMap[completeName]?.consistsOfComponents?.set(
                                        "$partName$i",
                                        componentsMap[qualifiedName]!!
                                    )
                                }
                            }
                        }
                        else {
                            if (componentsMap[qualifiedName] != null) {
                                componentsMap[completeName]?.consistsOfComponents?.set(
                                    partName,
                                    componentsMap[qualifiedName]!!
                                )
                            }
                        }
                }
            }
            if(element.second is ValueFeature) {
                val qualifiedName = (element.second as ValueFeature).qualifiedName
                val completeName = element.first.toString().removeSuffix("?") + "::" + qualifiedName
                val type = getTypeOfElement(completeName)
                val number = getNumberOfElements(completeName)
                if(type!=null) {
                    if (number > 1) {
                        for(i in 1..number){
                            componentsMap[element.first.toString().removeSuffix("?")]?.addProperty("$qualifiedName$i", type)
                            componentsMap[element.first.toString().removeSuffix("?")]?.getProperty("$qualifiedName$i")?.id =
                                element.second.elementId

                            if ((element.second as ValueFeature).isMeasurable)
                                componentsMap[element.first.toString().removeSuffix("?")]?.setMeasurable("$qualifiedName$i")
                            if ((element.second as ValueFeature).isInput)
                                componentsMap[element.first.toString().removeSuffix("?")]?.setInput("$qualifiedName$i")
                            if ((element.second as ValueFeature).isOutput)
                                componentsMap[element.first.toString().removeSuffix("?")]?.setOutput("$qualifiedName$i")
                        }
                    } else {
                        componentsMap[element.first.toString().removeSuffix("?")]?.addProperty(qualifiedName, type)
                        componentsMap[element.first.toString().removeSuffix("?")]?.getProperty(qualifiedName)?.id =
                            element.second.elementId

                        if ((element.second as ValueFeature).isMeasurable)
                            componentsMap[element.first.toString().removeSuffix("?")]?.setMeasurable(qualifiedName)
                        if ((element.second as ValueFeature).isInput)
                            componentsMap[element.first.toString().removeSuffix("?")]?.setInput(qualifiedName)
                        if ((element.second as ValueFeature).isOutput)
                            componentsMap[element.first.toString().removeSuffix("?")]?.setOutput(qualifiedName)
                    }
                }
            }

            if(element.second is Multiplicity) {

            }

            println(element.toString())
        }

        var i=0
        while(i<getUnownedElements().size) {
            var isAddionalSystemKnowledge = true;
            val element = getUnownedElements().elementAt(i)
            for(alreadyAnalyzed in componentsMap.keys) {
                if (element.first.toString().contains(alreadyAnalyzed)) {
                    isAddionalSystemKnowledge = false
                    break
                }
                if(element.second is Class){
                    if ((element.second as Class).name?.contains(alreadyAnalyzed) == true) {
                        isAddionalSystemKnowledge = false
                        break
                    }
                }
            }
            if(isAddionalSystemKnowledge) {
                println(element.toString())
                println("${element.first.toString()} will be analyzed")

                if(element.second is Import) {
                    SystemElements[element.first.toString().removeSuffix("?")] = SysMDTwin()
                }

                if(element.second is ValueFeature) {

                    println((element.second as ValueFeature).qualifiedName)

                    val qualifiedName = (element.second as ValueFeature).qualifiedName
                    val completeName = element.first.toString().removeSuffix("?") + "::" + qualifiedName
                    val type = getTypeOfElement(completeName)
                    val number = getNumberOfElements(completeName)
                    if(type!=null) {
                        if (number > 1) {
                            for (i in 1..number) {
                                SystemElements[element.first.toString().removeSuffix("?")]?.addProperty("$qualifiedName$i", type)
                                SystemElements[element.first.toString().removeSuffix("?")]?.getProperty("$qualifiedName$i")?.id = element.second.elementId
                                if ((element.second as ValueFeature).isMeasurable)
                                    SystemElements[element.first.toString().removeSuffix("?")]?.setMeasurable("$qualifiedName$i")
                                if ((element.second as ValueFeature).isInput)
                                    SystemElements[element.first.toString().removeSuffix("?")]?.setInput("$qualifiedName$i")
                                if ((element.second as ValueFeature).isOutput)
                                    SystemElements[element.first.toString().removeSuffix("?")]?.setOutput("$qualifiedName$i")
                            }
                        } else {
                            SystemElements[element.first.toString().removeSuffix("?")]?.addProperty(qualifiedName, type)
                            SystemElements[element.first.toString().removeSuffix("?")]?.getProperty(qualifiedName)?.id =
                                element.second.elementId
                            if ((element.second as ValueFeature).isMeasurable)
                                SystemElements[element.first.toString().removeSuffix("?")]?.setMeasurable(qualifiedName)
                            if ((element.second as ValueFeature).isInput)
                                SystemElements[element.first.toString().removeSuffix("?")]?.setInput(qualifiedName)
                            if ((element.second as ValueFeature).isOutput)
                                SystemElements[element.first.toString().removeSuffix("?")]?.setOutput(qualifiedName)
                        }
                    }
                }
                if((element.first !is Association)&&(element.second is Specialization)) {
                    if ((element.second as Specialization).target[0].toString() == "System?") {
                        SystemElements[element.first.toString().removeSuffix("?")] = SysMDTwin()
                    } else {
                        println(
                            "element ${(element.second as Specialization).source.first()} is ${
                                (element.second as Specialization).target.first().toString().removeSuffix("?")
                            }"
                        )
                        if ((element.second as Specialization).target.first().toString()
                                .removeSuffix("?") == "connectTo"
                        ) {
                            println("${element.first.javaClass}")
                            println("Connection Source ${(element.first.ref as Association).source}")
                            println("Connection Target ${(element.first.ref as Association).target}")
                        } else if ((element.second as Specialization).target.first().toString()
                                .removeSuffix("?") == "hasValue"
                        ) {
                            println("Has Value Source ${(element.first.ref as Association).source}")
                            println("Has Value Target ${(element.first.ref as Association).target}")
                            val fullName = (element.first.ref as Association).source.toString().removeSuffix("?]")
                                .removePrefix("[")
                            val property = getPropertyFromAddress(fullName)
                            getConstantOfValue((element.first.ref as Association).target.toString(), property)
                        } else if ((getTypeOfElement(
                                element.first.toString().removeSuffix("?")
                            ) == null) && (SystemElements[element.first.toString().split("::")
                                .first()]?.consistsOfComponents?.containsKey(
                                (element.second as Specialization).source.first().toString()
                            ) == false)
                        ) {
                            val number = getNumberOfElements(element.first.toString().removeSuffix("?"))
                            if (number > 1) {
                                for (i in 1..number) {
                                    SystemElements[element.first.toString().split("::")
                                        .first()]?.consistsOfComponents?.set(
                                        (element.second as Specialization).source.first().toString() + i,
                                        componentsMap[(element.second as Specialization).target.first().toString()
                                            .removeSuffix("?")]!!.copyOfElement(null)
                                    )
                                }
                            }
                            else
                            {
                                SystemElements[element.first.toString().split("::")
                                    .first()]?.consistsOfComponents?.set(
                                    (element.second as Specialization).source.first().toString(),
                                    componentsMap[(element.second as Specialization).target.first().toString()
                                        .removeSuffix("?")]!!.copyOfElement(null)
                                )
                            }
                        }
                    }
                }
            }
            i++
        }
        println("Remodel")
    }


    private fun getTypeOfElement(name:String): SysMDType? {
        for(element in getUnownedElements()){
            if((element.first.toString()==(name+"?"))&&(element.second is Specialization))
            {
                return when((element.second as Specialization).target[0].toString().removeSuffix("?")) {
                    "Voltage" -> SysMDType.VOLTAGE
                    "Capacity" -> SysMDType.CAPACITIY
                    "Resistance" -> SysMDType.RESISTANCE
                    "Current" -> SysMDType.CURRENT

                    else -> null
                }
            }
        }
        return null
    }

    private fun getNumberOfElements(name:String): Long {
        for(element in getUnownedElements()){
            if((element.first.toString()==(name+"?"))&&(element.second is Multiplicity))
            {
                return ((element.second as Multiplicity).valueSpecs.first() as IntegerRange).max
            }
        }
        return 1
    }


    private fun getConstantOfValue(name:String,property:SysMDProperty<*>){
        if (name.contains("Resistance22k"))
            (property as SysMDProperty<Double>).currentValue=22000.0
        else if(name.contains("Resistance4k7"))
            (property as SysMDProperty<Double>).currentValue=4700.0
        else if(name.contains("Resistance10k"))
            (property as SysMDProperty<Double>).currentValue=4700.0
        else if(name.contains("ZeroResistance"))
            (property as SysMDProperty<Double>).currentValue=0.0000000000000000001
        else if(name.contains("umFarat"))
            (property as SysMDProperty<Double>).currentValue=0.00001
    }

    private fun getPropertyFromAddress(address:String):SysMDProperty<*> {
        return getPropertyFromAddress(address.split("::"),0)
    }

    private fun getPropertyFromAddress(addressElements:List<String>,index:Int, elem:SysMDElement?=null) : SysMDProperty<*>{
        if((elem == null)&&index>0)
            throw Exception("Something Bad Happended")

        when (addressElements.size-index) {
            addressElements.size -> return getPropertyFromAddress(addressElements,index+1,SystemElements[addressElements[index]])
            1 -> return elem?.getProperty(addressElements[index]) ?: SysMDProperty<Double>(currentValue = -123456789.123456789, type=SysMDType.ERROR)
            else -> return getPropertyFromAddress(addressElements,index+1, elem?.consistsOfComponents!![addressElements[index]])
        }
    }


    val componentsMap = hashMapOf<String,SysMDElement>()
    val globalProperties = hashMapOf<String, SysMDProperty<*>>()
    val SystemElements = hashMapOf<String, SysMDElement>()
}