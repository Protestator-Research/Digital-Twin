package SysMDRestImport.entities.requests

import com.fasterxml.jackson.annotation.JsonProperty

/**
 * Request model for commits:
 * - name
 * - description
 * - id of the previous commit or null, if no previous commit.
 *
 * Changes since v2.8:
 * - Payload is an Array named elements with all elements to be committed.
 * (currently, all --> future: only changed elements)
 */
class CommitRequest(
    type: String = RESPONSE_TYPE,
    var description: String? = null,
    var change: List<DataVersionRequest> = ArrayList()
) {
    @JsonProperty("@type")
    var type: String = type

    companion object {
        const val RESPONSE_TYPE: String = "Commit"
    }
}