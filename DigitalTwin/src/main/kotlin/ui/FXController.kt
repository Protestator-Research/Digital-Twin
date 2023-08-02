package ui

import MQTT.MQTTClient
import javafx.application.Platform
import javafx.fxml.FXML
import javafx.geometry.Insets
import javafx.scene.Node
import javafx.scene.control.*
import javafx.scene.layout.GridPane
import javafx.util.Callback

class FXController {

    init {
        SessionController.setFxSessionController(this)
    }

    @FXML
    fun onActionOpenConnectionClicked(){
        val dialog: Dialog<Pair<Pair<String,String>,Pair<String,String>>> = Dialog()
        dialog.setTitle("Connection Dialog")
        dialog.setHeaderText("")
        val loginButtonType = ButtonType("Open Connection", ButtonBar.ButtonData.OK_DONE)
        dialog.dialogPane.buttonTypes.addAll(loginButtonType, ButtonType.CANCEL)
        val AGILAIP = TextField()
        AGILAIP.promptText = "AGILA URL / IP"
        val AGILAPort = TextField()
        AGILAPort.promptText = "AGILA Port"

        val MQTTIP = TextField()
        MQTTIP.promptText = "MQTT URL / IP"
        val MQTTPort = TextField()
        MQTTPort.promptText = "AGILA Port"

        val grid = GridPane()
        grid.hgap = 10.0
        grid.vgap = 10.0
        grid.padding = Insets(20.0, 150.0, 10.0, 10.0)

        grid.add(Label("URL / IP:"), 0, 0)
        grid.add(AGILAIP, 1, 0)
        grid.add(Label("Port:"), 0, 1)
        grid.add(AGILAPort, 1, 1)


        val loginButton: Node = dialog.dialogPane.lookupButton(loginButtonType)
        loginButton.setDisable(true)

        AGILAIP.textProperty().addListener { observable, oldValue, newValue ->
            loginButton.setDisable(
                newValue.trim().isEmpty()
            )
        }

        dialog.dialogPane.content = grid

        Platform.runLater { AGILAIP.requestFocus() }

        dialog.setResultConverter(Callback<ButtonType, Pair<Pair<String,String>,Pair<String,String>>?> setResultConverter@{ dialogButton: ButtonType ->
            if (dialogButton == loginButtonType) {
                return@setResultConverter Pair(Pair(AGILAIP.text, AGILAPort.text),Pair(MQTTIP.text,MQTTPort.text))
            }
            null
        })

        val result = dialog.showAndWait()

        SessionController.updateFromServer(result.get().first.first,result.get().first.second.toInt())
        MQTTClient.setServerURLandPort(result.get().second.first,result.get().second.second.toInt())
    }

    @FXML
    fun onPlayButtonClicked(){
        try {
            SessionController.startSimulation()
        }catch (ex:Exception){
            ex.printStackTrace()
        }
    }

    @FXML
    fun onDownloadButtonClicked() {

    }

    fun setNameOfDigitalTwin(name:String) {
        Platform.runLater({DigitalTwinLabel.text=name})
    }

    fun redecorateDigitalTwinStructure() {
        if (SessionController.selectedDT != null) {
            MQTTClient.checkoutProject(SessionController.selectedDT!!.parentProject !!, SessionController.selectedDT!!.id)

            DigitalTwinStructure.root = TreeItem(SessionController.selectedDT!!.name)
            var componentsItem :TreeItem<String> = TreeItem("Components")

            for(key in DTSessionManager.dtSession.componentsMap.keys){
                var componentItem = TreeItem(key)
                var propertyItem= TreeItem("Properties")
                for (pkey in DTSessionManager.dtSession.componentsMap[key]!!.properties){
                    var prop = TreeItem(pkey.key + " : " + pkey.value.type)
                    propertyItem.children.add(prop)
                }
                componentItem.children.add(propertyItem)
                var components = TreeItem("Components")
                for (ckey in DTSessionManager.dtSession.componentsMap[key]!!.consistsOfComponents) {
                    var comp = TreeItem(ckey.key)
                    components.children.add(comp)
                }
                componentItem.children.add(components)
                componentsItem.children.add(componentItem)
            }

            var SystemsItem :TreeItem<String> = TreeItem("Systems")

            for(key in DTSessionManager.dtSession.SystemElements.keys){
                var componentItem = TreeItem(key)
                var propertyItem= TreeItem("Properties")
                for (pkey in DTSessionManager.dtSession.SystemElements[key]!!.properties){
                    var prop = TreeItem(pkey.key + " : " + pkey.value.type)
                    propertyItem.children.add(prop)
                }
                componentItem.children.add(propertyItem)
                var components = TreeItem("Components")
                for (ckey in DTSessionManager.dtSession.SystemElements[key]!!.consistsOfComponents) {
                    var comp = TreeItem(ckey.key)
                    components.children.add(comp)
                }
                componentItem.children.add(components)
                SystemsItem.children.add(componentItem)
            }

            DigitalTwinStructure.root.children.add(componentsItem)
            DigitalTwinStructure.root.children.add(SystemsItem)
        }
    }

    lateinit var DigitalTwinLabel:Label
    lateinit var DigitalTwinStructure:TreeView<String>
}
