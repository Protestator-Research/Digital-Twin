package ui

import javafx.application.Application
import javafx.fxml.FXMLLoader
import javafx.scene.Parent
import javafx.scene.Scene
import javafx.scene.control.Label
import javafx.scene.control.TreeItem
import javafx.scene.control.TreeView
import javafx.stage.Stage


lateinit var SessionController:MainWindowController

class MainWindow: Application() {
    override fun start(primaryStage: Stage) {
        SessionController = MainWindowController(::updateState)
        val fxmlLoader = FXMLLoader()
        fxmlLoader.location = MainWindow::class.java.getResource("MainWindow.fxml")
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
        projectTree.root.isExpanded=true
        projectTree.isShowRoot = true

        for(project in SessionController.projects)
        {
            val projectChild = TreeItem(project.name)
            val projectDTs = SessionController.digitalTwins[project.id] !!
            if(projectDTs.isNotEmpty())
            {
                for(dt in projectDTs)
                {
                    val dtItem = TreeItem(dt.name)
                    projectChild.children.add(dtItem)
                }
            }else
                projectChild.children.add(TreeItem("No Digital Twins available"))
            projectTree.root.children.add(projectChild)
        }

        projectTree.selectionModel.selectedItemProperty()
            .addListener { observable, oldValue, newValue ->
                SessionController.onDigitalTwinSelected(newValue.value)
            }
    }
    private var scene:Scene?=null
}