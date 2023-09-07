package MQTT

import DTSessionManager
import MQTT.entities.DigitalTwinLoadingRequest
import com.fasterxml.jackson.databind.ObjectMapper
import com.fasterxml.jackson.module.kotlin.readValue
import io.moquette.interception.AbstractInterceptHandler
import io.moquette.interception.messages.InterceptPublishMessage




class PublisherListener : AbstractInterceptHandler() {
    private val objectMapper = ObjectMapper()
    private var writer = objectMapper.writer().withDefaultPrettyPrinter()

    override fun getID(): String {
        return "PublishListener";
    }

    override public fun onPublish(msg: InterceptPublishMessage?) {
        // Create array for payload
        // Create array for payload
        val readableBytes = msg!!.payload.readableBytes()
        val payload = ByteArray(readableBytes)

        // Read bytes from payload

        // Read bytes from payload
        for (i in 0 until readableBytes) {
            payload[i] = msg!!.payload.readByte()
        }

        // Create string from payload

        // Create string from payload
        val decodedPayload = String(payload)
        println("Received on topic: " + msg!!.topicName + " content: " + decodedPayload)

        when(msg!!.topicName){
            GlobalTopics.CONNECT_TO_TWIN.callString -> {
                val loadingReq:DigitalTwinLoadingRequest = objectMapper.readValue<DigitalTwinLoadingRequest>(decodedPayload)
                DTSessionManager.dtSession.connectToDigitalTwin(projectId = loadingReq.projectID, twinId = loadingReq.twinID)
            }
            GlobalTopics.EXIT.callString -> {

            }
            else -> {

            }
        }
    }



}
