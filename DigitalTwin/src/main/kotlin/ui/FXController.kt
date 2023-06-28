package ui

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
        val dialog: Dialog<Pair<String, String>> = Dialog()
        dialog.setTitle("Connection Dialog")
        dialog.setHeaderText("")
        val loginButtonType = ButtonType("Open Connection", ButtonBar.ButtonData.OK_DONE)
        dialog.dialogPane.buttonTypes.addAll(loginButtonType, ButtonType.CANCEL)
        val username = TextField()
        username.promptText = "URL / IP"
        val password = TextField()
        password.promptText = "Port"

        val grid = GridPane()
        grid.hgap = 10.0
        grid.vgap = 10.0
        grid.padding = Insets(20.0, 150.0, 10.0, 10.0)

        grid.add(Label("URL / IP:"), 0, 0)
        grid.add(username, 1, 0)
        grid.add(Label("Port:"), 0, 1)
        grid.add(password, 1, 1)


        val loginButton: Node = dialog.dialogPane.lookupButton(loginButtonType)
        loginButton.setDisable(true)

        username.textProperty().addListener { observable, oldValue, newValue ->
            loginButton.setDisable(
                newValue.trim().isEmpty()
            )
        }

        dialog.dialogPane.content = grid

        Platform.runLater { username.requestFocus() }

        dialog.setResultConverter(Callback<ButtonType, Pair<String,String>?> setResultConverter@{ dialogButton: ButtonType ->
            if (dialogButton == loginButtonType) {
                return@setResultConverter Pair(username.text, password.text)
            }
            null
        })

        val result = dialog.showAndWait()

        SessionController.updateFromServer(result.get().first,result.get().second.toInt())
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

    lateinit var DigitalTwinLabel:Label
}
