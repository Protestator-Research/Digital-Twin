package Parser

import com.github.tukcps.sysmd.entities.*
import com.github.tukcps.jaadd.*
import com.github.tukcps.sysmd.ast.*
import com.github.tukcps.sysmd.ast.functions.AstHasA
import com.github.tukcps.sysmd.ast.functions.AstIsA
import com.github.tukcps.sysmd.ast.functions.AstNot
import com.github.tukcps.sysmd.entities.implementation.AnnotationImplementation
import com.github.tukcps.sysmd.entities.implementation.FeatureImplementation
import com.github.tukcps.sysmd.entities.implementation.MultiplicityImplementation
import com.github.tukcps.sysmd.exceptions.*
import com.github.tukcps.sysmd.quantities.Quantity
import com.github.tukcps.sysmd.services.reportInfo
import com.github.tukcps.sysmd.services.resolveName
import com.github.tukcps.jaadd.values.IntegerRange
import com.github.tukcps.jaadd.values.XBool
import com.github.tukcps.sysmd.ast.AstLeaf
import com.github.tukcps.sysmd.parser.*
import com.github.tukcps.sysmd.parser.SysMdSemantics
import com.github.tukcps.sysmd.quantities.Unit
import com.github.tukcps.sysmd.quantities.VectorQuantity
import com.github.tukcps.sysmd.services.AgilaSession
import com.github.tukcps.sysmd.services.report


/**
 * This class provides a parser for the language SysMD, a mix of near-SysML v2 and Markdown.
 * The parser uses the recursive descent method.
 * Examples are in the unit test.
 * Parameters are:
 * @param model: the model in which the result will be saved; by default the memory-only model
 * @param input: optional input as a String that will be passed to the scanner.
 * @block: optional lambda that will be executed in scope of the parser production rules, for debugging & testing.
 */
open class ParserSysMD(
    override val model: AgilaSession,                            // model in which the results will be returned.
    override val textualRepresentation: TextualRepresentation,   // the textual representation in which parsing is done
    input: String? = null                               // input as a String of scanner; if not given, the body of textual representation
) : com.github.tukcps.sysmd.parser.ParserSysMD(model=model,textualRepresentation=textualRepresentation,input=input) {

    // A class that implements the Semantic Actions on the Agila model.
    override var semantics = SysMdSemantics(model, null, "Global", textualRepresentation)

    // The line in which a triple started with a subject; used for error reporting.
    private var subjectLine: Int = 0

    init {
        if (textualRepresentation.language.firstName() == "SysMD") {
            if (input == null) {
                val generatedElementsAnnotation = AnnotationImplementation(
                    name = "Generated elements",
                    sources = mutableListOf(Identity(ref = textualRepresentation)),
                    targets = mutableListOf()
                )
                semantics.namespacePrefix = textualRepresentation.getNamespacePrefix()
                require(model[textualRepresentation.elementId] != null)
                semantics.generatedElementsAnnotation = model.create(generatedElementsAnnotation, textualRepresentation)
            }
        } else {
            model.report("SysMD parser called with textual representation that is not tagged with language SysMD")
        }
    }


    /**
     * Implementation of the production rule:
     *    SysMD :- (Triple ".")* EOF
     */
    override fun parseSysMD() {
        while (token.kind != Definitions.Token.Kind.EOF) {
            try {
                parseTriple()
                consume(Definitions.Token.Kind.DOT, Definitions.Token.Kind.EOF) // Triple without DOT and only EOF shall deprecate.
            } catch (exception: Exception) {
                if (model.settings.catchExceptions) handleError(exception)
                else throw exception
            }
        }
        consume(Definitions.Token.Kind.EOF)
    }

    /**
     * A semantic triple, or a description:
     * Triple :- [COMMENT_LIT]
     *      [(
     *          | QualifiedName isA                 Classifier
     *          | QualifiedName hasA                FeatureList
     *          | QualifiedName uses                ProjectList
     *          | QualifiedName imports             QualifiedNameList
     *          | QualifiedName defines             DefinitionList
     *          | QualifiedName Name<Relationship>  QualifiedNameList
     *      ) *]
     *
     * For implementation, we consider the special relations IS_A, HAS_A, IMPORTS, DEFINES separately.
     */
    override fun parseTriple() {
        val subject: Identification

        if (tokenIs(Definitions.Token.Kind.EOF))  return

        parseIdentification().also { subject = it; subjectLine = consumedToken.lineNo }

        when(consumeToken()) {
            Definitions.Token.Kind.IS_A -> parseClass(subject, "")
            Definitions.Token.Kind.HAS_A -> parseFeatureList(subject)
            Definitions.Token.Kind.USES -> parseQualifiedNameList().also { semantics.uses(subject.toName(), it) }
            Definitions.Token.Kind.IMPORTS -> parseQualifiedNameList().also { semantics.imports(subject.toName(), it) }
            Definitions.Token.Kind.DEFINES -> parseDefinitionList(subject.toName())
            Definitions.Token.Kind.NAME_LIT -> {
                val relationship: QualifiedName = consumedToken.string
                parseQualifiedNameList()    .also {
                    semantics.hasRelationshipFeature(semantics.namespacePrefix?:"Global", null, listOf(subject.toName()), relationship, it)
                }
            }
            else ->
                throw SyntaxError(this, "Expecting relation but read $consumedToken")
        }
    }


    /**
     * Parses source and target of a relationship classification with multiplicity each.
     * The From and to parts are each optional; the default multiplicity is 1 .. 1.
     * Production rule:
     *
     *  sourceTargetWithMultiplicity :- [FROM NAME Multiplicity] [TO NAME Multiplicity]
     *
     * @return Pair of two pairs of name and multiplicity each.
     */
    private fun parseRelationshipDefinition(subject: Identification, owner: QualifiedName)   {
        consume(Definitions.Token.Kind.RELATIONSHIP, Definitions.Token.Kind.CONNECTOR)

        val superclass = if (token.kind == Definitions.Token.Kind.NAME_LIT) parseQualifiedName() else null
        var sources: Feature? = null    // We model the sources as an optional Typed Feature with multiplicity.
        var sourcesType: Identity<Type>? = null
        var sourcesMultiplicity: IntegerRange? = null
        var targets: Feature? = null
        var targetsType: Identity<Type>? = null
        var targetsMultiplicity: IntegerRange? = null

        if (consumeIfTokenIs(Definitions.Token.Kind.FROM)) {
            parseMultiplicity().also{ sourcesMultiplicity = it }
            parseQualifiedName().also {
                sources = FeatureImplementation(name = "from")
                sourcesType = Identity(str = it)
            }
        }
        if (consumeIfTokenIs(Definitions.Token.Kind.TO)) {
            parseMultiplicity().also { targetsMultiplicity = it }
            parseQualifiedName().also {
                targets = FeatureImplementation(name = "to")
                targetsType = Identity(str = it)
            }
        }
        semantics.isARelationship(subject, superclass, sources, sourcesType, sourcesMultiplicity, targets, targetsType, targetsMultiplicity, owner)
    }


    /**
     * Identification ["," identification]
     * Semantics: returns a list of identifications that have been parsed.
     */
    private fun parseQualifiedNameList(): MutableList<QualifiedName> {
        val result = mutableListOf<QualifiedName>()
        parseQualifiedName().also { result.add(it) }
        while (token.kind == Definitions.Token.Kind.COMMA) {
            consume(Definitions.Token.Kind.COMMA)
            parseQualifiedName().also { result.add(it) }
        }
        return result
    }


    /**
     * Definition :- Identification "isA" QualifiedName ["from" QualifiedNameList "to" QualifiedNameList]
     */
    private fun parseDefinition(owner: QualifiedName) {
        val subject: Identification

        parseIdentification().also { subject = it }
        consume(Definitions.Token.Kind.IS_A, Definitions.Token.Kind.NAME_LIT, Definitions.Token.Kind.COMMA)
        if (consumedToken.kind == Definitions.Token.Kind.IS_A) {
            parseClass(subject, owner)
        } else {
            model.report(semantics.namespace, "Error or deprecated syntax: Replace 'source RELATION target' with 'owner hasA Relation name: source REL target'")
        }
    }

    /**
     * Class definition
     * QualifiedName
     * | PACKAGE QualifiedName
     * | RELATIONSHIP [QualifiedName] [FROM QualifiedNameList] [TO QualifiedNameList]
     * | CONNECTOR [QualifiedName] [FROM QualifiedNameList] [TO QualifiedNameList]
     * /// eventually: | QUANTITY Real = ConstantExpression.
     */
    private fun parseClass(subject: Identification, owner: QualifiedName) {
        when (token.kind) {
            Definitions.Token.Kind.NAME_LIT ->     parseQualifiedName().also   { semantics.isAClass(subject, it, owner)}
            // Deprecated:
            Definitions.Token.Kind.PACKAGE ->      consumeToken().also {
                model.report(semantics.namespace, "Deprecated syntax: Replace 'name isA Package' with 'owner hasA Package name'")
                semantics.hasAPackage(owner, subject)
            }
            Definitions.Token.Kind.RELATIONSHIP, Definitions.Token.Kind.CONNECTOR -> parseRelationshipDefinition(subject, owner)
            else -> throw SyntaxError(parser = this, "In class definition: after 'isA', a name, or 'Relationship', 'Connector' is expected.")
        }
    }

    /**
     * Definition (";" Definition)*
     */
    private fun parseDefinitionList(subject: QualifiedName) {
        parseDefinition(subject)
        while (token.kind == Definitions.Token.Kind.SEMICOLON) {
            consume(Definitions.Token.Kind.SEMICOLON)
            parseDefinition(subject)
        }
    }


    /**
     * An identification, following the conventions of SysML v2 textual:
     *   '<' NAME '>' NAME | NAME '<' NAME '>'
     * This gives SysMD the ability to identify an element either via a unique ID or a path specification.
     */
    override fun parseIdentification(): Identification {
        var shortName: String? = null
        var name: String? = null
        when(token.kind) {
            Definitions.Token.Kind.LT -> {
                nextToken()
                parseSimpleName().also { shortName = it }
                consume(Definitions.Token.Kind.GT)
                if (token.kind == Definitions.Token.Kind.NAME_LIT) {
                    parseQualifiedName().also { name = it }
                }
            }
            Definitions.Token.Kind.NAME_LIT -> {
                parseQualifiedName().also { name = it }
                if (token.kind == Definitions.Token.Kind.LT) {
                    nextToken()
                    parseSimpleName().also { shortName = it }
                    consume(Definitions.Token.Kind.GT)
                }
            }
            else -> throw SyntaxError(this, "Expected identification or qualified name, but read: $token")
        }
        return Identification(shortName, name)
    }


    /**
     * A list of Objects, separated by comma and finished by a dot.
     *   ObjectList :- Object ("," Object)*
     */
    override fun parseFeatureList(subject: Identification) {
        parseFeature(subject)
        while (token.kind == Definitions.Token.Kind.COMMA || token.kind == Definitions.Token.Kind.SEMICOLON) {
            consume(Definitions.Token.Kind.COMMA, Definitions.Token.Kind.SEMICOLON)
            parseFeature(subject)
        }
    }


    /**
     * A SimpleName, just a name literal
     * SimpleName :- NAME
     **/
    private fun parseSimpleName(): SimpleName {
        if (token.kind != Definitions.Token.Kind.NAME_LIT)
            throw SyntaxError(this, "Expected name, but read ${toString()}")
        val name = token.string
        nextToken()
        return name
    }


    /**
     * Parses a qualified name.
     * QualifiedName :- "NAME("::"NAME)*
     */
    private fun parseQualifiedName(): String {
        if (token.kind != Definitions.Token.Kind.NAME_LIT)
            throw SyntaxError(this, "Expected name, but read ${toString()}")
        var path = token.string
        nextToken()

        while (token.kind == Definitions.Token.Kind.DPDP) {
            nextToken()
            if (token.kind != Definitions.Token.Kind.NAME_LIT)
                throw SyntaxError(this, "Expected name, but read ${toString()}$")
            path += "::" + token.string
            nextToken()
        }
        return path
    }


    /**
     * Parses an optional Multiplicity; if it is not present, the result is [1, 1]
     * "[" (IntegerLiteral | "*") ["," (IntegerLiteral | "*" ] "]"
     */
    private fun parseMultiplicity(): IntegerRange {
        var multiplicity = IntegerRange(1, 1)
        if(consumeIfTokenIs(Definitions.Token.Kind.LCBRACE)) {
            parseIntegerRange().also { multiplicity = it }
            consume(Definitions.Token.Kind.RCBRACE)
        }
        return multiplicity
    }


    /**
     * An object in a semantic triple that specifies a feature.
     * It must be of the form:
     * OwnedElement  :-
     *            Valuekind     Identification ValueFeature
     *          | ComponentKind Identification ComponentFeature
     *          | RelationKind  Identification RelationshipFeature
     */
    private fun parseFeature(subject: Identification) {
        when (token.kind) {
            Definitions.Token.Kind.PACKAGE -> {
                nextToken()
                val name = parseIdentification()
                semantics.hasAPackage(subject.toName(), name)
            }
            Definitions.Token.Kind.RELATIONSHIP, Definitions.Token.Kind.CONNECTOR -> {
                nextToken()
                val name = parseIdentification()
                parseRelationshipFeature(subject.toName(), name)
            }
            Definitions.Token.Kind.FEATURE -> {
                nextToken()
                val name = parseIdentification()
                parseComponentFeature(subject.toName(), name)
            }
            Definitions.Token.Kind.VALUE -> {
                nextToken()
                val name = parseIdentification()
                parseValueFeature(subject.toName(), name)
            }
            Definitions.Token.Kind.NAME_LIT -> {
                parseQualifiedName().also {
                    when (it) {
                        "Value", "Quantity", "Requirement", "Performance" -> {
                            val name = parseIdentification()
                            parseValueFeature(subject.toName(), name)
                        }

                        "Component", "Processor", "Software", "Part", "Function", "System" -> {
                            val name = parseIdentification()
                            parseComponentFeature(subject.toName(), name)
                        }

                        "Relation", "Connector", "Link" -> {
                            val name = if (token.kind == Definitions.Token.Kind.NAME_LIT) parseIdentification() else null
                            parseRelationshipFeature(subject.toName(), name)
                        }

                        else -> {
                            parseValueFeature(subject.toName(), Identification(name=it))
                        }
                    }
                }
            }
            else -> throw SyntaxError(this, "Expecting Feature, Value, Relationship, or NAME, but read $token")
        }
    }


    /**
     * ValueFeature :- ':' [All|One] OfClass ["(" (Range Unit | true | false ")" ] [ '[' Unit ']' ] [ '=' Expression ]
     */
    private fun parseValueFeature(subject: QualifiedName, identification: Identification) {
        val multiplicity: Multiplicity = MultiplicityImplementation(valueSpec = IntegerRange(1,1))
        val type: QualifiedName
        var direction = Feature.FeatureDirectionKind.IN

        consume(Definitions.Token.Kind.DP)

        // Optional multiplicity; default is 1.
        // parseMultiplicity().also { multiplicity = it }

        consumeIfTokenIs(Definitions.Token.Kind.ALL, Definitions.Token.Kind.ONE).also {
            when (consumedToken.kind) {
                Definitions.Token.Kind.ALL -> direction = Feature.FeatureDirectionKind.OUT
                Definitions.Token.Kind.ONE -> direction = Feature.FeatureDirectionKind.IN
                else -> {} // keep default.
            }
        }

        // Type: The type of which the feature shall be a subclass of ...
        parseQualifiedName().also { type = it }

        val feature = semantics.hasValueFeature(subject, identification, multiplicity, type, direction)

        parseConstraint().also { feature.valueSpecs = it }

        if (consumeIfTokenIs(Definitions.Token.Kind.LCBRACE)) {
            parseUnit().also { feature.unitSpec(it) }
            consume(Definitions.Token.Kind.RCBRACE)
        }

        if (tokenIs(Definitions.Token.Kind.EQ)) {
            val iBeforeExpression = position
            nextToken()
            parseExpression().also {
                feature.ast = AstRoot(model, feature, it)
                val iAfterExpression = position - token.string.length
                val exprString = input.subSequence(iBeforeExpression, iAfterExpression)
                feature.dependency = exprString.toString().trim()
            }
        }
    }


    /**
     * ComponentFeature :- ':' [ Multiplicity ] QualifiedName<type>  ['=' InstanceList]
     */
    private fun parseComponentFeature(subject: QualifiedName, name: Identification) {
        var multiplicity: Multiplicity
        val type: QualifiedName
        var instances: List<QualifiedName> = emptyList()

        consume(Definitions.Token.Kind.DP)
        parseMultiplicity().also { multiplicity = MultiplicityImplementation(valueSpec = it) }
        parseQualifiedName().also { type = it }

        if (consumeIfTokenIs(Definitions.Token.Kind.EQ)) {
            instances = parseQualifiedNameList()
        }
        semantics.hasComponentFeature(subject, name, multiplicity, type, instances)
    }

    /**
     * RelationshipFeatureExpression :-
     *          ':' QualifiedNameList<source> QualifiedName QualifiedNameList<target>
     *      |   '=' QualifiedName FROM QualifiedNameList<source> TO QualifiedNameList<target>
     */
    private fun parseRelationshipFeature(subject: QualifiedName, name: Identification?) {
        val source: List<QualifiedName>
        val relationship: QualifiedName
        val target: List<QualifiedName>

        if (tokenIs(Definitions.Token.Kind.EQ)) {
            consume(Definitions.Token.Kind.EQ)
            parseQualifiedNameList().also { source = it }
            parseQualifiedName().also { relationship = it }
            parseQualifiedNameList().also { target = it }
        } else if (tokenIs(Definitions.Token.Kind.DP)) {
            consume(Definitions.Token.Kind.DP)
            parseQualifiedName().also { relationship = it }
            consumeIfTokenIs(Definitions.Token.Kind.EQ)    // just optional.
            consume(Definitions.Token.Kind.FROM)
            parseQualifiedNameList().also { source = it }
            consume(Definitions.Token.Kind.TO)
            parseQualifiedNameList().also { target = it }
        } else throw SyntaxError(this, "In Relationship: expected 'from' or name.")
        semantics.hasRelationshipFeature(subject, name, source, relationship, target)
    }

    /**
     * IntegerRange :- ConstInt [".." ConstInt]
     */
    private fun parseIntegerRange(): IntegerRange {
        val result = IntegerRange(IntegerRange.Integers)
        parseConstInt().also { result.min = it; result.max = it }
        if (consumeIfTokenIs(Definitions.Token.Kind.DOTDOT)) {
            parseConstInt().also { result.max = it }
        }
        if (result.min > result.max)
            throw SyntaxError(this, message = "max of range must be larger or equal min")
        return result
    }

    /**
     * ValueRange :- ValueLiteral [.. ValueLiteral]
     **/
    private fun parseValueRange(): Quantity {
        var result: Quantity
        if (tokenIs(Definitions.Token.Kind.INTEGER_LIT)) {
            parseConstInt().also { result = Quantity(model.builder.rangeIDD(it, it)) }
        } else if (tokenIs(Definitions.Token.Kind.FLOAT_LIT)) {
            parseConstReal().also { result = Quantity(model.builder.range(it, it),"?") }
        } else
            throw SyntaxError(this, "expect value-range of form number .. number")

        if (!tokenIs(Definitions.Token.Kind.DOTDOT)) {
            return result
        }
        else { // ".." ValueLiteral
            consume(Definitions.Token.Kind.DOTDOT)
            if (tokenIs(Definitions.Token.Kind.INTEGER_LIT)) {
                parseConstInt().also {
                    result = when (result.value) {
                        is AADD -> Quantity(model.builder.range(result.aadd().getRange().min, it.toDouble()),"?")
                        is IDD -> Quantity(model.builder.rangeIDD(result.idd().getRange().min, it))
                        else -> throw SyntaxError(this, "expect range of form [number .. number]")
                    }
                }
            } else if (tokenIs(Definitions.Token.Kind.FLOAT_LIT)) {
                parseConstReal().also {
                    result = when (result.value) {
                        is AADD -> Quantity(model.builder.range(result.aadd().getRange().min, it),"?")
                        is IDD -> Quantity(model.builder.range(result.idd().getRange().min.toDouble(), it),"?")
                        else -> throw SyntaxError(this, "expect range of form [number .. number]")
                    }
                }
            }
            return result
        }
    }

    /** RealSubtype :- ["(" Number [.. ConstValue] | true | false ")"] */
    private fun parseConstraint(): MutableList<Any?> {
        val constraints = mutableListOf<Any?>()
        if (consumeIfTokenIs(Definitions.Token.Kind.LBRACE)) {
            when {
                tokenIs(Definitions.Token.Kind.MINUS, Definitions.Token.Kind.TIMES, Definitions.Token.Kind.INTEGER_LIT, Definitions.Token.Kind.FLOAT_LIT) -> {
                    while(!tokenIs(Definitions.Token.Kind.RBRACE)) { //Multiple constraints for vector
                        val min = parseNumber()
                        val max = optionalWhen(Definitions.Token.Kind.DOTDOT, default = min) {
                            consume(Definitions.Token.Kind.DOTDOT)
                            parseNumber()
                        }
                        constraints.add("$min .. $max")
                        consumeIfTokenIs(Definitions.Token.Kind.COMMA)
                    }
                    consumeIfTokenIs(Definitions.Token.Kind.RBRACE)
                }
                tokenIs(Definitions.Token.Kind.TRUE, Definitions.Token.Kind.FALSE) -> {
                    while(!tokenIs(Definitions.Token.Kind.RBRACE)){
                        if(consumeIfTokenIs(Definitions.Token.Kind.TRUE))
                            constraints.add(XBool.True)
                        else if(consumeIfTokenIs(Definitions.Token.Kind.FALSE))
                            constraints.add(XBool.False)
                    }
                    consumeIfTokenIs(Definitions.Token.Kind.RBRACE)
                }
                else ->
                    throw SyntaxError(this, "When parsing constraint: expect number range, true, or false, but read $token")
            }
            return constraints
        } else
            return mutableListOf(null)
    }


    /**
     * parseExpression parses an expression and returns the AST as result.
     * Expression :- Comparison
     * @return an AstNode with the abstract syntax tree
     */
    override fun parseExpression(): AstNode {
        return parseComparison()
    }

    /**
     * conditionalExpression :- IF expression ? expression ELSE expression
     */
    private fun parseConditionalExpression(): AstNode {
        consume(Definitions.Token.Kind.IF)
        val ifExpr: AstNode; val thenExpr: AstNode

        if (token.kind==Definitions.Token.Kind.LBRACE) {
            consume(Definitions.Token.Kind.LBRACE)
            model.reportInfo(semantics.namespace, "Deprecated: if (condition) expr else expr; use: if condition? expr else expr")
        }
        parseExpression().also { ifExpr = it }
        consume(Definitions.Token.Kind.QUESTION, Definitions.Token.Kind.RBRACE)
        parseExpression().also { thenExpr = it }
        consume(Definitions.Token.Kind.ELSE)
        parseExpression().also { return semantics.ifElseExpression(
            condExpr = ifExpr,
            thenExpr = thenExpr,
            elseExpr = it
        ) }
    }

    /**
     * parseComparison computes an expression and returns the AST as result.
     *
     * Expression :- Sum [ relOp Sum]
     */
    private fun parseComparison(): AstNode {
        var result = parseSum()
        if (consumeIfTokenIs(Definitions.Token.Kind.GT,
                Definitions.Token.Kind.LT,
                Definitions.Token.Kind.EQ,
                Definitions.Token.Kind.GE,
                Definitions.Token.Kind.LE,
                Definitions.Token.Kind.EE)) {
            val op = consumedToken.kind
            val t2 = parseSum()
            result = AstBinOp(result, op, t2)
        }
        return result
    }

    /** Sum :- Product ( ("+"|"-"|"|") Product )*     */
    private fun parseSum(): AstNode {
        var s1 = parseProduct()
        while (consumeIfTokenIs(Definitions.Token.Kind.PLUS, Definitions.Token.Kind.MINUS, Definitions.Token.Kind.OR)) {
            val op = consumedToken.kind
            val s2 = parseProduct()
            s1 = AstBinOp(s1, op, s2)
        }
        return s1
    }

    /** Product :- Value ( ("*"|"/"|"&") Value )*     */
    private fun parseProduct(): AstNode {
        var f1 = parseExponent()
        while (consumeIfTokenIs(Definitions.Token.Kind.TIMES, Definitions.Token.Kind.DIV, Definitions.Token.Kind.AND, Definitions.Token.Kind.CROSS, Definitions.Token.Kind.DOTProduct)) {
            val op = consumedToken.kind
            val f2 = parseExponent()
            f1 = AstBinOp(f1, op, f2)
        }
        return f1
    }

    /**
     * Exponent :- Value ( ("^") Value )*
     */
    private fun parseExponent(): AstNode {
        var f1 = parseUnaryOperatorExpression()
        while (consumeIfTokenIs(Definitions.Token.Kind.EXP)) {
            val op = consumedToken.kind
            val f2 = parseUnaryOperatorExpression()
            f1 = AstBinOp(f1, op, f2)
        }
        return f1
    }

    /**
     * UnaryOperatorExpression :- ["+" | "-" | "not"] ExponentExpression
     */
    private fun parseUnaryOperatorExpression(): AstNode {
        return if (consumeIfTokenIs(Definitions.Token.Kind.PLUS, Definitions.Token.Kind.MINUS, Definitions.Token.Kind.NOT)) {
            when (consumedToken.kind) {
                Definitions.Token.Kind.PLUS -> parseValue()
                Definitions.Token.Kind.MINUS -> AstUnaryOp(Definitions.Token.Kind.MINUS, parseValue())
                Definitions.Token.Kind.NOT -> AstNot(model, arrayListOf(parseValue()))
                else -> throw SyntaxError(this, "Error in unary expression")
            }
        } else
            parseValue()
    }

    /**
     *  Parameter :- "(" Expression ("," Expression)* ")"
     *             |  // nothing.
     */
    private fun parseParameter(): ArrayList<AstNode>? =
        optionalWhen(Definitions.Token.Kind.LBRACE, null) {
            nextToken()
            val parameters = ArrayList<AstNode>()
            while (token.kind != Definitions.Token.Kind.RBRACE) {
                parameters.add(parseExpression())
                while (consumeIfTokenIs(Definitions.Token.Kind.COMMA)) {
                    parameters.add(parseExpression())
                }
            }
            consume(Definitions.Token.Kind.RBRACE)
            parameters
        }


    /**
     *  Unit :-> "%" // Per cent as a unit
     *          | ["1"] (NAME_LIT ["^" INTEGER_LIT])* ["/" (NAME_LIT [^INTEGER_LIT] )+]
     **/
    override fun parseUnit(): String {
        var result = ""

        if (token.kind == Definitions.Token.Kind.PERCENT) {
            nextToken(); return "%"
        }

        // optional "1" , TODO: should be checked ...
        optionalConsume(Definitions.Token.Kind.INTEGER_LIT) { result += consumedToken.number.toInt().toString() + " " }

        while (token.kind == Definitions.Token.Kind.NAME_LIT || token.kind == Definitions.Token.Kind.EURO) {
            nextToken().also { result += consumedToken.toString() }
            optionalConsume(Definitions.Token.Kind.EXP) {
                consume(Definitions.Token.Kind.INTEGER_LIT).also { result += "^${consumedToken.number.toInt()}" }
            }
            result += " "
        }

        optionalConsume(Definitions.Token.Kind.DIV) {
            result += "$consumedToken "
            while (token.kind == Definitions.Token.Kind.NAME_LIT) {
                nextToken().also { result += consumedToken.toString() }
                optionalConsume(Definitions.Token.Kind.EXP) {
                    consume(Definitions.Token.Kind.INTEGER_LIT).also { result += "^${consumedToken.number.toInt()}" }
                }
                result += " "
            }
        }
        return result.trim()
    }


    /**
     * Value :- NUM_LIT | TRUE | FALSE
     * |  '(' ITE ("," ITE)* ')' ?????? FIX
     * |  ID that is a constant or variable.
     * |  ID '(' ITE ("," ITE)* ')' where word is a function on AADD.
     */
    private fun parseValue(): AstNode {
        var astNode: AstNode            // Value or expression
        val neg = consumeIfTokenIs(Definitions.Token.Kind.MINUS) // Sign of negative value.
        when (token.kind) {
            Definitions.Token.Kind.LCBRACE -> {                // Range of kind [number, number] unit
                consume(Definitions.Token.Kind.LCBRACE)
                val quantity: Quantity
                var unit = ""
                parseValueRange().also { quantity = it }
                consume(Definitions.Token.Kind.RCBRACE)
                if (consumeIfTokenIs(Definitions.Token.Kind.LCBRACE)) {
                    parseUnit().also { unit = it }
                    consume(Definitions.Token.Kind.RCBRACE)
                }
                astNode = when(quantity.value){
                    is AADD ->  AstLeaf(model, Quantity(quantity.value as AADD, unit))
                    is IDD ->  AstLeaf(model, Quantity(quantity.value as IDD))
                    is StrDD ->  AstLeaf(model, Quantity(quantity.value as StrDD))
                    is BDD ->  AstLeaf(model, Quantity(quantity.value as BDD))
                    else -> throw SemanticError("Unsupported type for $quantity.")
                }
            }

            Definitions.Token.Kind.FLOAT_LIT -> {              // Floating point literal of kind number unit
                val min = token.number
                var unit = ""
                consume(Definitions.Token.Kind.FLOAT_LIT)
                // optional: Extension to range by
                val max = if (tokenIs(Definitions.Token.Kind.DOTDOT)) {
                    consume(Definitions.Token.Kind.DOTDOT)
                    consume(Definitions.Token.Kind.FLOAT_LIT)
                    consumedToken.number
                } else min

                if (tokenIs(Definitions.Token.Kind.LCBRACE, Definitions.Token.Kind.NAME_LIT, Definitions.Token.Kind.PERCENT)) {
                    if (tokenIs(Definitions.Token.Kind.LCBRACE)) {
                        consume(Definitions.Token.Kind.LCBRACE)
                        parseUnit().also { unit = it }
                        consume(Definitions.Token.Kind.RCBRACE)
                    } else {
                        unit = token.string
                        consume(Definitions.Token.Kind.NAME_LIT)
                    }
                }
                astNode = AstLeaf(model, Quantity(model.builder.range(min, max), unit))
            }

            Definitions.Token.Kind.INTEGER_LIT -> {            // Integer literal
                val min = token.number.toLong()
                consume(Definitions.Token.Kind.INTEGER_LIT)
                // optional: Extension to range by
                val max = if (tokenIs(Definitions.Token.Kind.DOTDOT)) {
                    consume(Definitions.Token.Kind.DOTDOT)
                    consume(Definitions.Token.Kind.INTEGER_LIT)
                    consumedToken.number.toLong()
                } else min
                astNode = AstLeaf(model, Quantity(model.builder.rangeIDD(min, max)))
            }

            Definitions.Token.Kind.STRING_LIT -> {            // A string literal
                astNode = AstLeaf(model, Quantity(StrDD.Leaf(model.builder, token.string)))
                nextToken()
            }

            Definitions.Token.Kind.TRUE -> {               // True literal
                astNode = AstLeaf(model, Quantity(model.builder.True))
                nextToken()
            }

            Definitions.Token.Kind.FALSE -> {              // False literal
                astNode = AstLeaf(model, Quantity(model.builder.False))
                nextToken()
            }
            Definitions.Token.Kind.LBRACE -> {             // ( Expr )
                nextToken()
                var expression = parseExpression()
                try { //test if expression can be used as a Vector
                    val values = mutableListOf<DD>()
                    expression.evalUp()
                    values.add(expression.dd.clone())
                    while (consumeIfTokenIs(Definitions.Token.Kind.COMMA)) {  // Iterate through all vector elements
                        expression = parseExpression()
                        if(expression is AstLeaf || expression is AstUnaryOp) {
                            expression.evalUp()
                            values.add(expression.dd.clone())
                        }else
                            throw SemanticError("Vectors with expression, which are no values, is not allowed")
                    }
                    consume(Definitions.Token.Kind.RBRACE)
                    var unit = ""
                    if (tokenIs(Definitions.Token.Kind.LCBRACE, Definitions.Token.Kind.NAME_LIT,
                            Definitions.Token.Kind.PERCENT
                        )) {
                        if (tokenIs(Definitions.Token.Kind.LCBRACE)) {
                            consume(Definitions.Token.Kind.LCBRACE)
                            parseUnit().also { unit = it }
                            consume(Definitions.Token.Kind.RCBRACE)
                        } else {
                            unit = token.string
                            consume(Definitions.Token.Kind.NAME_LIT)
                        }
                    }
                    astNode = AstLeaf(model, VectorQuantity(values, Unit(unit)))
                } catch (e:Exception) { // is a simple expression, no vector
                    astNode = expression
                    consume(Definitions.Token.Kind.RBRACE)
                }
            }
            Definitions.Token.Kind.HAS_A -> {              // hasA(partName)
                nextToken()
                consume(Definitions.Token.Kind.LBRACE)
                val ownerName = parseQualifiedName()
                consume(Definitions.Token.Kind.COMMA)
                val ownedName = parseQualifiedName()
                consume(Definitions.Token.Kind.RBRACE).also { astNode = AstHasA(model, semantics.namespace, ownerName, ownedName) }
            }
            Definitions.Token.Kind.IS_A -> {               // isA(typeName)
                nextToken()
                consume(Definitions.Token.Kind.LBRACE)
                val subclassName = parseQualifiedName()
                consume(Definitions.Token.Kind.COMMA)
                val superclassName = parseQualifiedName()
                consume(Definitions.Token.Kind.RBRACE).also { astNode = AstIsA(model, semantics.namespace, subclassName, superclassName) }
            }
            Definitions.Token.Kind.NAME_LIT -> {           // qualifiedName [( parameters )]
                val name = parseQualifiedName()
                val params = parseParameter()
                astNode = if (params == null)
                        AstLeaf(semantics.namespace, name, model)   // an identifier
                    else
                        semantics.mkFuncCall(name, params)     // a function call
            }
            Definitions.Token.Kind.IF -> { astNode = parseConditionalExpression() }
            else ->
                throw SyntaxError(this, "expected value, but read: $token")
        }
        return if (neg) {
            if ((astNode is AstLeaf) && (astNode as AstLeaf).property == null) {
                (astNode as AstLeaf).literalVal = VectorQuantity(
                    (astNode as AstLeaf).literalVal!!.negate().values, (astNode as AstLeaf).literalVal!!.unit.clone(),
                    (astNode as AstLeaf).literalVal!!.unitSpec
                )
                val remember = astNode.upQuantity.negate().values
                astNode.upQuantity = VectorQuantity(astNode.downQuantity.negate().values, astNode.upQuantity.unit, astNode.upQuantity.unitSpec)
                astNode.downQuantity = VectorQuantity(remember, astNode.downQuantity.unit, astNode.downQuantity.unitSpec)
                astNode
            } else AstBinOp(AstLeaf(model, Quantity(model.builder.scalar(0.0),"?")),
                Definitions.Token.Kind.MINUS, astNode)
        }
        else astNode
    }


    /** A number literal (Int or Float) or a property with known value */
    private fun parseConstReal(): Double {
        val neg = consumeIfTokenIs(Definitions.Token.Kind.MINUS) // Sign of negative value.
        return when (token.kind) {
            Definitions.Token.Kind.INTEGER_LIT,
            Definitions.Token.Kind.FLOAT_LIT -> {           // Number literal
                val value = token.number
                nextToken()
                if (neg) -value else value
            }
            Definitions.Token.Kind.NAME_LIT -> {
                val name = parseQualifiedName()
                val p = semantics.namespace.resolveName<ValueFeature>(name) ?: throw ElementNotFoundException(this, name)
                if (neg) -p.rangeSpecs[0].min else p.rangeSpecs[0].min //ConstReal is never a vector
            }
            Definitions.Token.Kind.TIMES ->  {
                nextToken()
                return if(neg) model.settings.minReal else model.settings.maxReal
            }
            else -> throw SemanticError("Expected real constant (number literal, defined name, or '*'.")
        }
    }

    /** A number literal (Int or Float) or a property with known value */
    private fun parseNumber(): String {
        val neg = consumeIfTokenIs(Definitions.Token.Kind.MINUS) // Sign of negative value.
        return when (token.kind) {
            Definitions.Token.Kind.INTEGER_LIT, Definitions.Token.Kind.FLOAT_LIT -> {           // Number literal
                val value = if (token.number.rem(1).equals(0.0))
                        token.number.toLong().toString()  // No ".0" as in Double.toString ...
                    else
                        token.number.toString()
                nextToken()
                if (neg) "-$value" else value
            }
            Definitions.Token.Kind.TIMES ->  {
                nextToken()
                "*"
            }
            else -> throw SemanticError("Expected number literal or '*'.")
        }
    }

    /** A number literal (Int) or a property with known value */
    private fun parseConstInt(): Long {
        val neg = consumeIfTokenIs(Definitions.Token.Kind.MINUS) // Sign of negative value.
        when (token.kind) {
            Definitions.Token.Kind.INTEGER_LIT -> {           // Number literal
                val value = token.number
                nextToken()
                return if (neg) -value.toLong() else value.toLong()
            }
            Definitions.Token.Kind.NAME_LIT -> {
                val name = parseQualifiedName()
                val p = semantics.namespace.resolveName<ValueFeature>(name)
                    ?: throw ElementNotFoundException(this.textualRepresentation, element=null, name=name, token=consumedToken)
                return if (neg) -p.intSpecs[0].min else p.intSpecs[0].min // ConstInt is never a vector
            }
            Definitions.Token.Kind.TIMES -> {
                nextToken()
                return if(neg) model.settings.minInt else model.settings.maxInt
            }
            else -> throw SyntaxError(this,"Expected integer constant (number literal, defined name, or '*'.")        }
    }


    /**
     * Helper function that checks if the token is start, and if so executes production rule,
     * otherwise it returns default
     */
    private fun <T> optionalWhen(start: Definitions.Token.Kind, default: T, rule: ParserSysMD.() -> T): T =
        if (token.kind == start) rule() else default


    /**
     * Helper function. If there is the token start in the input stream,
     * consume it and apply the following production rule. The rule produces a result of type T.
     * The default result is given as parameter of the same type.
     */
    private fun <T> optionalConsume(start: Definitions.Token.Kind, default: T? = null, rule: ParserSysMD.() -> T? = { null }): T? =
        if (this.token.kind == start) {
            consume(start)
            rule()
        } else
            default

    /**
     * Consumes a token and returns its kind.
     */
    private fun consumeToken(): Definitions.Token.Kind {
        nextToken()
        return consumedToken.kind
    }

    /**
     * Enters an error message in the status and tries to re-sync with stream of token.
     * It does so by reading until reaching a DOT which marks the end of a triple.
     * @param exception Exception that was thrown and caught prior to starting error handling
     */
    private fun handleError(exception: Exception) {

        // report error.
        if (exception is SysMDException) {
            model.report(exception)
        } else
            model.report(SysMDError("Exception: ${exception.message}"))
        // Skip input until we get next DOT (=end of statement) or EOF.
        while (token.kind != Definitions.Token.Kind.DOT && token.kind != Definitions.Token.Kind.EOF)
            consumeToken()
        consumeToken()
    }
}
