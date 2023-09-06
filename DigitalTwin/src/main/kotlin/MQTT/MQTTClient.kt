package MQTT

import MQTT.entities.DigitalTwinLoadingRequest
import com.fasterxml.jackson.databind.ObjectMapper
import com.hivemq.client.mqtt.MqttClient
import com.hivemq.client.mqtt.mqtt3.Mqtt3BlockingClient
import com.hivemq.client.mqtt.mqtt5.Mqtt5AsyncClient
import com.hivemq.client.mqtt.mqtt5.Mqtt5BlockingClient
import java.util.*


object MQTTClient {
    private val objectMapper = ObjectMapper()
    private var writer = objectMapper.writer().withDefaultPrettyPrinter()

    private var ServerUrl:String = "localhost"
    private var ServerPort:Int = 1884

    private var client:Mqtt3BlockingClient

    init {
        client = MqttClient.builder()
            .identifier(UUID.randomUUID().toString())
            .serverHost(ServerUrl)
            .serverPort(ServerPort)
            .useMqttVersion3()
            .build().toBlocking()
        client.connect()
    }

    fun checkoutProject(projectId:UUID, dtId:UUID) {
        val dtReq = DigitalTwinLoadingRequest()
        dtReq.projectID = projectId
        dtReq.twinID = dtId

        val result = client.publishWith()
            .topic(GlobalTopics.CONNECT_TO_TWIN.callString)
            .payload(writer.writeValueAsBytes(dtReq))
            .send()
        println(result)
    }

    fun setServerURLandPort(url:String,port:Int){
        client.disconnect()
        ServerUrl = url
        ServerPort = port
        client = MqttClient.builder()
            .identifier(UUID.randomUUID().toString())
            .serverHost(ServerUrl)
            .serverPort(ServerPort)
            .useMqttVersion3()
            .build().toBlocking()
        client.connect()
    }

    fun disconnectFromServer(){
        client.disconnect()
    }
}