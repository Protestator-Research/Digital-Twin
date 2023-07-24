package MQTT

import io.moquette.broker.Server
import io.moquette.broker.config.ClasspathResourceLoader
import io.moquette.broker.config.IConfig
import io.moquette.broker.config.IResourceLoader
import io.moquette.broker.config.ResourceLoaderConfig
import io.moquette.interception.InterceptHandler
import io.netty.buffer.Unpooled
import io.netty.handler.codec.mqtt.MqttMessageBuilders
import io.netty.handler.codec.mqtt.MqttQoS
import org.slf4j.Logger
import org.slf4j.LoggerFactory
import java.io.IOException


object Broker {
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
        pushTopic("/exit")
        println("Topics pushed...")
    }

    fun stopServer() {
        mqttBroker.stopServer()
    }

    fun pushTopic(topic: String?) {
        val message = MqttMessageBuilders.publish()
            .topicName(topic)
            .retained(true)
            .qos(MqttQoS.EXACTLY_ONCE)
            .payload(Unpooled.copiedBuffer("{}".encodeToByteArray()))
            .build()
        mqttBroker.internalPublish(message, "INTRLPUB")
    }
}