package MQTT

import MQTT.entities.DigitalTwinLoadingRequest
import com.fasterxml.jackson.databind.ObjectMapper
import io.moquette.broker.Server
import io.moquette.broker.config.ClasspathResourceLoader
import io.moquette.broker.config.IConfig
import io.moquette.broker.config.IResourceLoader
import io.moquette.broker.config.ResourceLoaderConfig
import io.moquette.interception.InterceptHandler
import io.netty.buffer.Unpooled
import io.netty.handler.codec.mqtt.MqttMessageBuilders
import io.netty.handler.codec.mqtt.MqttPublishMessage
import io.netty.handler.codec.mqtt.MqttQoS
import org.slf4j.Logger
import org.slf4j.LoggerFactory
import java.io.IOException



object Broker {
    private val objectMapper = ObjectMapper()
    private var writer = objectMapper.writer().withDefaultPrettyPrinter()

    val mqttBroker: Server = Server()
    fun startServer() {

        // Load class path for configuration
        val classpathLoader: IResourceLoader = ClasspathResourceLoader()
        val classPathConfig: IConfig = ResourceLoaderConfig(classpathLoader)

        // Start MQTT broker
        println("Start MQTT broker...")
        val userHandlers: List<*> = listOf<Any>(PublisherListener())
        try {
            mqttBroker.startServer(classPathConfig, userHandlers as MutableList<out InterceptHandler>?)
        } catch (e: IOException) {
            println("MQTT broker start failed...")
        }

        // Wait before publish topics
        println("Wait before topics are pushed...")
        try {
            Thread.sleep(20000)
        } catch (e: InterruptedException) {
            println("Pause for publishing topics interupted.")
        }

        // Publishing topics
        println("Pushing topics...")

        for (topic in GlobalTopics.values())
            pushTopic(topic.callString)

        println("Topics pushed...")
    }

    fun stopServer() {
        mqttBroker.stopServer()
    }

    fun pushTopic(topic: String?) {
        var message = MqttMessageBuilders.publish()
            .topicName(topic)
            .retained(true)
            .qos(MqttQoS.EXACTLY_ONCE)
            .payload(Unpooled.copiedBuffer("{}".encodeToByteArray()))
            .build()
        when(topic){
            GlobalTopics.CONNECT_TO_TWIN.callString -> {
                message = MqttMessageBuilders.publish()
                    .topicName(topic)
                    .retained(true)
                    .qos(MqttQoS.EXACTLY_ONCE)
                    .payload(Unpooled.copiedBuffer(writer.writeValueAsString(DigitalTwinLoadingRequest()).toByteArray()))
                    .build()
            }
            else -> {

            }
        }

        mqttBroker.internalPublish(message, "INTRLPUB")
    }

    fun publishToTopic(topic:String, payload:String) {
        var message = MqttMessageBuilders.publish()
            .topicName(topic)
            .retained(true)
            .qos(MqttQoS.EXACTLY_ONCE)
            .payload(Unpooled.copiedBuffer(payload.toByteArray()))
            .build()
    }
}