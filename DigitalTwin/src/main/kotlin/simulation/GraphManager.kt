package simulation

import Elements.SysMDElement
import java.util.UUID

class GraphManager {
    fun registerNode(element: SysMDElement, uuid: UUID){
        for(component in element.consistsOfComponents){
//            registerNode()
        }

    }

    fun connectTwoNodes(uuidElem1: UUID,uuidElem2: UUID) {

    }

    val allAvailableNodes = hashMapOf<UUID,GraphNode>()
}