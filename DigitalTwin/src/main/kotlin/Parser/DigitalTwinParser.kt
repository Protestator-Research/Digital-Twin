package Parser

import DTSessionManager
import com.github.tukcps.sysmd.entities.TextualRepresentation
import com.github.tukcps.sysmd.parser.ParserSysMD
import com.github.tukcps.sysmd.rest.ElementDAO
import com.github.tukcps.sysmd.rest.toElement
import java.util.UUID

class DigitalTwinParser(
    var elements:MutableList<ElementDAO>,
    var dtSelectedElements:MutableList<UUID>
) {
    fun filterToSelectedElements(){
        for(element in elements){
            if(dtSelectedElements.contains(element.elementId))
            {
                elementsToParse.add(element)
            }
        }
        println(elementsToParse.size)
    }

    fun parseElements(){
        var index = 0
        DTSessionManager.dtSession.import(elementsToParse)
        for(element in elementsToParse)
        {
            println("index: " + index)
            var textualRepresentation = element.toElement() as TextualRepresentation
            var parser = ParserSysMD(model = DTSessionManager.dtSession, textualRepresentation = textualRepresentation)
            parser.parseSysMD()
            index++
        }
        DTSessionManager.dtSession.remodelRepresentation()
    }

    fun reloadDocumentsIfNeccesary() {

    }

    private var elementsToParse:MutableList<ElementDAO> = mutableListOf()
}