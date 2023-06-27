package ui

import javafx.application.Application
import javafx.fxml.FXMLLoader
import javafx.scene.Parent
import javafx.scene.Scene
import javafx.stage.Stage

class MainWindow: Application() {
    override fun start(primaryStage: Stage) {
        val fxmlLoader = FXMLLoader()
        fxmlLoader.location = MainWindow::class.java.getResource("MainWindow.fxml")
        val parent: Parent = fxmlLoader.load()
        val scene = Scene(parent)
        primaryStage.title="MainWindow"
        primaryStage.scene=scene
        primaryStage.show()
    }
}