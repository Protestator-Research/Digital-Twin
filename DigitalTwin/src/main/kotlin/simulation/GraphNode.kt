package simulation

import Elements.*
import java.util.UUID

class GraphNode(
    var SysMDProperty:SysMDProperty<*>?=null,
    var SysMDElementCopy:SysMDElement?=null,
    var ownGraph:GraphManager?=null,
    var ConnectedNodes:ArrayList<GraphNode> = arrayListOf()
)
{
    fun propagate(){
        if(SysMDProperty!=null){
            for(node in ConnectedNodes){
                when(node.SysMDProperty?.type) {
                    SysMDType.ANY -> TODO()
                    SysMDType.NUMBER ->
                        if((node.SysMDProperty!! is SysMDPropertyNumber) && (SysMDProperty!! is SysMDPropertyNumber))
                            (node.SysMDProperty as SysMDPropertyNumber).setNewValue((SysMDProperty as SysMDPropertyNumber).currentValue)

                    SysMDType.INTEGER ->
                        if((node.SysMDProperty!! is SysMDPropertyInteger) && (SysMDProperty!! is SysMDPropertyInteger))
                            (node.SysMDProperty as SysMDPropertyInteger).setNewValue((SysMDProperty as SysMDPropertyInteger).currentValue)

                    SysMDType.BOOLEAN ->
                        if((node.SysMDProperty!! is SysMDPropertyBoolean) && (SysMDProperty!! is SysMDPropertyBoolean))
                            (node.SysMDProperty as SysMDPropertyBoolean).setNewValue((SysMDProperty as SysMDPropertyBoolean).currentValue)

                    SysMDType.REAL ->
                        if((node.SysMDProperty!! is SysMDPropertyDouble) && (SysMDProperty!! is SysMDPropertyDouble))
                            (node.SysMDProperty as SysMDPropertyDouble).setNewValue((SysMDProperty as SysMDPropertyDouble).currentValue)

                    SysMDType.QUANTITY -> TODO()
                    SysMDType.SCALAR_VALUE -> TODO()

                    SysMDType.STRING ->
                        if((node.SysMDProperty !! is SysMDPropertyString) && (SysMDProperty!! is SysMDPropertyString))
                            (node.SysMDProperty as SysMDPropertyString).setNewValue((SysMDProperty as SysMDPropertyString).currentValue)

                    SysMDType.REQUIREMENT -> TODO()
                    SysMDType.PERFORMANCE -> TODO()
                    SysMDType.QUALITY -> TODO()
                    SysMDType.WORK ->
                        if((node.SysMDProperty!! is SysMDPropertyDouble) && (SysMDProperty!! is SysMDPropertyDouble))
                            (node.SysMDProperty as SysMDPropertyDouble).setNewValue((SysMDProperty as SysMDPropertyDouble).currentValue)

                    SysMDType.MASS ->
                        if((node.SysMDProperty!! is SysMDPropertyDouble) && (SysMDProperty!! is SysMDPropertyDouble))
                            (node.SysMDProperty as SysMDPropertyDouble).setNewValue((SysMDProperty as SysMDPropertyDouble).currentValue)

                    SysMDType.TIME ->
                        if((node.SysMDProperty!! is SysMDPropertyDouble) && (SysMDProperty!! is SysMDPropertyDouble))
                            (node.SysMDProperty as SysMDPropertyDouble).setNewValue((SysMDProperty as SysMDPropertyDouble).currentValue)

                    SysMDType.POWER ->
                        if((node.SysMDProperty!! is SysMDPropertyDouble) && (SysMDProperty!! is SysMDPropertyDouble))
                            (node.SysMDProperty as SysMDPropertyDouble).setNewValue((SysMDProperty as SysMDPropertyDouble).currentValue)

                    SysMDType.LENGTH ->
                        if((node.SysMDProperty!! is SysMDPropertyDouble) && (SysMDProperty!! is SysMDPropertyDouble))
                            (node.SysMDProperty as SysMDPropertyDouble).setNewValue((SysMDProperty as SysMDPropertyDouble).currentValue)

                    SysMDType.SPEED ->
                        if((node.SysMDProperty!! is SysMDPropertyDouble) && (SysMDProperty!! is SysMDPropertyDouble))
                            (node.SysMDProperty as SysMDPropertyDouble).setNewValue((SysMDProperty as SysMDPropertyDouble).currentValue)

                    SysMDType.ACCELLERATION ->
                        if((node.SysMDProperty!! is SysMDPropertyDouble) && (SysMDProperty!! is SysMDPropertyDouble))
                            (node.SysMDProperty as SysMDPropertyDouble).setNewValue((SysMDProperty as SysMDPropertyDouble).currentValue)

                    SysMDType.AREA ->
                        if((node.SysMDProperty!! is SysMDPropertyDouble) && (SysMDProperty!! is SysMDPropertyDouble))
                            (node.SysMDProperty as SysMDPropertyDouble).setNewValue((SysMDProperty as SysMDPropertyDouble).currentValue)

                    SysMDType.VOLUME ->
                        if((node.SysMDProperty!! is SysMDPropertyDouble) && (SysMDProperty!! is SysMDPropertyDouble))
                            (node.SysMDProperty as SysMDPropertyDouble).setNewValue((SysMDProperty as SysMDPropertyDouble).currentValue)

                    SysMDType.CURRENT ->
                        if((node.SysMDProperty!! is SysMDPropertyDouble) && (SysMDProperty!! is SysMDPropertyDouble))
                            (node.SysMDProperty as SysMDPropertyDouble).setNewValue((SysMDProperty as SysMDPropertyDouble).currentValue)

                    SysMDType.CURRENCY ->
                        if((node.SysMDProperty!! is SysMDPropertyDouble) && (SysMDProperty!! is SysMDPropertyDouble))
                            (node.SysMDProperty as SysMDPropertyDouble).setNewValue((SysMDProperty as SysMDPropertyDouble).currentValue)

                    SysMDType.TEMPRETURE ->
                        if((node.SysMDProperty!! is SysMDPropertyDouble) && (SysMDProperty!! is SysMDPropertyDouble))
                            (node.SysMDProperty as SysMDPropertyDouble).setNewValue((SysMDProperty as SysMDPropertyDouble).currentValue)

                    SysMDType.VOLTAGE ->
                        if((node.SysMDProperty!! is SysMDPropertyDouble) && (SysMDProperty!! is SysMDPropertyDouble))
                            (node.SysMDProperty as SysMDPropertyDouble).setNewValue((SysMDProperty as SysMDPropertyDouble).currentValue)

                    SysMDType.CAPACITIY ->
                        if((node.SysMDProperty!! is SysMDPropertyDouble) && (SysMDProperty!! is SysMDPropertyDouble))
                            (node.SysMDProperty as SysMDPropertyDouble).setNewValue((SysMDProperty as SysMDPropertyDouble).currentValue)

                    SysMDType.RESISTANCE ->
                        if((node.SysMDProperty!! is SysMDPropertyDouble) && (SysMDProperty!! is SysMDPropertyDouble))
                            (node.SysMDProperty as SysMDPropertyDouble).setNewValue((SysMDProperty as SysMDPropertyDouble).currentValue)

                    SysMDType.ELEMENT -> TODO()
                    SysMDType.FUNCTION -> TODO()
                    SysMDType.COMPONENT -> TODO()
                    SysMDType.SYSTEM -> TODO()
                    SysMDType.PART -> TODO()
                    SysMDType.SOFTWARE -> TODO()
                    SysMDType.PROCESSOR -> TODO()
                    SysMDType.DISPLAY -> TODO()
                    SysMDType.COMMENT ->
                        if((node.SysMDProperty !! is SysMDPropertyString) && (SysMDProperty!! is SysMDPropertyString))
                            (node.SysMDProperty as SysMDPropertyString).setNewValue((SysMDProperty as SysMDPropertyString).currentValue)

                    SysMDType.shows -> TODO()
                    SysMDType.implmenents -> TODO()
                    SysMDType.implementedBy -> TODO()
                    SysMDType.executes -> TODO()
                    SysMDType.executedby -> TODO()
                    SysMDType.ERROR ->
                        if((node.SysMDProperty!! is SysMDPropertyDouble) && (SysMDProperty!! is SysMDPropertyDouble))
                            (node.SysMDProperty as SysMDPropertyDouble).setNewValue((SysMDProperty as SysMDPropertyDouble).currentValue)

                    null -> TODO()
                }
            }
        }
    }
}