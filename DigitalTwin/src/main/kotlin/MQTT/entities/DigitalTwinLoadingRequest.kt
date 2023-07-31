package MQTT.entities

import java.util.*

class DigitalTwinLoadingRequest {
    val projectID: UUID = UUID.randomUUID()
    val twinID:UUID = UUID.randomUUID()
}