import MQTT.Broker
import javafx.application.Application
import ui.MainWindow


fun main() {
   // Start broker
   // Start broker
   Broker.startServer()

// Bind a shutdown hook

// Bind a shutdown hook
   Runtime.getRuntime().addShutdownHook(Thread {
      println("Stopping MQTT broker...")
      Broker.stopServer()
   })

   Application.launch(MainWindow::class.java)
}