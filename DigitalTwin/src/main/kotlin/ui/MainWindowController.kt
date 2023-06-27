package ui

import com.github.tukcps.sysmd.rest.AgilaRepository
import com.github.tukcps.sysmd.rest.Rest
import com.github.tukcps.sysmd.rest.entities.DigitalTwin
import com.github.tukcps.sysmd.rest.entities.Project
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

    init {
        projects = AgilaRepository.getProjects()
        getDigitalTwinsOfProjects()
        applicationState = if(projects.isNotEmpty()) MainWindowStates.ONLINE_STATE else MainWindowStates.OFFLINE_STATE
    }
    fun getDigitalTwinsOfProjects(){
        for (project in projects)
        {
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
}
