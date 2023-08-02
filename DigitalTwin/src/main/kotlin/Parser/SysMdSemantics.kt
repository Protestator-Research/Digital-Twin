package Parser

import BaseEntities.*
import BaseEntities.Annotation
import BaseEntities.implementation.*
import ImportedSysMDExceptions.SemanticError
import ImportedSysMDServices.AgilaSession
import ImportedSysMDServices.report
import com.github.tukcps.jaadd.values.IntegerRange
import SysMDCopyAst.AstNode
import SysMDCopyAst.functions.AstAbs
import SysMDCopyAst.functions.AstByImplements
import SysMDCopyAst.functions.AstBySubclasses
import SysMDCopyAst.functions.AstCeil
import SysMDCopyAst.functions.AstDate
import SysMDCopyAst.functions.AstDateTime
import SysMDCopyAst.functions.AstEnum
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
class SysMdSemantics(
    val model: AgilaSession,
    var generatedElementsAnnotation: Annotation? = null,
    var namespacePrefix: QualifiedName? = null,
    var textualRepresentation: TextualRepresentation
){
    /**  Marker only for the generation of AST! */
    var namespace: Namespace = model.global

    /**
     * Sets a marker for the last created Namespace; it must be explicitly set here,
     * and can be used i.e. for creating AST that need references to namespace.
     */
    fun setScope(namespace: Namespace) {
        this.namespace = namespace
    }

    /**
     * Adds the namespace prefix of a parse run to the name given as parameter.
     * @param name
     * @return namespacePrefix + name, considering "::" and formatting.
     */
    private fun toEffectiveName(name: QualifiedName): QualifiedName {
        return when {
            namespacePrefix == "Global" && name.isNotEmpty() -> name
            namespacePrefix != null && name.isNotEmpty() -> "$namespacePrefix::$name"
            namespacePrefix != null -> namespacePrefix!!
            else -> name
        }
    }


    /**
     * Definition of a new package.
     * If the semantic actions object was build with a generatedElementsAnnotation not null,
     * the created package will be added to it as well.
     * e.g. X isA Package.
     */
    fun hasAPackage(owner: QualifiedName, identification: Identification): PackageImplementation {
        val pkg = PackageImplementation(name=identification.name, shortName = identification.shortName)
        model.addUnownedElement(pkg, toEffectiveName(owner))
        generatedElementsAnnotation?.addAnnotatedElement(toEffectiveName(owner)+"::"+identification.toName())
        return pkg
    }


    /**
     * Handles the specification of a feature with a value and an expression.
     * The corresponding SysMD - statement is " name hasA name: number className".
     *  @param identification the name of the feature
     *  @param multiplicity the multiplicity of the feature
     *  @param owner the owner of the feature
     *  @param className the class of which the feature instances have to be
     *  @return The created feature.
     */
    fun hasValueFeature(owner: QualifiedName, identification: Identification, multiplicity: Multiplicity, className: QualifiedName, direction: Feature.FeatureDirectionKind): ValueFeature {

        // if classId is referring to a class, we simply declare the occurrence of this class.
        val feature = ValueFeatureImplementation(
            name = identification.name,
            shortName = identification.shortName,
            direction = direction
        ).also {
            it.textualRepresentation = textualRepresentation
        }
        // remember namespace for following AST elements that need it for constructor
        setScope(feature)

        val specialization = FeatureTypingImplementation(
            owner = Identity(ref=feature),
            typedFeature = Identity(ref=feature),
            type = Identity(str=className)
        )
        model.addUnownedElement(feature, toEffectiveName(owner))
        model.addUnownedElement(multiplicity, toEffectiveName(owner + "::${identification.toName()}"))
        model.addUnownedElement(specialization, toEffectiveName(owner + "::${identification.toName()}"))
        generatedElementsAnnotation?.addAnnotatedElement(toEffectiveName(owner)+"::"+feature.name)
        return feature
    }

    fun hasComponentFeature(owner: QualifiedName, identification: Identification, multiplicity: Multiplicity, className: QualifiedName, usages: List<QualifiedName>) {
        val feature = FeatureImplementation(name = identification.name, shortName = identification.shortName)
        val specialization = FeatureTypingImplementation(
            owner = Identity(ref=feature),
            typedFeature = Identity(ref=feature),
            type = Identity(str=className)
        )
        model.addUnownedElement(feature, toEffectiveName(owner))
        model.addUnownedElement(multiplicity, toEffectiveName(owner + "::${identification.toName()}"))
        model.addUnownedElement(specialization, toEffectiveName(owner + "::${identification.toName()}"))
        generatedElementsAnnotation?.addAnnotatedElement(toEffectiveName(owner)+"::"+feature.name)
    }

    fun hasRelationshipFeature(owner: QualifiedName, identification: Identification?, sources: List<QualifiedName>, relationship: QualifiedName, targets: List<QualifiedName>) {
        val relation = AssociationImplementation(
            name = identification?.name,
            shortName = identification?.shortName,
            sources = sources.toIdentityList(),
            targets = targets.toIdentityList(),
        )

        // Special case: Display ... for notebook.
        if (generatedElementsAnnotation != null) {
            val display = sources.find { it == "Display" }
            if (display != null)
                return handleDisplayRelationship(relationship, targets, generatedElementsAnnotation!!)
        }


        val specialization = SpecializationImplementation(
            owner = Identity(str = null, ref = relation, id = relation.elementId),
            subclass = Identity(ref=relation),
            superclass = Identity(str=relationship)
        )
        model.addUnownedElement(relation, toEffectiveName(owner))
        model.addUnownedElement(specialization, Identity(relation))
        generatedElementsAnnotation?.addAnnotatedElement(toEffectiveName(owner)+"::"+relation.effectiveName)
    }

    /**
     * Semantic action for the declaration of a classifier.
     * @param identification The name of the Classifiable
     * @param superclass The qualified name of the superclass of the Classifiable
     * @param owner The qualified name of the owner of the Classifiable
     */
    fun isAClass(identification: Identification, superclass: QualifiedName, owner: QualifiedName) {
        // Call constructor depending on type
        val name = identification.name?.lastName()
        val shortName = identification.shortName?.lastName()
        val namespace = (toEffectiveName(owner)+"::"+identification.toName().dropLastName()).removeSuffix("::")

        val element = ClassImplementation(name=name, shortName = shortName)
        model.addUnownedElement(element, namespace)
        val specialization = SpecializationImplementation(subclass = Identity(ref=element), superclass= Identity(str=superclass))
        model.addUnownedElement(specialization, namespace+"::${element.effectiveName}")

        generatedElementsAnnotation?.addAnnotatedElement(toEffectiveName(owner)+"::"+element.effectiveName)

    }

    /**
     * Semantic action for the definition of an association type.
     * @param identification The name of the Classifiable
     * @param superclass The qualified name of the superclass of the Classifiable
     * @param sourcessType A type that specifies class of the sources.
     * @param targetsType A type that specifies class of the targets.
     * @param owner The qualified name of the owner of the Classifiable
     * an Association will be created; otherwise, a Classifier
     */
    fun isARelationship(
        relationshipKind: Scanner.Definitions.Token.Kind,
        identification: Identification,
        superclass: QualifiedName?,
        sourcesType: Identity<Feature>?,
        sourcesMult: IntegerRange?,
        targetsType: Identity<Feature>?,
        targetsMult: IntegerRange?,
        owner: QualifiedName
    ) {
        val targets = if (targetsType?.str=="ScalarValues::Real")
                ValueFeatureImplementation(name="to", isEnd = true)
            else
                FeatureImplementation(name = "to", isEnd = true)
        val sources = if (sourcesType?.str=="ScalarValues::Real")
                ValueFeatureImplementation(name="from", isEnd = true)
            else
                FeatureImplementation(name = "from", isEnd = true)

        val sourcesOptName = mutableListOf<Identity<Element>>()
        if (sourcesType != null) sourcesOptName.add(Identity<Feature>(id=sources.elementId, ref=sources, str = "from"))
        val targetsOptName = mutableListOf<Identity<Element>>()
        if (targetsType != null) targetsOptName.add(Identity<Feature>(id=targets.elementId, ref=targets, str = "to"))

        // The respective KerML element is either an Association (=typed relationship), or a
        // Connector (=relationship and feature)
        val association = if (relationshipKind == Scanner.Definitions.Token.Kind.RELATIONSHIP)
            AssociationImplementation(
                name = identification.name,
                shortName = identification.shortName,
                sources = sourcesOptName,
                targets = targetsOptName)
        else
            ConnectorImplementation(
                name = identification.name,
                shortName = identification.shortName,
                from = sourcesOptName,
                to = targetsOptName)

        // add element to owner
        model.addUnownedElement(association, toEffectiveName(owner))

        // add Specialization to association; Link if no specialization is given (TODO).
        model.addUnownedElement(SpecializationImplementation(association, superclass?:"Any"), toEffectiveName(owner)+ "::${association.effectiveName}")

        if (sourcesType != null) {
            model.addUnownedElement(sources, toEffectiveName(owner) + "::${association.effectiveName}")
            val fromMultiplicity = ValueFeatureImplementation(name="from-multiplicity", valueSpecs = mutableListOf(sourcesMult))
            model.addUnownedElement(fromMultiplicity, Identity(association))
            model.addUnownedElement(SpecializationImplementation(fromMultiplicity, "ScalarValues::Integer"), Identity(fromMultiplicity))
            model.addUnownedElement(SpecializationImplementation(sources, sourcesType.str!!) , toEffectiveName(owner) + "::${association.effectiveName}::from")
        }
        if (targetsType != null) {
            model.addUnownedElement(targets, toEffectiveName(owner) + "::${association.effectiveName}")
            val toMultiplicity = ValueFeatureImplementation(name="to-multiplicity", valueSpecs =  mutableListOf(targetsMult))
            model.addUnownedElement(toMultiplicity, toEffectiveName(owner) + "::${association.effectiveName}")
            model.addUnownedElement(SpecializationImplementation(toMultiplicity, "ScalarValues::Integer"), Identity(toMultiplicity))
            model.addUnownedElement(SpecializationImplementation(targets, targetsType.str!!) , toEffectiveName(owner) + "::${association.effectiveName}::to")
        }
    }


    /**
     * The semantic action for production 'subject "imports"  ObjectList'.
     * Each of the strings in the objList is added to the import list of the subject.
     */
    fun imports(subject: QualifiedName, objList: List<QualifiedName>) {
        objList.forEach { obj ->
            if (subject == "Document")
                uses(subject, listOf(obj)) // Deprecated!
            else
                model.addUnownedElement(
                    NamespaceImportImplementation(
                        owner = Identity(subject),
                        importedNamespace = Identity(obj)),
                    subject)
        }
    }

    /**
     * The semantic action for production: subjectName "loads" projectNameList.
     * Loads a project from the repository.
     * @param subject must be either Cell or Document.
     * @param objList a list of projects that shall be imported
     */
    fun uses(subject: QualifiedName, objList: List<QualifiedName>) {
        objList.forEach { projectName ->
            if (subject == "Document" || subject == "Cell")
                try {
                    model.loadProjectFromRepository(projectName, false)
                } catch (error: Exception) {
                    model.report("in project usage $projectName: " + (error.message?:"error in import"))
                }
            else
                throw SemanticError("uses can only be used with a Document or Cell")
        }
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
            "ITE" -> return AstIte(model, param)
            "ENUM" -> return AstEnum(model, param)
            "sum_i" -> return AstSumI(namespace, model, param)
            "SUM" -> return AstSumHasA(model, namespace, param, transitive = true)
            "sumOverParts" -> return AstSumHasA(model, namespace, param, transitive = true)
            "sumOverSubclasses" -> return AstSumIsA(model, namespace, param, transitive = true)
            "productOverParts" -> return AstProductHasA(model, namespace, param, transitive = true)
            "productOverSubclasses" -> return AstProductIsA(model, namespace, param, transitive = true)
            "sumOverPartsNotTransitive" -> return AstSumHasA(model, namespace, param, transitive = false)
            "sumOverSubclassesNotTransitive" -> return AstSumIsA(model, namespace, param, transitive = false)
            "productOverPartsNotTransitive" -> return AstProductHasA(model, namespace, param, transitive = false)
            "productOverSubclassesNotTransitive" -> return AstProductIsA(model, namespace, param, transitive = false)
            "ln" -> return AstLn(model, param)
            "exp" -> return AstExp(model, param)
            "sqr" -> return AstSqr(model, param)
            "sqrt" -> return AstSqrt(model, param)
            "ceil" -> return AstCeil(model, param)
            "floor" -> return AstFloor(model, param)
            "power2" -> return AstPower2(model, param)
            "pow2" -> return AstPower2(model, param)
            "powerb" -> return AstPower(model, param)
            "power" -> return AstPower(model, param)
            "powb" -> return AstPower(model, param)
            "pow" -> return AstPower(model, param)
            "toReal" -> return AstToReal(model, param)
            "DateTime" -> return AstDateTime(model, param)
            "Date" -> return AstDate(model, param)
            "Month" -> return AstMonth(model, param)
            "Year" -> return AstYear(model, param)
            "max" -> return AstMax(model, param)
            "min" -> return AstMin(model, param)
            "abs" -> return AstAbs(model, param)
            "intersect" -> return AstIntersect(model, param)
            "bySubclasses" -> return AstBySubclasses(model, namespace, param)
            "byImplements" -> return AstByImplements(model, namespace, param)
            "linear" -> return AstLinear(model, param)
            "step" -> return AstStep(model,param)
            "Real" -> return AstReal(model, param)
            "Integer" -> return AstInteger(model, param)
            "norm" -> return AstNormalizeVector(model,param)
            "angle" -> return AstVectorAngle(model,param)
            else -> throw SemanticError("identifier $function not a known function")
        }
    }

    /**
     * Will be phased out ...
     * Special processing of "shows" relationship.
     */
    private fun handleDisplayRelationship(relationshipQualifiedName: QualifiedName, targets: List<QualifiedName>, annotation: Annotation) {
        require(relationshipQualifiedName == "shows")
        val relationship = RelationshipImplementation(
            name = "shows",
            source = mutableListOf(Identity("Display")),
            target = targets.toIdentityList()
        )
        model.create(relationship, annotation)
        // Add annotation between textual representation and generated relationship
        // annotation.addAnnotatedElement(anno)
    }

    /**
     * Generates an ITE-Statement from an if-else expression.
     */
    fun ifElseExpression(condExpr: AstNode, thenExpr: AstNode, elseExpr: AstNode): AstIte {
        return AstIte(model, arrayListOf(condExpr, thenExpr, elseExpr))
    }
}
