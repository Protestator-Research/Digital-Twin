package MQTT

import DTSessionManager
import MQTT.entities.DigitalTwinLoadingRequest
import MQTT.entities.EnergyProbeData
import SysMDRestImport.AgilaRepository
import SysMDRestImport.entities.requests.RealTwinDataRequest
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

    override fun onPublish(msg: InterceptPublishMessage?) {
        val readableBytes = msg!!.payload.readableBytes()
        val payload = ByteArray(readableBytes)

        for (i in 0 until readableBytes) {
            payload[i] = msg!!.payload.readByte()
        }

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
                val payload:EnergyProbeData = objectMapper.readValue<EnergyProbeData>(decodedPayload)
                if(dataPoints[msg!!.topicName]==null)
                {
                    dataPoints[msg!!.topicName] = arrayListOf()
                }
                dataPoints[msg!!.topicName]?.add(payload.value)
                if(dataPoints[msg!!.topicName]?.size !! > 10) {
                    val payload = RealTwinDataRequest()
                    payload.data = dataPoints
                    DTSessionManager.dtSession.postDataToDigitalTwinAndBackend(payload)
                }
            }
        }
    }

    val dataPoints:HashMap<String, ArrayList<Float>> = hashMapOf()
}
