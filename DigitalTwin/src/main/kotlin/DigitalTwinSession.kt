import Elements.*
import com.github.tukcps.jaadd.DDBuilder
import com.github.tukcps.sysmd.cspsolver.DiscreteSolver
import com.github.tukcps.sysmd.entities.*
import com.github.tukcps.sysmd.entities.implementation.PackageImplementation
import com.github.tukcps.sysmd.parser.QualifiedName
import com.github.tukcps.sysmd.rest.ElementDAO
import com.github.tukcps.sysmd.services.*
import java.util.*
import kotlin.collections.HashMap

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
            if(element.second is Specialization) {
                if ((element.second as Specialization).target[0].toString() == "Component?") {
                    componentsMap[(element.second as Specialization).source[0].toString()] = SysMDComponent()
                } else {
                    val qualifiedName = (element.second as Specialization).target[0].toString().removeSuffix("?")
                    val completeName = element.first.toString().removeSuffix("?").split("::").first()
                    val type = getTypeOfElement(qualifiedName)
                    if (type == null)

                        if (componentsMap[qualifiedName] != null) {
                            componentsMap[completeName]?.consistsOfComponents?.set(
                                qualifiedName,
                                componentsMap[qualifiedName] !!
                            )
                        }
                }
            }
            if(element.second is ValueFeature) {
//                println((element.second as ValueFeature).qualifiedName)
                val qualifiedName = (element.second as ValueFeature).qualifiedName
                val completeName = element.first.toString().removeSuffix("?") + "::" + qualifiedName
                val type = getTypeOfElement(completeName)
                if(type!=null) {
                    componentsMap[element.first.toString().removeSuffix("?")]?.addProperty(qualifiedName,type)
                    if((element.second as ValueFeature).isMeasurable)
                        componentsMap[element.first.toString().removeSuffix("?")]?.setMeasurable(qualifiedName)
                    if((element.second as ValueFeature).isInput)
                        componentsMap[element.first.toString().removeSuffix("?")]?.setInput(qualifiedName)
                    if((element.second as ValueFeature).isOutput)
                        componentsMap[element.first.toString().removeSuffix("?")]?.setOutput(qualifiedName)
                }
            }

//            println(element.toString())
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
                    if(type!=null) {
                        SystemElements[element.first.toString().removeSuffix("?")]?.addProperty(qualifiedName,type)
                        if((element.second as ValueFeature).isMeasurable)
                            SystemElements[element.first.toString().removeSuffix("?")]?.setMeasurable(qualifiedName)
                        if((element.second as ValueFeature).isInput)
                            SystemElements[element.first.toString().removeSuffix("?")]?.setInput(qualifiedName)
                        if((element.second as ValueFeature).isOutput)
                            SystemElements[element.first.toString().removeSuffix("?")]?.setOutput(qualifiedName)
                    }
                }

                if(element.second is Specialization) {
                    println("element ${(element.second as Specialization).source.first()} is ${(element.second as Specialization).target.first().toString().removeSuffix("?")}")
                    if((getTypeOfElement(element.first.toString().removeSuffix("?"))==null)&&(SystemElements[element.first.toString().split("::").first()]?.consistsOfComponents?.containsKey((element.second as Specialization).source.first().toString())==false)){
                        SystemElements[element.first.toString().split("::").first()]?.consistsOfComponents?.set((element.second as Specialization).source.first().toString(),componentsMap[(element.second as Specialization).target.first().toString().removeSuffix("?")] !!)
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


    val componentsMap = hashMapOf<String,SysMDElement>()
    val globalProperties = hashMapOf<String, SysMDPropertie<*>>()
    val SystemElements = hashMapOf<String, SysMDElement>()
}