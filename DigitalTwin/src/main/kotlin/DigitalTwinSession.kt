import Elements.SysMDComponent
import Elements.SysMDElement
import Elements.SysMDType
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
                println((element.second as ValueFeature).qualifiedName)
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

            println(element.toString())
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
}