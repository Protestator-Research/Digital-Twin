package ui

import DTSessionManager
import DigitalTwinSession
import Parser.DigitalTwinParser
import SysMDRestImport.AgilaRepository
import SysMDRestImport.ElementDAO
import SysMDRestImport.Rest
import SysMDRestImport.entities.DigitalTwin
import SysMDRestImport.entities.Project

import javafx.application.Platform
import java.util.*
import kotlin.collections.HashMap


enum class MainWindowStates {
    OFFLINE_STATE,
    ONLINE_STATE,
    DIGITAL_TWIN_SELECTED
}

class MainWindowController
    (var onUpdateState:(MainWindowStates)->Unit) {

    var projects:MutableList<Project>
    var digitalTwins:HashMap<UUID,MutableList<DigitalTwin>> = hashMapOf()
    var applicationState:MainWindowStates
    var elements:MutableList<ElementDAO> = mutableListOf()
    var selectedDT:DigitalTwin?=null

    private lateinit var fxController: FXController

    init {
        Rest.baseURI= "cpsiot2.cs.uni-kl.de"
        Rest.port = 8081
        projects = AgilaRepository.getProjects()
        getDigitalTwinsOfProjects()
        applicationState = if(projects.isNotEmpty()) MainWindowStates.ONLINE_STATE else MainWindowStates.OFFLINE_STATE
    }
    fun getDigitalTwinsOfProjects(){
        for (project in projects)
        {
            DTSessionManager.dtSession.loadedProjects.add(project.id)
            digitalTwins[project.id]=AgilaRepository.getDigitalTwinsFromProject(project.id)
        }
    }

    fun updateFromServer(domain:String,port:Int){
        Rest.baseURI=(domain)
        Rest.port = port
        projects = AgilaRepository.getProjects()
        getDigitalTwinsOfProjects()
        applicationState = if(projects.isNotEmpty()) MainWindowStates.ONLINE_STATE else MainWindowStates.OFFLINE_STATE
        Platform.runLater({onUpdateState(applicationState)})
    }

    fun onDigitalTwinSelected(name:String){
        if(isDigitalTwinSelected(name))
        {
            for(key in digitalTwins.keys){
                if(digitalTwins[key]?.isNotEmpty() == true){
                    for(digitalTwin in digitalTwins[key]!!){
                        if(digitalTwin.name==name){
                            fxController.setNameOfDigitalTwin(name)
                            selectedDT = digitalTwin
                        }
                    }
                }
            }
        }
        if(selectedDT!=null){
            for(project in projects){
                if(selectedDT!!.parentProject==project.id){
                    downloadAllProjectData(projectId = selectedDT!!.parentProject!!, commitId = selectedDT!!.commitId)
//                    val branch = project.defaultBranch?.let { AgilaRepository.getBranchById(project.id, it.id) }
//                    branch?.head?.id?.let { downloadAllProjectData(project.id, it) }
                }
            }



            val parser = selectedDT?.connectedModels?.let { DigitalTwinParser(elements, it) }
            if (parser != null) {
                parser.filterToSelectedElements()
                parser.parseElements()
                parser.reloadDocumentsIfNeccesary()

                Platform.runLater({
                    fxController.redecorateDigitalTwinStructure()
                })
            }
        }
    }

    fun isDigitalTwinSelected(name:String):Boolean {
        if((name=="")||(name=="Projects"))
            return false

        for(project in projects)
            if(name == project.name)
                return false

        return true
    }

    fun downloadAllProjectData(projectId:UUID,commitId:UUID) {
        elements = AgilaRepository.getAllElements(projectId,commitId)
    }

    fun setFxSessionController(controller:FXController){
        fxController = controller
    }

    fun startSimulation(){
        if(selectedDT==null)
            return

    }

    fun propagateValueToMainWindow(topic:String, value:String){
        val newValue=value.removePrefix("{").removeSuffix("}")

    }

}
