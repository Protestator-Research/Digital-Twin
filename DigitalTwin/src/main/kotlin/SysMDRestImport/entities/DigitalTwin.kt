package SysMDRestImport.entities


import com.fasterxml.jackson.annotation.JsonIgnore
import com.fasterxml.jackson.annotation.JsonProperty
import java.util.UUID
import kotlin.collections.ArrayList
import kotlin.collections.HashMap


class DigitalTwin  {
    @JsonProperty("@id")
    var id: UUID = UUID.randomUUID()
    @JsonProperty("@type")
    var type:String="DigitalTwin"

    var connectedModels : ArrayList<UUID> = arrayListOf()

    var name:String?=null

    var parentProject:UUID?=null

    var branchId : UUID = UUID.randomUUID()
    var commitId : UUID = UUID.randomUUID()

    @JsonIgnore
    var data:HashMap<String,ArrayList<Any>> = hashMapOf()
}