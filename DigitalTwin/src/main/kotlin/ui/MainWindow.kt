package ui

import javafx.application.Application
import javafx.application.Platform
import javafx.event.ActionEvent
import javafx.event.EventHandler
import javafx.fxml.FXMLLoader
import javafx.geometry.Insets
import javafx.scene.Node
import javafx.scene.Parent
import javafx.scene.Scene
import javafx.scene.control.*
import javafx.scene.control.ButtonBar.ButtonData
import javafx.scene.layout.GridPane
import javafx.stage.Stage
import javafx.util.Callback


lateinit var SessionController:MainWindowController

class MainWindow: Application() {
    override fun start(primaryStage: Stage) {
        SessionController = MainWindowController(::updateState)
        val fxmlLoader = FXMLLoader()
        fxmlLoader.location = MainWindow::class.java.getResource("MainWindow.fxml")
//        fxmlLoader.getController<FXController>().SessionController=Controller
        val parent: Parent = fxmlLoader.load()
        scene = Scene(parent)
        primaryStage.title="MainWindow"
        primaryStage.scene=scene
        primaryStage.show()
        updateState(SessionController.applicationState)
    }

    fun updateState(states: MainWindowStates){
        if (scene!=null) {
            when (states) {
                MainWindowStates.ONLINE_STATE -> {
                    val label = scene!!.lookup("#StatusLabel") as Label
                    label.text = "DT Client Online"
                    redecorateTreeView()
                }

                MainWindowStates.OFFLINE_STATE -> {
                    val label = scene!!.lookup("#StatusLabel") as Label
                    label.text = "DT Client Offline"
                }

                MainWindowStates.DIGITAL_TWIN_SELECTED -> TODO()
            }
        }
    }

    fun redecorateTreeView(){
        val projectTree = scene!!.lookup("#ProjectTreeView") as TreeView<String>
        projectTree.root = TreeItem<String>("Projects")
        projectTree.isShowRoot = true

        for(project in SessionController.projects)
        {
            val projectChild = TreeItem(project.name)
            val projectDTs = SessionController.digitalTwins[project.id] !!
            if(projectDTs.isNotEmpty())
            {
                for(dt in projectDTs)
                {
                    projectChild.children.add(TreeItem(dt.name))
                }
            }else
                projectChild.children.add(TreeItem("No Digital Twins available"))
            projectTree.root.children.add(projectChild)
        }
    }
    private var scene:Scene?=null
}