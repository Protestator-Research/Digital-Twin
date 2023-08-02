package SysMDRestImport.entities.requests

import com.fasterxml.jackson.annotation.JsonAlias
import java.util.*

data class SimpleElementRequestModel (
    @JsonAlias("@id")
    val id: UUID? = null,
    val desc: String? = null
)
