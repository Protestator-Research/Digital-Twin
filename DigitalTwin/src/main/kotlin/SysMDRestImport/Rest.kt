package SysMDRestImport

import SysMDRestImport.entities.requests.RealTwinDataRequest
import com.fasterxml.jackson.core.JsonProcessingException
import com.fasterxml.jackson.databind.ObjectMapper
import org.json.JSONObject
import org.springframework.http.*
import org.springframework.lang.NonNull
import org.springframework.lang.Nullable
import org.springframework.web.client.RestTemplate


object Rest {
    // Will result in "http://baseURI:port/entryURI
    private val protocol = "http://"
    var baseURI = "cps-agila.cs.rptu.de"
//    var baseURI = "192.168.0.14"
    var entryURI: String = "/agila-server"
    var port: Int = 8080

    private var auth: String = "none"
    private val objectMapper = ObjectMapper()

    /**
     * @SpringBootTest registers a TestRestTemplate bean so we can directly @Autowire
     */
    private val restTemplate: RestTemplate = RestTemplate()

    private fun generateURI(endpoint: String, @Nullable queryParameters : Map<String, String>? = null): String {
        // if no query parameters are given, return plain endpoint URI
        var queryParameterString = ""
        if (queryParameters != null) {
            val entries: List<String> = queryParameters.entries.map { "${it.key}=${it.value}" }
            queryParameterString = entries.joinToString(prefix="?", separator = "&")
        }
        return "$protocol${baseURI}:$port$entryURI$endpoint$queryParameterString"
    }

    private fun generateHttpHeaders(withAuthToken: Boolean = true, @Nullable sessionId: String?): HttpHeaders {
        // set headers to accept and provide json data. also include the bearer auth token
        val headers = HttpHeaders()
        headers.contentType = MediaType.APPLICATION_JSON
        headers.accept = mutableListOf(MediaType.APPLICATION_JSON)
        if (withAuthToken)
            headers.setBearerAuth(auth)
        if (sessionId != null) {
            headers.set("SessionId", sessionId)
        }
        return headers
    }

    @JvmOverloads
    fun requestToEndpoint(endpoint: String, @Nullable payload: String?, method: HttpMethod, @Nullable queryParameters : Map<String, String>? = null, withAuthToken: Boolean = true, @Nullable sessionId: String?): ResponseEntity<String> {
        //generate the default header information: ContentType and accepted response are JSON. Bearer auth token is optional
        val headers = generateHttpHeaders(withAuthToken, sessionId = sessionId)

        // formulate the request with the default header and given payload
        val request = HttpEntity(payload, headers)
        return restTemplate.exchange(generateURI(endpoint, queryParameters), method, request, String::class.java)
    }

    fun requestToEndpoint(endpoint: String, @NonNull payload: JSONObject, method: HttpMethod, @Nullable queryParameters : Map<String, String>? = null, withAuthToken: Boolean, @Nullable sessionId: String?): ResponseEntity<String> {
        return this.requestToEndpoint(endpoint, payload.toString(), method, queryParameters, withAuthToken, sessionId)
    }

    fun requestToEndpoint(endpoint: String, @NonNull payload: JSONObject, method: HttpMethod, @Nullable queryParameters : Map<String, String>? = null, @Nullable sessionId: String?): ResponseEntity<String> {
        return this.requestToEndpoint(endpoint, payload.toString(), method, queryParameters, true, sessionId)
    }

    fun get(endpoint: String, @Nullable queryParameters : Map<String, String>? = null, @Nullable sessionId: String?): ResponseEntity<String> {
        return this.requestToEndpoint(endpoint, null as String?, HttpMethod.GET, queryParameters, true, sessionId)
    }

    fun get(endpoint: String, @Nullable sessionId: String?): ResponseEntity<String> {
        return this.requestToEndpoint(endpoint, null as String?, HttpMethod.GET, null, true, sessionId)
    }

    fun post(endpoint: String, @NonNull payload: JSONObject, @Nullable queryParameters : Map<String, String>? = null, @Nullable sessionId: String?): ResponseEntity<String> {
        return this.requestToEndpoint(endpoint, payload, HttpMethod.POST, queryParameters, true, sessionId)
    }

    fun post(endpoint: String, @NonNull payload: JSONObject, @Nullable sessionId: String?): ResponseEntity<String> {
        return this.post(endpoint, payload, null, sessionId)
    }

    fun post(endpoint: String, @Nullable payload: String?, @Nullable queryParameters : Map<String, String>? = null, @Nullable sessionId: String?): ResponseEntity<String> {
        return this.requestToEndpoint(endpoint, payload, HttpMethod.POST, queryParameters, true, sessionId)
    }

    fun post(endpoint: String, @Nullable payload: String?, @Nullable sessionId: String?): ResponseEntity<String> {
        return this.post(endpoint, payload, null, sessionId)
    }

    fun put(endpoint: String, @NonNull payload: JSONObject, @Nullable queryParameters : Map<String, String>? = null, @Nullable sessionId: String?): ResponseEntity<String> {
        return this.requestToEndpoint(endpoint, payload, HttpMethod.PUT, queryParameters, true, sessionId)
    }

    fun put(endpoint: String, @NonNull payload: JSONObject, @Nullable sessionId: String?): ResponseEntity<String> {
        return this.put(endpoint, payload, null, sessionId)
    }

    fun put(endpoint: String, @Nullable payload: String?, @Nullable queryParameters : Map<String, String>? = null, @Nullable sessionId: String?): ResponseEntity<String> {
        return this.requestToEndpoint(endpoint, payload, HttpMethod.PUT, queryParameters, true, sessionId)
    }

    fun put(endpoint: String, @Nullable payload: String?, @Nullable sessionId: String?): ResponseEntity<String> {
        return this.put(endpoint, payload,  null, sessionId)
    }

    fun patch(endpoint: String, @NonNull payload: JSONObject, @Nullable queryParameters : Map<String, String>? = null, @Nullable sessionId: String?): ResponseEntity<String> {
        return this.requestToEndpoint(endpoint, payload, HttpMethod.PATCH, queryParameters, true, sessionId)
    }

    fun patch(endpoint: String, @NonNull payload: JSONObject, @Nullable sessionId: String?): ResponseEntity<String> {
        return this.patch(endpoint, payload, null, sessionId)
    }

    fun patch(endpoint: String, @Nullable payload: String?, @Nullable queryParameters : Map<String, String>? = null, @Nullable sessionId: String?): ResponseEntity<String> {
        return this.requestToEndpoint(endpoint, payload, HttpMethod.PATCH, queryParameters, true, sessionId)
    }

    fun patch(endpoint: String, @Nullable payload: String?, @Nullable sessionId: String?): ResponseEntity<String> {
        return this.patch(endpoint, payload, null, sessionId)
    }

    fun delete(endpoint: String, @NonNull payload: JSONObject, @Nullable queryParameters : Map<String, String>? = null, @Nullable sessionId: String?): ResponseEntity<String> {
        return this.requestToEndpoint(endpoint, payload, HttpMethod.DELETE, queryParameters, true, sessionId)
    }

    fun delete(endpoint: String, @NonNull payload: JSONObject, @Nullable sessionId: String?): ResponseEntity<String> {
        return this.delete(endpoint, payload, null, sessionId)
    }

    fun delete(endpoint: String, @Nullable payload: String?, @Nullable queryParameters : Map<String, String>? = null, @Nullable sessionId: String?): ResponseEntity<String> {
        return this.requestToEndpoint(endpoint, payload, HttpMethod.DELETE, queryParameters, true, sessionId)
    }

    fun delete(endpoint: String, @Nullable payload: String?, @Nullable sessionId: String?): ResponseEntity<String> {
        return this.delete(endpoint, payload, null, sessionId)
    }

    @Nullable
    fun extractKeyFromBody(key: String, body: String?): String? {
        try {
            val root = objectMapper.readTree(body)
            return root[key].asText()
        } catch (e: JsonProcessingException) {
            println("Key $key not found in body.")
        }
        return null
    }

    @Nullable
    fun extractKeyFromBody(key: String, body: ResponseEntity<String?>): String? {
        return this.extractKeyFromBody(key, body.body)
    }

    @Nullable
    fun extractEntityIdFromBody(body: String?): String? {
        return this.extractKeyFromBody("@id", body)
    }

    @Nullable
    fun extractEntityUUIDFromBody(body: String?):String? {
        return this.extractKeyFromBody("entityId",body)
    }

    @Nullable
    fun extractEntityIdFromBody(body: ResponseEntity<String?>): String? {
        return this.extractKeyFromBody("entityId", body.body)
    }


    fun login(endpoint: String,
              userKey: String, user: String,
              passwordKey: String, password: String)
    {
        val loginJsonObject = JSONObject()
                .put(userKey, user)
                .put(passwordKey, password)

        val loginResult = requestToEndpoint(endpoint, loginJsonObject, HttpMethod.POST, withAuthToken=false, sessionId = null)

        // check if log in was successful
        // set the bearer token, as we will need that for following requests
        try {
            // remove the "Bearer: "
            auth = loginResult.headers["authorization"].toString().substring(7)
            // remove the closing bracket
            auth = auth.substring(0, auth.length - 1)
        } catch (e: NullPointerException) {
            throw Exception("login failed")
        }
    }
}
