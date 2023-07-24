package simulation

import Elements.SysMDElement
import Elements.SysMDProperty
import java.util.UUID

class GraphNode(
    var SysMDProperty:SysMDProperty<*>?=null,
    var SysMDElementCopy:SysMDElement?=null,
    var ownGraph:GraphManager?=null,
    var ConnectedNodes:ArrayList<GraphNode> = arrayListOf()
)
{

}