package Parser

import BaseEntities.Element
import BaseEntities.TextualRepresentation
import BaseEntities.ValueFeature
import BaseEntities.implementation.AnnotatingElementImplementation
import BaseEntities.implementation.TextualRepresentationImplementation
import ImportedSysMDExceptions.SemanticError
import ImportedSysMDExceptions.SysMDError
import ImportedSysMDServices.*
import ImportedSysMDServices.AgilaSessionMgr.runInSession
import com.github.tukcps.jaadd.AADD
import com.github.tukcps.jaadd.BDD
import com.github.tukcps.jaadd.IDD

import SysMDCopyAst.AstRoot
import SysMDRestImport.ElementDAO

import java.io.File


/**
 * Loads a SysMD file from an input string.
 * @param input A SysMD language sting; pure SysMD without interwoven MD.
 * @param catchExceptions by default, exceptions are caught and errors are reported via status; this can
 * @param textualRepresentationName name of textual representation that will be created in the KerML model
 * be turned off by setting catchExceptions to false.
 */
fun AgilaSession.loadSysMD(
    input: String,
    catchExceptions: Boolean = settings.catchExceptions,
    textualRepresentationName: String? = "test.md"
){
    settings.catchExceptions = catchExceptions
    val rep = TextualRepresentationImplementation(name=textualRepresentationName, language = "SysMD", body=input)
    SysMDParser(this, create(rep, global)).parseSysMD()

    try {
        if (settings.initialize) initialize()
    }  catch (exception: SysMDError) {
        report(exception)
        if (!settings.catchExceptions)
            throw exception
    }
}


/**
 * Parses a textual representation and adds results to the model of the session/service.
 * The model will be (re-)initialized; uid and qualified names are replaced by references thereby.
 */
fun TextualRepresentation.compile() {
    SysMDParser(model!!, this).parseSysMD()
}


/**
 * Loads a complete project from the repository into the session service.
 * The project is first searched in the repository; if not there, in
 * the local files.
 * The method imports *all* elements into the session.
 * @param projectName Name of project that will be imported.
 * @param initialize whether to also call initialize.
 */
fun AgilaSession.loadProjectFromRepository(projectName: String, initialize: Boolean = true) {
    val newElements = AgilaSessionMgr.repository.getAllElements(projectName)
    if (newElements.isNotEmpty())
        import(newElements)
    // identify Qualified Names, etc.
    if (settings.initialize && initialize)
        initialize()
}

/**
 * Loads a document from the repository, but limits the import to the
 * documents persisted as annotating elements.
 */
fun AgilaSession.loadDocumentSourceOnly(projectName: String) {
    val newElements = AgilaSessionMgr.repository.getAllElements(projectName)
    val documents = newElements.filter {
        it.type == "AnnotatingElement" && it.owner == null
    }
    val textualRepresentations = newElements.filter {
        it.type == "TextualRepresentation"
    }
    val generatedElements = newElements.filter {
        it.name == "Generated elements"
    }
    generatedElements.forEach { it.ownedElements?.clear() }
    import(documents+textualRepresentations+generatedElements)
}


/**
 * This method calls the parser with a given property, from which the dependency string is
 * parsed and the ast is created.
 */
fun ValueFeature.parseDependency() {

    try {
        if (model != null) {
            val parserSysMD = SysMDParser(model!!, TextualRepresentationImplementation(language = "SysMD", body = dependency), dependency)

            // The parsing itself, can throw exceptions that are caught optionally below.
            if (model!!.repo.numberType == null)
                model!!.report(this, "Could not resolve ScalarValues::Number -- include ScalarValues")
            if (ofClass?.ref == null)
                model!!.report(this, "Could not resolve Type '${superclass?.str}' of feature $qualifiedName")
            if (ofClass!!.ref?.isSubclassOf(model!!.repo.numberType) != true)
                return

            if (dependency.isNotBlank()) {
                // set the scope to the element to which the property belongs.
                owner.ref?:throw SemanticError("No owner of ${effectiveName}; initialize identifications before using services.")
                parserSysMD.semantics.setScope(owningNamespace!!)
                ast = AstRoot(model!!, this, parserSysMD.parseExpression())
                // Initialize internal AST nodes, starting from leaves
                ast?.runDepthFirst { initialize() }
                when {
                    ofClass?.ref?.isSubclassOf(model!!.repo.realType) == true -> {
                        if (ast!!.upQuantity.values[0] !is AADD)
                            throw SemanticError("Expecting dependency of type Real")
                    }

                    ofClass?.ref?.isSubclassOf(model!!.repo.booleanType) == true -> {
                        if (ast!!.upQuantity.values[0] !is BDD)
                            throw SemanticError("Expecting dependency of type Boolean")
                    }

                    ofClass?.ref?.isSubclassOf(model!!.repo.integerType) == true -> {
                        if (ast!!.upQuantity.values[0] !is IDD)
                            throw SemanticError("Expecting dependency of type Integer")
                    }
                }
            }
        }
     } catch (exception: Exception) {
        ast = null
        model?.report(this, "Error parsing expression '$dependency' of $qualifiedName; problem: ${exception.message}", exception)
    }
}


/**
 * Imports a SysMD file from the resources' folder. These are usually the pre-defined libraries.
 * The file may be in .md format, and will first be pre-processed by an MD parser, and the then
 * code cells will be processed by the SysMD parser.
 * @param fileName The filename of the file in SysMD-resources/projects
 * @param initialize Whether to do also initialization and 1st propagation or only name resolution
 */
fun AgilaSession.loadSysMDFromResources(fileName: String, initialize: Boolean = true) {

    /**
     * Recursively walk through the owned elements of type TextualRepresentation and language SysMD.
     * Compile them, considering imports (!!!).
     */
    fun compileRecursively(element: Element) {
        element.ownedElements.forEach {
            val ownedElement = getElement(it.id!!)
            if (ownedElement is TextualRepresentation && ownedElement.language.startsWith("SysMD")) {
                ownedElement.compile()
            }
            compileRecursively(ownedElement) // if there are textual representations embedded in text. rep?
        }
    }

    // Search project file in resources or in SysMD-Home; just for default-libraries.
//    val inputStreamSysMD = javaClass.getResourceAsStream("/projects/$fileName")?:
//        if (File(Init.folder.canonicalPath, fileName).exists()) File(Init.folder.canonicalPath, fileName).inputStream()
//        else if (File("/"+fileName).exists()) File("/"+fileName).inputStream()
//        else File(Init.folder.canonicalPath +"/" + fileName) .inputStream()
//     first read the MD into the memory; creates only annotations with textual models and description annotations
//
//    val inputString = inputStreamSysMD.bufferedReader().use { it.readText() }

    val fileAnnotation = createOrReplace(AnnotatingElementImplementation(name=fileName, body = ""), global)
//    importMD(inputString, fileAnnotation)
    compileRecursively(fileAnnotation)
    if (initialize && settings.initialize)
        initialize()
    else
        resolveNames()
}


fun getImportFromFile(file: File): List<ElementDAO> {

    var elementsAsDAO: List<ElementDAO> = listOf()

    runInSession {

        /**
         * Recursively walk through the owned elements of type TextualRepresentation and language SysMD.
         * Compile them, considering imports (!!!).
         */
        fun compileRecursively(element: Element) {
            element.ownedElements.forEach {
                val ownedElement = getElement(it.id!!)
                if (ownedElement is TextualRepresentation && ownedElement.language.startsWith("SysMD")) {
                    ownedElement.compile()
                }
                compileRecursively(ownedElement) // if there are textual representations embedded in text. rep?
            }
        }

        val inputString = file.bufferedReader().use { it.readText() }

        val fileAnnotation =
            createOrReplace(AnnotatingElementImplementation(name = file.nameWithoutExtension, body = ""), global)
        importMD(inputString, fileAnnotation)
        compileRecursively(fileAnnotation)
        elementsAsDAO = export()
    }
    return elementsAsDAO
}