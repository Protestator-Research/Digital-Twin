package SysMDRestImport.entities


import com.fasterxml.jackson.annotation.JsonIgnore
import com.fasterxml.jackson.annotation.JsonIgnoreProperties
import com.fasterxml.jackson.annotation.JsonInclude
import com.fasterxml.jackson.annotation.JsonProperty
//import com.github.tukcps.sysmd.entities.*
//import com.github.tukcps.sysmd.rest.ElementDAO
import BaseEntities.*
import SysMDRestImport.ElementDAO
import java.util.*


//@Document
open class Element{
    init {

    }

    constructor() {

    }

    constructor(id:UUID,
                type: String,
                name:String? = null,
                shortName:String? = null,
                ownedElements:MutableList<UUID> = mutableListOf(),
                owner:UUID? = null,
                direction:Feature.FeatureDirectionKind? = null,
                importedMemberName:String? = null,
                importedNamespace: String? = null,
                valueSpecs: MutableList<Any?>? = mutableListOf(),
                unitSpec:String? = null,
                dependency: String? = null,
                language: String? = null,
                body: String? = null,
                source: MutableList<String> = mutableListOf(),
                target: MutableList<String> = mutableListOf()
    )
    {
        this.id=id
        this.type = type
        this.name = name
        this.shortName = shortName
        this.ownedElements = ownedElements
        this.owner = owner
        this.direction = direction
        this.importedNamespace = importedNamespace
        this.importedMemberName = importedMemberName
        //TODO To Fix
//        this.valueSpecs = valueSpecs
        this.unitSpec = unitSpec
        this.dependency = dependency
        this.language = language
        this.body = body
        this.source = source
        this.target = target
    }
    @JsonProperty("@id")
    var id: UUID = UUID.randomUUID()
    @JsonProperty("@type")
    var type: String = "Element"       // mandatory type of the metamodel as annotation

    @JsonProperty("name")
    var name: String? = null
    @JsonProperty("shortName")
    @JsonInclude(JsonInclude.Include.NON_NULL)
    @JsonIgnoreProperties(ignoreUnknown = true)
    var shortName: String? = null

    @JsonProperty("ownedElements")
    var ownedElements: MutableList<UUID> = mutableListOf()   //  --> Via own endpoint!
    @JsonProperty("owner")
    @JsonInclude(JsonInclude.Include.NON_NULL)
    @JsonIgnoreProperties(ignoreUnknown = true)
    var owner: UUID? = null

    @JsonProperty("direction")
    @JsonInclude(JsonInclude.Include.NON_NULL)
    @JsonIgnoreProperties(ignoreUnknown = true)
    var direction: Feature.FeatureDirectionKind? = null

    @JsonProperty("importedMemberName")
    var importedMemberName: String? = null
    @JsonProperty("importedNamespace")
    var importedNamespace: String? = null

    // For type Property; move to owned elements
    @JsonProperty("valueSpec")
    @JsonInclude(JsonInclude.Include.NON_NULL)
    @JsonIgnoreProperties(ignoreUnknown = true)
    var valueSpecs:String?=null
    @JsonProperty("unitSpec")
    @JsonInclude(JsonInclude.Include.NON_NULL)
    @JsonIgnoreProperties(ignoreUnknown = true)
    var unitSpec:  String? = null  // Owned element of type Value/Unit?
    @JsonProperty("dependency")
    var dependency: String? = null // Owned element of type ExpressionValue?

    // For type AnnotationElement:
    @JsonProperty("language")
    var language: String? = null  // language, e.g. SysMD, SysML
    @JsonProperty("body")
    var body: String? = null      // The code in e.g. SysMD or SysML v2 textual

    // For Relationship and subtypes thereof:
    @JsonProperty("source")
    var source: MutableList<String> = mutableListOf()     // list of id
    @JsonProperty("target")
    var target: MutableList<String> = mutableListOf()     // list of if


    @JsonIgnore
    @JsonProperty("valueStr")
    var valueStr:String?=null
}

fun String?.toUUIDorNull(): UUID? =
    if (this == "null" || this == null) null
    else UUID.fromString(this)


fun ElementDAO.toElementRepositoryDAO() = Element(
    id = elementId,
    type = type?:"null",
    name = name,
    shortName = shortName,
    ownedElements = ownedElements?: mutableListOf(),
    owner = owner,
    direction = direction,
    importedMemberName = importedMemberName,
    importedNamespace = importedNamespace,
    valueSpecs = valueSpecs,
    unitSpec = unitSpec,
    dependency = dependency,
    language = language,
    body = body,
//    source = source?.map { it?.toString()?:"null" }?.toMutableList()?: mutableListOf(),
//    target = target?.map { it?.toString()?:"null" }?.toMutableList()?: mutableListOf()
)


fun Element.toElementDAO() = if (valueSpecs!=null) ElementDAO(
    elementId = id,
    type = type,
    name = name,
    shortName = shortName,
    ownedElements = ownedElements,
    owner = owner,
    direction = direction,
    importedMemberName = importedMemberName,
    importedNamespace = importedNamespace,
    //TODO to Fix
//    valueSpecs = valueSpecs!!,
    unitSpec = unitSpec,
    dependency = dependency,
    language = language,
    body = body,
//    source = source.map { if ((it != "null")&&(it!=null)) UUID.fromString(it) else null }.toMutableList(),
//    target = target.map { if ((it != "null")&&(it!=null)) UUID.fromString(it) else null }.toMutableList()
) else
    ElementDAO(
        elementId = id,
        type = type,
        name = name,
        shortName = shortName,
        ownedElements = ownedElements,
        owner = owner,
        direction = direction,
        importedMemberName = importedMemberName,
        importedNamespace = importedNamespace,
        unitSpec = unitSpec,
        dependency = dependency,
        language = language,
        body = body)

fun Element.toElementSnapshot() = if(valueSpecs!=null) ElementSnapshot(
    elementId = id,
    type = type,
    name = name,
    shortName = shortName,
    ownedElements = ownedElements,
    owner = owner,
    direction = direction,
    importedMemberName = importedMemberName,
    importedNamespace = importedNamespace,
    //TODO To Fix
//    valueSpecs = valueSpecs !!,
    unitSpec = unitSpec,
    dependency = dependency,
    language = language,
    body = body,
    source = source.map { it }.toMutableList(),
    target = target.map { it }.toMutableList()
) else ElementSnapshot(
    elementId = id,
    type = type,
    name = name,
    shortName = shortName,
    ownedElements = ownedElements,
    owner = owner,
    direction = direction,
    importedMemberName = importedMemberName,
    importedNamespace = importedNamespace,
    unitSpec = unitSpec,
    dependency = dependency,
    language = language,
    body = body,
    source = source.map { it }.toMutableList(),
    target = target.map { it }.toMutableList()
)


fun ElementSnapshot.toElementDAO() = ElementDAO(
    elementId = elementId,
    type = type,
    name = name,
    shortName = shortName,
    ownedElements = ownedElements,
    owner = owner,
    direction = direction,
    importedMemberName = importedMemberName,
    importedNamespace = importedNamespace,
    valueSpecs = valueSpecs,
    unitSpec = unitSpec,
    dependency = dependency,
    language = language,
    body = body,
    source = source.map { if (it != "null") UUID.fromString(it) else null }.toMutableList(),
    target = target.map { if (it != "null") UUID.fromString(it) else null }.toMutableList()
)

