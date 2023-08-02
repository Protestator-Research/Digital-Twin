package Parser

import BaseEntities.*
import BaseEntities.implementation.*
import BaseEntities.Annotation
import ImportedSysMDExceptions.SemanticError
import ImportedSysMDServices.AgilaSession
import ImportedSysMDServices.reportInfo
import ImportedSysMDServices.resolveName
import SysMDCopyAst.AstNode
import SysMDCopyAst.functions.AstAbs
import SysMDCopyAst.functions.AstCeil
import SysMDCopyAst.functions.AstDate
import SysMDCopyAst.functions.AstDateTime
import SysMDCopyAst.functions.AstExp
import SysMDCopyAst.functions.AstFloor
import SysMDCopyAst.functions.AstFunction
import SysMDCopyAst.functions.AstInteger
import SysMDCopyAst.functions.AstIntersect
import SysMDCopyAst.functions.AstIte
import SysMDCopyAst.functions.AstLinear
import SysMDCopyAst.functions.AstLn
import SysMDCopyAst.functions.AstMax
import SysMDCopyAst.functions.AstMin
import SysMDCopyAst.functions.AstMonth
import SysMDCopyAst.functions.AstNormalizeVector
import SysMDCopyAst.functions.AstNot
import SysMDCopyAst.functions.AstPower
import SysMDCopyAst.functions.AstPower2
import SysMDCopyAst.functions.AstProductHasA
import SysMDCopyAst.functions.AstProductIsA
import SysMDCopyAst.functions.AstReal
import SysMDCopyAst.functions.AstSqr
import SysMDCopyAst.functions.AstSqrt
import SysMDCopyAst.functions.AstStep
import SysMDCopyAst.functions.AstSumHasA
import SysMDCopyAst.functions.AstSumI
import SysMDCopyAst.functions.AstSumIsA
import SysMDCopyAst.functions.AstToReal
import SysMDCopyAst.functions.AstVectorAngle
import SysMDCopyAst.functions.AstYear

/**
 * This class extends the AgilaModel interface with semantic actions that each
 * implement the actions required by a production rule of the SysMD2 language.
 */
class Semantics(
    val model: AgilaSession,
    var generatedElementsAnnotation: Annotation? = null,
) : AgilaSession by model {
    /**
     * A stack in which we save the context in which the parser is currently working.
     * Each entry is an id string.
     */
    val scope: ArrayList<Namespace> = arrayListOf(global)

    /**
     * The owning namespace of the currently processed statement or element.
     */
    val namespace get() = scope.last()

    /**
     * Semantic action that starts a new scope.
     * It adds a new element on the stack of scopes. This element will be used in the next find operations
     * as a starting point for search, unless scope is changed by entering a new scope or leaving the scope.
     */
    fun enterScope(namespace: Namespace) {
        this.scope.add(namespace)
    }

    /**
     * Leaves the current scope by removing one element from the stack of scopes. Then, the last scope
     * becomes active.
     */
    fun leaveScope() {
        this.scope.removeLast()
    }

    /**
     * Definition of a new package.
     * If the semantic actions object was build with a generatedElementsAnnotation not null,
     * the created package will be added to it as well.
     * e.g. X isA Package.
     */
    fun declarePackage(identification: Identification): PackageImplementation {
        val pkg = PackageImplementation(name=identification.name, shortName = identification.shortName)
        val found = namespace.getOwnedElement(identification.name?:"", identification.shortName)
        return if (found == null) {
            model.create(pkg, namespace)
            generatedElementsAnnotation?.addAnnotatedElement(namespace.qualifiedName+"::"+pkg.effectiveName)
            pkg
        } else {
            reportInfo(namespace, "Package ${identification.name} exists; keeping existing.")
            found as PackageImplementation
        }
    }

    /**
     * For call from SysML parser. Defines or instantiates a new part.
     * If def is true, we create a new class.
     * If def is true, we create an instance in another element (via has-a relation).
     */
    fun definePart(def: Boolean, part: Identification, clazz: QualifiedName?): Type {
        if (def) {      // Definition
            val element = ClassImplementation(name = part.name, shortName = part.shortName)
            val created = create(element, namespace)
            val foundClass = if (clazz != null ) namespace.resolveName<Type>(clazz) else any
            val specialization = SpecializationImplementation(subclass = Identity(ref=element), superclass=Identity(
                str =clazz?:"Any", ref = foundClass, id=foundClass?.elementId))
            create(specialization, created)
            return created
        } else {        // Instantiation in part or design
            val feature = FeatureImplementation(name = part.name, shortName = part.shortName)
            val created = create(feature, namespace)
            val type = if (clazz != null) namespace.resolveName<Type>(clazz) else any
            val featureTyping = FeatureTypingImplementation(
                typedFeature = Identity(ref=feature),
                type = Identity(ref=type!!)
            )
            create(featureTyping, created)
            return feature
        }
    }

    fun partUsage(part: Identification, typeName: QualifiedName?): Feature {
        val feature = FeatureImplementation(name = part.name, shortName = part.shortName)
        val created = create(feature, namespace)
        val type = if (typeName != null) namespace.resolveName<Type>(typeName) else any
        val featureTyping = FeatureTypingImplementation(
            typedFeature = Identity(ref=feature),
            type = Identity(ref=type!!)
        )
        create(featureTyping, created)
        return feature
    }

    /**
     * Saves a feature into the model, but still as a temporary artifact
     * as we do not have yet identified its owner.
     */
    fun hasFeature(value: ValueFeature, owner: QualifiedName) {
        addUnownedElement(value, owner)
        generatedElementsAnnotation?.addAnnotatedElement(owner+"::"+value.name)
    }

    /**
     * Handles the specification of a feature.
     * Corresponding SysMD - statement is " name hasA name: number className".
     *  @param name the name of the feature
     *  @param multiplicity the multiplicity of the feature
     *  @param owner the owner of the feature
     *  @param className the class of which the feature instances have to be
     *  @return The created feature.
     */
    fun hasFeature(owner: Identification, name: String, multiplicity: Multiplicity, className: QualifiedName): Feature {
        // if classId is referring to a class, we simply declare the occurrence of this class.
        val feature = ValueFeatureImplementation(name=name)
        addUnownedElement(feature, owner.toName())
        addUnownedElement(multiplicity, owner.toName()+"::$name")
        addUnownedElement(SpecializationImplementation(feature, className),owner.toName()+"::$name")
        generatedElementsAnnotation?.addAnnotatedElement(owner.toName()+"::"+feature.name)
        return feature
    }


    /**
     * The semantic action for production 'subject "imports"  ObjectList'.
     * Each of the strings in the objList is added to the import list of the subject.
     */
    fun imports(subject: QualifiedName, objList: List<QualifiedName>) {
        objList.forEach {
            if (subject == "Document")
                dependsOn(subject, it)
            else
                model.addUnownedElement(
                    NamespaceImportImplementation(
                        owner=Identity(subject),
                        importedNamespace = Identity(objList.first())
                    )
                )
        }
    }

    /**
     * The semantic action for production: subjectName "dependsOn" projectName.
     * Loads a project from the repository.
     * @param projectName name of the project; default-branch head will be used.
     */
    @Suppress("unused_parameter")
    fun dependsOn(subject: QualifiedName, projectName: String) {
        loadProjectFromRepository(projectName)
    }


    /**
     * @method mkFuncCall
     * @detail This method builds the expression tree for a function call.
     * It sets the parameters of a function into the local symbol table.
     * @param function name of the function to be called
     * @param param of the function as an ArrayList of AstNode
     * @return The Expression tree
     * @throws SemanticError
     */
    fun mkFuncCall(function: QualifiedName, param: ArrayList<AstNode>): AstFunction {
        when (function) {
            "ITE" -> return AstIte(this, param)
            "sum_i" -> return AstSumI(namespace, this, param)
            "SUM" -> return AstSumHasA(this,namespace, param, transitive = true)
            "sumOverParts" -> return AstSumHasA(this,namespace, param, transitive = true)
            "sumOverSubclasses" -> return AstSumIsA(this,namespace, param, transitive = true)
            "productOverParts" -> return AstProductHasA(this,namespace, param, transitive = true)
            "productOverSubclasses" -> return AstProductIsA(this,namespace, param, transitive = true)
            "sumOverPartsNotTransitive" -> return AstSumHasA(this,namespace, param, transitive = false)
            "sumOverSubclassesNotTransitive" -> return AstSumIsA(this,namespace, param, transitive = false)
            "productOverPartsNotTransitive" -> return AstProductHasA(this,namespace, param, transitive = false)
            "productOverSubclassesNotTransitive" -> return AstProductIsA(this,namespace, param, transitive = false)
            "not" -> return AstNot(this, param)
            "ln" -> return AstLn(this, param)
            "exp" -> return AstExp(this, param)
            "sqr" -> return AstSqr(this, param)
            "sqrt" -> return AstSqrt(this, param)
            "ceil" -> return AstCeil(this, param)
            "floor" -> return AstFloor(this, param)
            "power2" -> return AstPower2(this, param)
            "pow2" -> return AstPower2(this, param)
            "powerb" -> return AstPower(this, param)
            "power" -> return AstPower(this, param)
            "powb" -> return AstPower(this, param)
            "pow" -> return AstPower(this, param)
            "toReal" -> return AstToReal(this, param)
            "DateTime" -> return AstDateTime(this, param)
            "Date" -> return AstDate(this, param)
            "Month" -> return AstMonth(this, param)
            "Year" -> return AstYear(this, param)
            "max" -> return AstMax(this, param)
            "min" -> return AstMin(this, param)
            "abs" -> return AstAbs(this, param)
            "intersect" -> return AstIntersect(this, param)
            // "i"     -> return AstRange(this, param)
            // "bySubclasses" -> return AstBySubclasses(this, param)
            "linear" -> return AstLinear(this, param)
            "step" -> return AstStep(this,param)
            "Real" -> return AstReal(this, param)
            "Integer" -> return AstInteger(this, param)
            "norm" -> return AstNormalizeVector(this,param)
            "angle" -> return AstVectorAngle(this,param)
            else -> throw SemanticError("identifier $function not a known function")
        }
    }

    /**
     * Gets two properties and an element that is a Connector.
     */
    @Suppress("UNUSED")
    fun connects(from: QualifiedName, to: QualifiedName, subject: Identification, namespace: Namespace) {
        val connector = namespace.resolveName<Element>(subject.toName()) as Namespace?
            ?: throw SemanticError("Subject not found in connects relation: $subject")

        connector.importNamespace(model, from)
        connector.importNamespace(model, to)

        val connection = ConnectorImplementation(
            name = subject.name,
            shortName = subject.shortName,
            from = mutableListOf(Identity(str = from)),
            to = mutableListOf(Identity(str = to)),
        )
        create(connection, namespace)
    }

    @Suppress("UNUSED")
    fun associates(from: QualifiedName, to: QualifiedName, subject: Identification, namespace: Namespace) {
        val connector = namespace.resolveName<Element>(subject.toName()) as Namespace?
            ?: throw SemanticError("Subject not found in connects relation: $subject")

        connector.importNamespace(model, from)
        connector.importNamespace(model, to)

        val association = AssociationImplementation(
            identification = subject,
            sources = mutableListOf(Identity(str = from)),
            targets = mutableListOf(Identity(str = to))
        )
        create(association, namespace)
    }


    /**
     * Semantic action for a general relationship-triple of the form
     * 'a relationship b, c, ...' where a, b, c, ... are elements of arbitrary class, and
     * relationship must be of class relationship.
     * The action creates a Relationship element between a (source) and b, c, ... (targets) elements.
     * @param source name of the source element
     * @param relationshipQualifiedName Name of the relationship
     * @param targets List of names of the target elements
     * @param owner currently active namespace
     * @param annotation Annotation relationship in which relationships code - generated elements are maintained.
     */
    fun relationship(
        source: QualifiedName,
        relationshipQualifiedName: QualifiedName,
        targets: MutableList<QualifiedName>,
        annotation: Annotation?,
        owner: QualifiedName
    ) {
        // Special case: Display ... for notebook.
        if (source == "Display" && annotation != null)
            return handleDisplayRelationship(relationshipQualifiedName, targets, annotation)

        // Create the relationship
        val relationship = AssociationImplementation(
            name = null, shortName = null,
            sources = mutableListOf(Identity(source)),
            targets = targets.toIdentityList()
        )
        // Add it to the model
         addUnownedElement(relationship, owner)
        // Add annotation between textual representation and generated relationship
    }


    private fun handleDisplayRelationship(relationshipQualifiedName: QualifiedName, targets: List<QualifiedName>, annotation: Annotation) {
        require(relationshipQualifiedName == "shows")
        val relationship = RelationshipImplementation(
            name = "shows",
            source = mutableListOf(Identity("Display")),
            target = targets.toIdentityList()
        )
        create(relationship, annotation)
        // Add annotation between textual representation and generated relationship
        // annotation.addAnnotatedElement(anno)
    }

}
