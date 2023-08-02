package SysMDRestImport.entities

import com.fasterxml.jackson.annotation.JsonProperty
import java.util.*

open class Identity(
    id: UUID? = null,
) {
    @JsonProperty("@id")
    var id: UUID? = id
}