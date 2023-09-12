package MQTT

import MQTT.entities.DigitalTwinLoadingRequest
import com.fasterxml.jackson.databind.ObjectMapper
import com.hivemq.client.mqtt.MqttClient
import com.hivemq.client.mqtt.mqtt3.Mqtt3AsyncClient
import com.hivemq.client.mqtt.mqtt3.Mqtt3BlockingClient
import java.nio.charset.StandardCharsets
import java.util.*


object MQTTClient {
    private val objectMapper = ObjectMapper()
    private var writer = objectMapper.writer().withDefaultPrettyPrinter()

    private var ServerUrl:String = "cpsiot2.cs.uni-kl.de"
    private var ServerPort:Int = 1884

    private var client:Mqtt3AsyncClient

    init {
        client = MqttClient.builder()
            .identifier(UUID.randomUUID().toString())
            .serverHost(ServerUrl)
            .serverPort(ServerPort)
            .useMqttVersion3()
            .build().toAsync()
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
            .build().toAsync()
        client.connect()
    }

    fun subscribeToTopic(topic:String, callback:(String)->Unit){
        client.subscribeWith()
            .topicFilter(topic)
            .callback { publish ->
                if(publish.payload.isPresent){
                    val string_Value = StandardCharsets.UTF_8.decode(publish.payload.get()).toString()
                    callback(string_Value)
//                    println(publish.topic.toString() + string_Value)
                }
            }
            .send()
            .whenComplete { subAck, throwable ->
                if (throwable != null) {
                    println("Subscribing issue")
                } else {
                    println("Successfully Subscribed")
                }
            }

    }
    fun disconnectFromServer(){
        client.disconnect()
    }
}