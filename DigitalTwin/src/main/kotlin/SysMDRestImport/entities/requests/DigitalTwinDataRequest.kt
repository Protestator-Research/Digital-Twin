package SysMDRestImport.entities.requests

import java.util.*

class DigitalTwinDataRequest(
//    identification:UUID? = null,
    var beginning: Date = Date(2000,1,1),
    var ending: Date = Date(2000,1,1)
) {
//    @JsonProperty("@id")
//    val id:UUID? = identification
}