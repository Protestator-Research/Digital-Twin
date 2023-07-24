package MQTT

import io.moquette.interception.AbstractInterceptHandler
import io.moquette.interception.messages.InterceptPublishMessage




class PublisherListener : AbstractInterceptHandler() {
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
    }

}
