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

    /**
     * Connects two nodes with the given Address from SysMD
     */
    fun connectTwoNodes(AdressNode1:String,AdressNode2:String) {
        println("Connect Node $AdressNode2 to $AdressNode1")
        if((getNodeRecursive(AdressNode2.split("::"))!=null)&&(getNodeRecursive(AdressNode1.split("::"))!=null)) {
            println("In Reality connected Nodes ${getNodeRecursive(AdressNode1.split("::"))?.SysMDProperty?.topic} to ${getNodeRecursive(AdressNode1.split("::"))?.SysMDProperty?.topic}")
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

    /**
     * Checks the list recursive and Returns the fitting node for the path
     */
    private fun getNodeRecursive(address:List<String>,index:Int = 1) : GraphNode? {
        if(index == address.size)
            return null

        if(allAvailableNodes.isNullOrEmpty())
            return null

        if(allAvailableNodes.containsKey(address[index]) && (allAvailableNodes[address[index]]?.ownGraph != null) && (allAvailableNodes[address[index]]?.ownGraph?.allAvailableNodes?.size !!>0)) {
            return allAvailableNodes[address[index]]?.ownGraph?.getNodeRecursive(address,index+1)
        } else if(allAvailableNodes.containsKey(address[index])){
            return allAvailableNodes[address[index]]
        }else {
            return null
        }
    }

    private fun getNodesWithJustANumberAtTheEnd(address: List<String>, node: GraphNode? = null, index: Int = 1) : List<GraphNode?>? {
//        if(getNodeRecursive(address, index)==null) {
//            val returnValue:ArrayList<GraphNode?> = arrayListOf()
//            for(i in 1..9)
//            {
//                var newAddress:String = address[index]+i
//
//                if((index>1)&&(node==null))
//                    return null
//
//                if(allAvailableNodes.containsKey(address[index]) && (allAvailableNodes[address[index]]?.ownGraph?.allAvailableNodes?.size !!>0)) {
//                    return allAvailableNodes[address[index]]?.ownGraph?.getNodeRecursive(address,index+1)
//                } else if(allAvailableNodes.containsKey(address[index])){
//                    return allAvailableNodes[address[index]]
//                }else {
//                    return null
//                }
//            }
//            return returnValue
//        }
//        else
//            return arrayListOf(getNodeRecursive(address, index) !!)
        return null
    }

    fun propagateValues(topic:String) {
        val node = getNodeRecursive(topic.split("/"))
        node?.propagate()

    }

    val inputs = hashMapOf<String,GraphNode>()
    val outputs = hashMapOf<String,GraphNode>()

    val allAvailableNodes = hashMapOf<String,GraphNode>()
}