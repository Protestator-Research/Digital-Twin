package SysMDRestImport.entities.user

import com.fasterxml.jackson.annotation.JsonProperty

class Token {
    //    @Id
    @JsonProperty("@id")
    private val id: String? = null
    private val token: String? = null
    private val userEmail: String? = null
    private val isBlackListed = false
}
