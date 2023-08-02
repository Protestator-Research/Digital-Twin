package SysMDRestImport.entities.requests

import com.fasterxml.jackson.annotation.JsonProperty
import java.util.*

class DigitalTwinRequest (
    iden:UUID = UUID.randomUUID(),
    var name : String? = null,
    var branchId : UUID,
    var commitId : UUID,
    var connectedElements : MutableList<UUID> = mutableListOf()
) {
    @JsonProperty("@id")
    var id:UUID = iden
}