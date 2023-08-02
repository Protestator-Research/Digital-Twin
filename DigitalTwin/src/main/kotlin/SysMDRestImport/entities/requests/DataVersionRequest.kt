package SysMDRestImport.entities.requests

import SysMDRestImport.entities.requests.commitData.CommitData
import com.fasterxml.jackson.annotation.JsonInclude
import com.fasterxml.jackson.annotation.JsonProperty
import java.util.UUID

class DataVersionRequest(
    id: UUID? = null,
    type: String = RESPONSE_TYPE,
    identity: DataIdentityRequest? = null,
    @JsonInclude(JsonInclude.Include.NON_NULL)
    val payload: CommitData? = null,
) {
    @JsonProperty("@id")
    val id:UUID? = id
    @JsonProperty("@type")
    var type: String = type
    @JsonInclude(JsonInclude.Include.NON_NULL)
    var identity: DataIdentityRequest? = identity

    companion object {
        const val RESPONSE_TYPE: String = "DataVersion"
    }
}

class DataIdentityRequest(
    id: UUID,
    type: String = RESPONSE_TYPE
) {
    @JsonProperty("@id")
    val id:UUID = id
    @JsonProperty("@type")
    var type: String = type

    companion object {
        const val RESPONSE_TYPE: String = "DataIdentity"
    }
}
