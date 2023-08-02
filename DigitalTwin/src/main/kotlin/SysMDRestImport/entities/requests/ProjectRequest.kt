package SysMDRestImport.entities.requests

import com.fasterxml.jackson.annotation.JsonAlias
import com.fasterxml.jackson.annotation.JsonInclude

class ProjectRequest (
    @JsonInclude(JsonInclude.Include.ALWAYS)
    val name: String,
    var description: String? = null,
    @JsonAlias("default")
    var defaultBranchName: String? = null,
)