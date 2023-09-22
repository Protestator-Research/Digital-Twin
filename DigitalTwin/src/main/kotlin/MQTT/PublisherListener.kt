package MQTT

import DTSessionManager
import DigitalTwinSession
import MQTT.entities.DigitalTwinLoadingRequest
import MQTT.entities.EnergyProbeData
import SysMDRestImport.AgilaRepository
import SysMDRestImport.entities.requests.RealTwinDataRequest
import com.fasterxml.jackson.databind.ObjectMapper
import com.fasterxml.jackson.module.kotlin.readValue
import io.moquette.interception.AbstractInterceptHandler
import io.moquette.interception.messages.InterceptPublishMessage




class PublisherListener(val digitalTwinSession:DigitalTwinSession ) : AbstractInterceptHandler() {
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
                digitalTwinSession.connectToDigitalTwin(projectId = loadingReq.projectID, twinId = loadingReq.twinID)
            }
            GlobalTopics.EXIT.callString -> {

            }
            else -> {
                if (!Broker.listener.ignorablePoints.contains(msg!!.topicName)) {
                    val payload: EnergyProbeData = objectMapper.readValue<EnergyProbeData>(decodedPayload)
                    if (dataPoints[msg!!.topicName] == null) {
                        dataPoints[msg!!.topicName] = arrayListOf()
                    }
                    dataPoints[msg!!.topicName]?.add(payload.value)
                    val RTpayload = RealTwinDataRequest()
                    RTpayload.data = dataPoints
                    digitalTwinSession.postDataToDigitalTwinAndBackend(RTpayload, msg!!.topicName)
                    dataPoints.clear()
                }
            }
        }
    }

    val dataPoints:HashMap<String, ArrayList<Float>> = hashMapOf()
    val ignorablePoints:ArrayList<String> = arrayListOf()
    val inputTopics = arrayListOf<String>(
        "DeltaSigma/inputVoltage",
        "DeltaSigma/outputVoltage",
        "DeltaSigma/integrator1/outputVoltage",
        "DeltaSigma/integrator2/outputVoltage")
}
