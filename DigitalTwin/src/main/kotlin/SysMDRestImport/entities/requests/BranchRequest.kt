package SysMDRestImport.entities.requests

import SysMDRestImport.entities.Commit
import com.fasterxml.jackson.annotation.JsonIncludeProperties
import com.fasterxml.jackson.annotation.JsonProperty
import java.util.*

class BranchRequest{
    constructor(id:UUID=UUID.randomUUID(),name: String? = null){
        head.id=id
        this.name=name
    }

    var name:String?=null

    @JsonProperty("@type")
    var type: String = "Branch"

    @JsonIncludeProperties("@id")
    var head: Commit = Commit()

}