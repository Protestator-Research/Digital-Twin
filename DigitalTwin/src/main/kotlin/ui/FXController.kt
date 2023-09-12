package ui

import Elements.SysMDElement
import MQTT.MQTTClient
import MQTT.entities.EnergyProbeData
import SysMDRestImport.entities.requests.DigitalTwinDataRequest
import com.fasterxml.jackson.databind.ObjectMapper
import com.fasterxml.jackson.module.kotlin.readValue
import javafx.application.Platform
import javafx.collections.ObservableList
import javafx.fxml.FXML
import javafx.geometry.Insets
import javafx.scene.Node
import javafx.scene.chart.CategoryAxis
import javafx.scene.chart.LineChart
import javafx.scene.chart.NumberAxis
import javafx.scene.chart.XYChart
import javafx.scene.control.*
import javafx.scene.layout.GridPane
import javafx.util.Callback
import java.text.SimpleDateFormat

class FXController {

    val xAxis: CategoryAxis = CategoryAxis()
    val yAxis: NumberAxis = NumberAxis()
    @FXML
    lateinit var DTDataChart : LineChart<String, Number>
    private val sdf1 = SimpleDateFormat("mm:ss.SSS")
    private val objectMapper = ObjectMapper()


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
        MQTTPort.promptText = "MQTT Port"

        val grid = GridPane()
        grid.hgap = 10.0
        grid.vgap = 10.0
        grid.padding = Insets(20.0, 150.0, 10.0, 10.0)

        grid.add(Label("Backend URL / IP:"), 0, 0)
        grid.add(AGILAIP, 1, 0)
        grid.add(Label("Bakcned Port:"), 0, 1)
        grid.add(AGILAPort, 1, 1)

        grid.add(Label("MQTT URL / IP:"), 0, 2)
        grid.add(MQTTIP, 1, 2)
        grid.add(Label("MQTT Port:"), 0, 3)
        grid.add(MQTTPort, 1, 3)


        val loginButton: Node = dialog.dialogPane.lookupButton(loginButtonType)
        loginButton.setDisable(true)

        AGILAIP.textProperty().addListener { _, _, newValue ->
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

            DigitalTwinStructure.root = TreeItem("Systems")

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
//                    var comp = TreeItem(ckey.key)
                    components.children.add(getTreeForComponent(ckey.key, ckey.value))
                }
                componentItem.children.add(components)
                DigitalTwinStructure.root.children.add(componentItem)
            }
        }
        DigitalTwinStructure.selectionModel.selectedItemProperty()
            .addListener { observable, oldValue, newValue ->
                mouseClickOnDTTreeItem()
            }
    }


    fun getTreeForComponent(key:String, elem: SysMDElement):TreeItem<String> {
        val item = TreeItem<String>(key)

        val propItem = TreeItem<String>("Properties")

        for(iter in elem.inputs)
            propItem.children.add(TreeItem(iter.key + " : " + iter.value.type.name))
        for(iter in elem.outputs)
            propItem.children.add(TreeItem(iter.key + " : " + iter.value.type.name))

        item.children.add(propItem)

        val compItem = TreeItem<String>("Components")
        for(iter in elem.consistsOfComponents)
            compItem.children.add(getTreeForComponent(iter.key, iter.value))
        item.children.add(compItem)

        return item
    }
    @FXML
    fun mouseClickOnDTTreeItem(){
        val selectedItems = DigitalTwinStructure.selectionModel.selectedItems
        for (item in selectedItems) {
            val mqttTesterName = checkForPropertiesAndCreateMQTTTree(item)
            println(mqttTesterName)
            if (mqttTesterName.isNotEmpty()) {
                tableColumnsMap[mqttTesterName] = TableColumn(item.value)

                lineChatsMap[mqttTesterName] = XYChart.Series<String, Number>()
                lineChatsMap[mqttTesterName]?.name = mqttTesterName

                MQTTClient.subscribeToTopic(mqttTesterName, {
                    var dataElement = objectMapper.readValue<EnergyProbeData>(it)
                    Platform.runLater {
                        lineChatsMap[mqttTesterName]?.data?.add(
                            XYChart.Data(
                                dataElement.current_time,
                                dataElement.value
                            )
                        )
                        if (lineChatsMap[mqttTesterName]?.data?.size!! > 40) {
                            lineChatsMap[mqttTesterName]?.data?.remove(lineChatsMap[mqttTesterName]?.data?.first()!!)
                        }
                    }
                })

                DTDataChart.data.add(lineChatsMap[mqttTesterName]!!)
            }
        }
    }

    fun checkForPropertiesAndCreateMQTTTree(selectedItem: TreeItem<String>):String {
        for(item in DigitalTwinStructure.root.children) {
            val returnValue = checkForPropertiesAndCreateMQTTTreeState(item, selectedItem)
            if(returnValue.isNotEmpty())
                return returnValue
        }
        return ""
    }

    fun checkForPropertiesAndCreateMQTTTreeState(treeState:TreeItem<String>, selectedItem: TreeItem<String>):String {
        if (treeState.children.isNotEmpty()) {
            for (item in treeState.children[0].children) {
                if (item == selectedItem) {
                    return treeState.value + "/" + item.value.split(":").first().removeSuffix(" ")
                }
            }
            for (item in treeState.children[1].children) {
                val previousRetrunValue = checkForPropertiesAndCreateMQTTTreeState(item, selectedItem)
                if (previousRetrunValue.isNotEmpty()) {
                    return treeState.value +"/"+ previousRetrunValue
                }
            }
        }
        return ""
    }

    @FXML
    fun writeVauesToTableAndGraph(topic:String, value: String){
        tableColumnsDataMap[topic]?.add(value)
    }

    lateinit var DigitalTwinLabel:Label
    lateinit var DigitalTwinStructure:TreeView<String>
//    lateinit var DTDataTable:TableView<ObservableList<String>>
    var tableColumnsMap:HashMap<String,TableColumn<ObservableList<String>, String>> = hashMapOf()
    var tableColumnsDataMap:HashMap<String,ObservableList<String>> = hashMapOf()
    var lineChatsMap:HashMap<String, XYChart.Series<String, Number>> = hashMapOf()
}
