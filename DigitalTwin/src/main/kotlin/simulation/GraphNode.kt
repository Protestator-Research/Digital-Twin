package simulation

import Elements.SysMDElement
import java.util.UUID

class GraphNode(
    val id:UUID,
    var SysMDElementCopy:SysMDElement?=null,
    var ConsistsOfGraphNodes:List<GraphNode>?=null,
    var ConnectedNodes:List<GraphNode>
)
{

}