package MQTT.entities

import java.util.*

class DigitalTwinLoadingRequest {
    var projectID: UUID = UUID.randomUUID()
    var twinID:UUID = UUID.randomUUID()
}