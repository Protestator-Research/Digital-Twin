package simulation

import Elements.SysMDComponent
import Elements.SysMDElement
import Elements.SysMDProperty
import Elements.SysMDType
import java.util.UUID

class GraphManager {
    fun registerSubNode(element: SysMDElement, Address: String){
        if(allAvailableNodes.keys.contains(Address)){
            if(allAvailableNodes[Address]?.ownGraph != null){
                allAvailableNodes[Address]?.ownGraph?.registerSystemNode(element)
            }else{
                allAvailableNodes[Address]?.ownGraph = GraphManager()
                allAvailableNodes[Address]?.ownGraph?.registerSystemNode(element)
            }
        }
    }

    fun registerSystemNode(element: SysMDElement) {

        for(prop in element.properties) {
            allAvailableNodes[prop.key] = GraphNode(SysMDProperty = prop.value)
        }

        for (input in element.inputs) {
            inputs[input.key] = allAvailableNodes[input.key]!!
        }

        for (output in element.outputs) {
            outputs[output.key] = allAvailableNodes[output.key]!!
        }

        for (comp in element.consistsOfComponents) {
            allAvailableNodes[comp.key] = GraphNode(SysMDElementCopy = comp.value)
            registerSubNode(comp.value, comp.key)
        }
    }


    fun connectTwoNodes(AdressNode1:String,AdressNode2:String) {
        if((getNodeRecursive(AdressNode2.split("::"))!=null)&&(getNodeRecursive(AdressNode1.split("::"))!=null)) {
            getNodeRecursive(AdressNode1.split("::"))?.ConnectedNodes?.add(getNodeRecursive(AdressNode2.split("::"))!!)
            getNodeRecursive(AdressNode2.split("::"))?.ConnectedNodes?.add(getNodeRecursive(AdressNode1.split("::"))!!)
        }else {
            val elementsWithNumbers2 = getNodesWithJustANumberAtTheEnd(AdressNode2.split("::"))
            val elementsWithNumbers1 = getNodesWithJustANumberAtTheEnd(AdressNode1.split("::"))
            if ((elementsWithNumbers1!=null)&&(elementsWithNumbers2!=null)&&(elementsWithNumbers1.size==1)&&((elementsWithNumbers2.size>1))){
                for(element in elementsWithNumbers2) {
                    if (element != null) {
                        element.ConnectedNodes.add(elementsWithNumbers1.first() !!)
                        elementsWithNumbers1.first()!!.ConnectedNodes.add(element)
                    }
                }
            }else if ((elementsWithNumbers1!=null)&&(elementsWithNumbers2!=null)&&(elementsWithNumbers2.size==1)&&((elementsWithNumbers1.size>1))) {
                for(element in elementsWithNumbers1) {
                    if (element != null) {
                        element.ConnectedNodes.add(elementsWithNumbers2.first() !!)
                        elementsWithNumbers2.first()!!.ConnectedNodes.add(element)
                    }
                }
            }else if ((elementsWithNumbers1!=null)&&(elementsWithNumbers2!=null)&&(elementsWithNumbers2.size==elementsWithNumbers1.size)) {
                for (i in 0 until elementsWithNumbers1.size) {
                    elementsWithNumbers1[i]?.ConnectedNodes?.add(elementsWithNumbers2[i] !!)
                    elementsWithNumbers2[i]?.ConnectedNodes?.add(elementsWithNumbers1[i] !!)
                }
            }
        }
    }

    private fun getNodeRecursive(address:List<String>,node:GraphNode? = null,index:Int = 1) : GraphNode? {
        if((index>1)&&(node==null))
            return null
//            throw Exception("Error in Model.")

        if(allAvailableNodes.keys.contains(address[index]))
            return allAvailableNodes[address[index]]

        when (address.size - index) {
            address.size - 1 -> return getNodeRecursive(address,allAvailableNodes[address[index]],index + 1)
            1 -> return node?.ownGraph?.allAvailableNodes?.get(address[index]) ?: null
            else -> return getNodeRecursive(address, node?.ownGraph?.allAvailableNodes?.get(address[index]),index + 1)
        }
    }

    private fun getNodesWithJustANumberAtTheEnd(address: List<String>, node: GraphNode? = null, index: Int = 1) : List<GraphNode?>? {
        if(getNodeRecursive(address, node, index)==null) {
            val returnValue:ArrayList<GraphNode?> = arrayListOf()
            for(i in 1..9)
            {
                var newAddress:String = address[index]+i

                if((index>1)&&(node==null))
                    return null

                if(allAvailableNodes.keys.contains(newAddress))
                    returnValue.add(allAvailableNodes[newAddress] !!)

                when (address.size - index) {
                    address.size - 1 ->
                        getNodesWithJustANumberAtTheEnd(address,allAvailableNodes[newAddress],index + 1)?.forEach({
                            if(it!=null)
                                returnValue.add(it)
                        })
                    else -> getNodesWithJustANumberAtTheEnd(address, node?.ownGraph?.allAvailableNodes?.get(newAddress),index + 1)?.forEach({
                        if(it!=null)
                            returnValue.add(it)
                    })
                }
            }
            return returnValue
        }
        else
            return arrayListOf(getNodeRecursive(address,node, index) !!)
    }

    val inputs = hashMapOf<String,GraphNode>()
    val outputs = hashMapOf<String,GraphNode>()

    val allAvailableNodes = hashMapOf<String,GraphNode>()
}