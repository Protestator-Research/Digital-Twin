package Parser

import com.github.tukcps.jaadd.*
import com.github.tukcps.jaadd.values.IntegerRange
import com.github.tukcps.jaadd.values.XBool
import com.github.tukcps.sysmd.ast.*
import com.github.tukcps.sysmd.ast.functions.AstHasA
import com.github.tukcps.sysmd.ast.functions.AstIsA
import com.github.tukcps.sysmd.ast.functions.AstIte
import com.github.tukcps.sysmd.ast.functions.AstNot
import com.github.tukcps.sysmd.entities.*
import com.github.tukcps.sysmd.entities.implementation.AnnotationImplementation
import com.github.tukcps.sysmd.entities.implementation.MultiplicityImplementation
import com.github.tukcps.sysmd.exceptions.*
import com.github.tukcps.sysmd.parser.*
import com.github.tukcps.sysmd.parser.Scanner
import com.github.tukcps.sysmd.parser.Scanner.Definitions.Token
import com.github.tukcps.sysmd.parser.Scanner.Definitions.Token.Kind.*
import com.github.tukcps.sysmd.quantities.Quantity
import com.github.tukcps.sysmd.quantities.Unit
import com.github.tukcps.sysmd.quantities.VectorQuantity
import com.github.tukcps.sysmd.services.AgilaSession
import com.github.tukcps.sysmd.services.report
import com.github.tukcps.sysmd.services.reportInfo
import com.github.tukcps.sysmd.services.resolveName
import java.util.*


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
    open val model: AgilaSession,                            // model in which the results will be returned.
    open val textualRepresentation: TextualRepresentation,   // the textual representation in which parsing is done
    input: String? = null                               // input as a String of scanner; if not given, the body of textual representation
) : Scanner(input?:textualRepresentation.body) {

    // A class that implements the Semantic Actions on the Agila model.
    var semantics = SysMdSemantics(model, null, textualRepresentation)

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
    open fun parseSysMD() {
        while (token.kind != EOF) {
            try {
                if (token.toKind() in setOf(PACKAGE, PART, ATTRIBUTE, DEF, ASSOC)) {
                    parseElement()
                    consume(DOT, EOF, SEMICOLON)
                } else {
                    parseTriple()
                    consume(DOT, EOF)
                }
            } catch (exception: Exception) {
                if (model.settings.catchExceptions) {
                    handleError(exception)
                    semantics.initOwners()
                }
                else throw exception
            }
        }
        consume(EOF)
    }


    /**
     * Element  :-
     *                "Def"         Definition
     *              | "Part"        Identification ":" QualifiedName = Expression
     *              | "Attribute"   Identification ":" QualifiedName ... = Expression
     *              | "Association" Identification ":" QualifiedName from ... to
     *              | "Package"     Identification
     *
     * NOTE: We allow alternative 'kind names' for the leading keywords
     */
    fun parseElement() {
        nextToken()
        when(consumedToken.toKind()) {
            DEF        -> { parseDefinition() }
            PART       -> { parseComponentFeature() }
            ATTRIBUTE  -> { parseValueFeature() }
            ASSOC      -> { parseAssociation() }
            PACKAGE    -> { parseIdentification().also { semantics.addPackage(identification = it) } }
            else ->
                throw Exception("Syntax Error: Expected def, part, attribute, assoc, package or equivalent class name, but read '${consumedToken.string}' and ${token.string}")
//                throw SyntaxError(this, "Expected def, part, attribute, assoc, package or equivalent class name, but read '${consumedToken.string}' and ${token.string}")}
        }
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
    open fun parseTriple() {

        if (tokenIs(EOF))  return

        parseIdentification().also { subjectLine = consumedToken.lineNo; semantics.pushOwner(it.name?:it.shortName!!) }

        when(consumeToken()) {
            IS_A     -> parseQualifiedName().also       {
                val subject = semantics.owners.pop()
                semantics.addClass(identification = Identification(name=subject), superclass = it)
                semantics.owners.push(subject)
            }
            HAS_A    -> parseElementList()
            USES     -> parseQualifiedNameList().also { semantics.loadProject(it) }
            IMPORTS  -> parseQualifiedNameList().also { semantics.addImport(semantics.ownerName(), it) }
            DEFINES  -> parseDefinitionList()
            NAME_LIT -> {
                val relationship: QualifiedName = consumedToken.string
                parseQualifiedNameList()    .also {
                    semantics.addLink(semantics.textualRepresentation.getOwnerPrefix(), null, listOf(semantics.ownerName()), relationship, it)
                }
            }
            else -> {
                semantics.initOwners()
                throw Exception("Expecting valid triple (isA, hasA, uses, imports, defines, user-defined, but read $consumedToken")
//                throw SyntaxError(this, "Expecting valid triple (isA, hasA, uses, imports, defines, user-defined, but read $consumedToken")
            }
        }
        semantics.popOwner()
    }


    /**
     * Parses source and target of a relationship classification with multiplicity each.
     * The From and to parts are each optional; the default multiplicity is 1 .. 1.
     * Production rule:
     *
     *  RelationShipDefinition :- Kind [QualifiedName<Type>]
     *                              [FROM QualifiedName<Type> Multiplicity]
     *                              [TO QualifiedName<Type> Multiplicity]
     *
     * @return Pair of two pairs of name and multiplicity each.
     */
    private fun parseRelationshipDefinition(identification: Identification) {
        val relationshipKind = token.string
        var valueRel = false
        consume(ASSOC, NAME_LIT)

        val superclass = if (token.kind == NAME_LIT) parseQualifiedName() else "Any" // Link!
        var sourcesType: Identity<Feature>? = null
        var sourcesMultiplicity: IntegerRange? = null
        var targetsType: Identity<Feature>? = null
        var targetsMultiplicity: IntegerRange? = null

        if (consumeIfTokenIs(FROM)) {
            parseMultiplicity().also  { sourcesMultiplicity = it }
            if (token.toKind() == ATTRIBUTE) consumeToken().also { valueRel = true }
            parseQualifiedName().also { sourcesType = Identity(str = it) }
        }
        if (consumeIfTokenIs(TO)) {
            parseMultiplicity().also { targetsMultiplicity = it }
            parseQualifiedName().also { targetsType = Identity(str = it) }
        }
        semantics.defAssoc(relationshipKind, identification, superclass, sourcesType, sourcesMultiplicity, targetsType, targetsMultiplicity, valueRel)
    }


    /**
     * Identification ["," identification]
     * Semantics: returns a list of identifications that have been parsed.
     */
    private fun parseQualifiedNameList(): MutableList<QualifiedName> {
        val result = mutableListOf<QualifiedName>()
        parseQualifiedName().also { result.add(it) }
        while (token.kind == COMMA) {
            consume(COMMA)
            parseQualifiedName().also { result.add(it) }
        }
        return result
    }


    /**
     * Definition :- Identification "isA" QualifiedName ["from" QualifiedNameList "to" QualifiedNameList]
     */
    private fun parseDefinition() {
        val identification: Identification

        parseIdentification().also { identification = it }
        consume(IS_A)
        when (token.toKind()) {
            ASSOC    -> parseRelationshipDefinition (identification)
            else     -> parseQualifiedName().also { semantics.addClass(identification = identification, superclass = it) }
        }
    }


    /**
     * Definition (";" Definition)*
     */
    private fun parseDefinitionList() {
        parseDefinition()
        while (token.kind == SEMICOLON) {
            consume(SEMICOLON)
            parseDefinition()
        }
    }


    /**
     * An identification, following the conventions of SysML v2 textual:
     *   '<' NAME '>' NAME | NAME '<' NAME '>'
     * This gives SysMD the ability to identify an element either via a unique ID or a path specification.
     */
    open fun parseIdentification(): Identification {
        var shortName: String? = null
        var name: String? = null
        when(token.kind) {
            LT -> {
                nextToken()
                parseSimpleName().also { shortName = it }
                consume(GT)
                if (token.kind == NAME_LIT) {
                    parseQualifiedName().also { name = it }
                }
            }
            NAME_LIT -> {
                parseQualifiedName().also { name = it }
                if (token.kind == LT) {
                    nextToken()
                    parseSimpleName().also { shortName = it }
                    consume(GT)
                }
            }
            else ->
                throw Exception("Syntax Error: Expected identification or qualified name, but read: $token")
//                throw SyntaxError(this, "Expected identification or qualified name, but read: $token")
        }
        return Identification(shortName, name)
    }


    /**
     * A list of Objects, separated by comma and finished by a dot.
     *   ObjectList :- Object (","|";" Object)*
     */
    fun parseElementList() {
        parseElement()
        while (token.kind == COMMA || token.kind == SEMICOLON) {
            consume(COMMA, SEMICOLON)
            parseElement()
        }
    }


    /**
     * A SimpleName, just a name literal
     * SimpleName :- NAME
     **/
    private fun parseSimpleName(): SimpleName {
        if (token.kind != NAME_LIT)
            throw Exception("Syntax Error: Expected name, but read ${toString()}")
//            throw SyntaxError(this, "Expected name, but read ${toString()}")
        val name = token.string
        nextToken()
        return name
    }


    /**
     * Parses a qualified name.
     * QualifiedName :- "NAME("::"NAME)*
     */
    private fun parseQualifiedName(): String {
        if (token.kind != NAME_LIT)
            throw Exception("Syntax Error: Expected name, but read ${toString()}")
//            throw SyntaxError(this, "Expected name, but read ${toString()}")
        var path = token.string
        nextToken()

        while (token.kind == DPDP) {
            nextToken()
            if (token.kind != NAME_LIT)
                throw Exception("Syntax Error: Expected name, but read ${toString()}")
//                throw SyntaxError(this, "Expected name, but read ${toString()}$")
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
        if(consumeIfTokenIs(LCBRACE)) {
            parseIntegerRange().also { multiplicity = it }
            consume(RCBRACE)
        }
        return multiplicity
    }


    /**
     * ValueFeature :- ':' [All|One] OfClass ["(" (Range Unit | true | false ")" ] [ '[' Unit ']' ] [ '=' Expression ]
     */
    private fun parseValueFeature() {
        val multiplicity: Multiplicity = MultiplicityImplementation(valueSpec = IntegerRange(1,1))
        val type: QualifiedName
        var direction = Feature.FeatureDirectionKind.IN

        val identification = parseIdentification()

        consume(DP)

        // Optional multiplicity; default is 1.
        // parseMultiplicity().also { multiplicity = it }

        consumeIfTokenIs(ALL, ONE).also {
            when (consumedToken.kind) {
                ALL -> direction = Feature.FeatureDirectionKind.OUT
                ONE -> direction = Feature.FeatureDirectionKind.IN
                else -> {} // keep default.
            }
        }

        // Type: The type of which the feature shall be a subclass of ...
        parseQualifiedName().also { type = it }

        val feature = semantics.addExpression(identification = identification, multiplicity = multiplicity, className = type, direction = direction)

        parseConstraint().also { feature.valueSpecs = it }

        if (consumeIfTokenIs(LCBRACE)) {
            parseUnit().also { feature.unitSpec(it) }
            consume(RCBRACE)
        }

        if (tokenIs(EQ)) {
            val iBeforeExpression = position
            nextToken()
            parseExpression(type, feature.elementId).also {
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
    private fun parseComponentFeature() {
        val identification = parseIdentification()
        var multiplicity: Multiplicity
        val type: QualifiedName
        var instances: List<QualifiedName> = emptyList()

        consume(DP)
        parseMultiplicity().also { multiplicity = MultiplicityImplementation(valueSpec = it) }
        parseQualifiedName().also { type = it }

        if (consumeIfTokenIs(EQ)) {
            instances = parseQualifiedNameList()
        }
        semantics.hasComponentFeature(semantics.ownerName(), identification, multiplicity, type, instances)
    }

    /**
     * RelationshipFeatureExpression :-
     *          ':' QualifiedNameList<source> QualifiedName QualifiedNameList<target>
     *      |   '=' QualifiedName FROM QualifiedNameList<source> TO QualifiedNameList<target>
     */
    private fun parseAssociation() {
        val identification: Identification
        val source: List<QualifiedName>
        val relationship: QualifiedName
        val target: List<QualifiedName>

        parseIdentification().also { identification = it  }

        if (tokenIs(EQ)) {
            consume(EQ)
            parseQualifiedNameList().also { source = it }
            parseQualifiedName().also { relationship = it }
            parseQualifiedNameList().also { target = it }
        } else if (tokenIs(DP)) {
            consume(DP)
            parseQualifiedName().also { relationship = it }
            consumeIfTokenIs(EQ)    // just optional.
            consume(FROM)
            parseQualifiedNameList().also { source = it }
            consume(TO)
            parseQualifiedNameList().also { target = it }
        } else
            throw Exception("Syntax Error: In Relationship: expected 'from' or name.")
//            throw SyntaxError(this, "In Relationship: expected 'from' or name.")
        semantics.addLink(identification=identification, sources = source, relationship = relationship, targets = target)
    }



    /**
     * IntegerRange :- ConstInt [".." ConstInt]
     */
    private fun parseIntegerRange(): IntegerRange {
        val result = IntegerRange(IntegerRange.Integers)
        parseConstInt().also { result.min = it; result.max = it }
        if (consumeIfTokenIs(DOTDOT)) {
            parseConstInt().also { result.max = it }
        }
        if (result.min > result.max)
            throw Exception("Syntax Error: max of range must be larger or equal min")
//            throw SyntaxError(this, message = "max of range must be larger or equal min")
        return result
    }

    /**
     * ValueRange :- ValueLiteral [.. ValueLiteral]
     **/
    private fun parseValueRange(): Quantity {
        var result: Quantity
        if (tokenIs(INTEGER_LIT)) {
            parseConstInt().also { result = Quantity(model.builder.rangeIDD(it, it)) }
        } else if (tokenIs(FLOAT_LIT)) {
            parseConstReal().also { result = Quantity(model.builder.range(it, it),"?") }
        } else
            throw Exception("Syntax Error: expect value-range of form number .. number")
//            throw SyntaxError(this, "expect value-range of form number .. number")

        if (!tokenIs(DOTDOT)) {
            return result
        }
        else { // ".." ValueLiteral
            consume(DOTDOT)
            if (tokenIs(INTEGER_LIT)) {
                parseConstInt().also {
                    result = when (result.value) {
                        is AADD -> Quantity(model.builder.range(result.aadd().getRange().min, it.toDouble()),"?")
                        is IDD -> Quantity(model.builder.rangeIDD(result.idd().getRange().min, it))
                        else ->
                            throw Exception("Syntax Error: expect range of form [number .. number]")
//                            throw SyntaxError(this, "expect range of form [number .. number]")
                    }
                }
            } else if (tokenIs(FLOAT_LIT)) {
                parseConstReal().also {
                    result = when (result.value) {
                        is AADD -> Quantity(model.builder.range(result.aadd().getRange().min, it),"?")
                        is IDD -> Quantity(model.builder.range(result.idd().getRange().min.toDouble(), it),"?")
                        else ->
                            throw Exception("Syntax Error: expect range of form [number .. number]")
//                            throw SyntaxError(this, "expect range of form [number .. number]")
                    }
                }
            }
            return result
        }
    }

    /** RealSubtype :- ["(" Number [.. ConstValue] | true | false ")"] */
    private fun parseConstraint(): MutableList<Any?> {
        val constraints = mutableListOf<Any?>()
        if (consumeIfTokenIs(LBRACE)) {
            when {
                tokenIs(MINUS, TIMES, INTEGER_LIT, FLOAT_LIT) -> {
                    while(!tokenIs(RBRACE)) { //Multiple constraints for vector
                        val min = parseNumber()
                        val max = optionalWhen(DOTDOT, default = min) {
                            consume(DOTDOT)
                            parseNumber()
                        }
                        constraints.add("$min .. $max")
                        consumeIfTokenIs(COMMA)
                    }
                    consumeIfTokenIs(RBRACE)
                }
                tokenIs(TRUE, FALSE) -> {
                    while(!tokenIs(RBRACE)){
                        if(consumeIfTokenIs(TRUE))
                            constraints.add(XBool.True)
                        else if(consumeIfTokenIs(FALSE))
                            constraints.add(XBool.False)
                    }
                    consumeIfTokenIs(RBRACE)
                }
                else ->
                    throw Exception("Syntax Error: When parsing constraint: expect number range, true, or false, but read $token")
//                    throw SyntaxError(this, "When parsing constraint: expect number range, true, or false, but read $token")
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
    open fun parseExpression(type: QualifiedName? = null, elementID: UUID? = null): AstNode {
        if (type != "Real" && type != "Integer") return parseComparison(null)
        return parseComparison(type, elementID)
    }

    /**
     * conditionalExpression :- IF expression ? expression ELSE expression
     */
    private fun parseConditionalExpression(): AstNode {
        consume(IF)
        val ifExpr: AstNode; val thenExpr: AstNode

        if (token.kind==LBRACE) {
            consume(LBRACE)
            model.reportInfo(semantics.namespace, "Deprecated: if (condition) expr else expr; use: if condition? expr else expr")
        }
        parseExpression().also { ifExpr = it }
        consume(QUESTION, RBRACE)
        parseExpression().also { thenExpr = it }
        consume(ELSE)
        parseExpression().also { return semantics.ifElseExpression(ifExpr, thenExpr, it) }
    }

    /**
     * parseComparison computes an expression and returns the AST as result.
     *
     * Expression :- Sum [ relOp Sum]
     */
    private fun parseComparison(type: QualifiedName? = null, elementID: UUID? = null): AstNode {
        var result = parseSum(type, elementID)
        if (consumeIfTokenIs(GT, LT, EQ, GE, LE, EE)) {
            val op = consumedToken.kind
            val t2 = parseSum()
            result = AstBinOp(result, op, t2)
        }
        return result
    }

    /** Sum :- Product ( ("+"|"-"|"|") Product )*
     * Will also process enumerations on reals and ints */
    private fun parseSum(type: QualifiedName? = null, elementID: UUID? = null): AstNode {
        //FIXME: Ignore douplicate values
        var decVarCounter = 0

        var s1 = parseProduct()
        while (consumeIfTokenIs(PLUS, MINUS, OR)) {
            val op = consumedToken.kind
            s1 = if (type != null) {
                if ((type == "Real" || type == "Integer") && op == OR) {
                    val s2 = parseProduct()
                    if (s1 is AstIte && s1.getLeaves().filter { it.literalVal == (s2 as AstLeaf).literalVal }.isNotEmpty()) continue
                    val cond = AstLeaf(model, Quantity(model.builder.variable("${elementID.toString()}::EnumDecision-$decVarCounter")))
                    semantics.ifElseExpression(cond, s1, s2)
                } else { //no real/int or no "or"
                    val s2 = parseProduct()
                    AstBinOp(s1, op, s2)
                }
            } else { //type == null
                val s2 = parseProduct()
                AstBinOp(s1, op, s2)
            }
            decVarCounter++
        }
        return s1
    }

    /** Product :- Value ( ("*"|"/"|"&") Value )*     */
    private fun parseProduct(): AstNode {
        var f1 = parseExponent()
        while (consumeIfTokenIs(TIMES, DIV, AND, CROSS, DOTProduct)) {
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
        while (consumeIfTokenIs(EXP)) {
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
        return if (consumeIfTokenIs(PLUS, MINUS, NOT)) {
            when (consumedToken.kind) {
                PLUS -> parseValue()
                MINUS -> AstUnaryOp(MINUS, parseValue())
                NOT -> AstNot(model, arrayListOf(parseValue()))
                else ->
                    throw Exception("Syntax Error: Error in unary expression")
//                    throw SyntaxError(this, "Error in unary expression")
            }
        } else
            parseValue()
    }

    /**
     *  Parameter :- "(" Expression ("," Expression)* ")"
     *             |  // nothing.
     */
    private fun parseParameter(): ArrayList<AstNode>? =
        optionalWhen(LBRACE, null) {
            nextToken()
            val parameters = ArrayList<AstNode>()
            while (token.kind != RBRACE) {
                parameters.add(parseExpression())
                while (consumeIfTokenIs(COMMA)) {
                    parameters.add(parseExpression())
                }
            }
            consume(RBRACE)
            parameters
        }


    /**
     *  Unit :-> "%" // Per cent as a unit
     *          | ["1"] (NAME_LIT ["^" INTEGER_LIT])* ["/" (NAME_LIT [^INTEGER_LIT] )+]
     **/
    open fun parseUnit(): String {
        var result = ""

        if (token.kind == PERCENT) {
            nextToken(); return "%"
        }

        // optional "1" , TODO: should be checked ...
        optionalConsume(INTEGER_LIT) { result += consumedToken.number.toInt().toString() + " " }

        while (token.kind == NAME_LIT || token.kind == EURO) {
            nextToken().also { result += consumedToken.toString() }
            optionalConsume(EXP) {
                consume(INTEGER_LIT).also { result += "^${consumedToken.number.toInt()}" }
            }
            result += " "
        }

        optionalConsume(DIV) {
            result += "$consumedToken "
            while (token.kind == NAME_LIT) {
                nextToken().also { result += consumedToken.toString() }
                optionalConsume(EXP) {
                    consume(INTEGER_LIT).also { result += "^${consumedToken.number.toInt()}" }
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
        val neg = consumeIfTokenIs(MINUS) // Sign of negative value.
        when (token.kind) {
            LCBRACE -> {                // Range of kind [number, number] unit
                consume(LCBRACE)
                val quantity: Quantity
                var unit = ""
                parseValueRange().also { quantity = it }
                consume(RCBRACE)
                if (consumeIfTokenIs(LCBRACE)) {
                    parseUnit().also { unit = it }
                    consume(RCBRACE)
                }
                astNode = when(quantity.value){
                    is AADD ->  AstLeaf(model, Quantity(quantity.value as AADD, unit))
                    is IDD ->  AstLeaf(model, Quantity(quantity.value as IDD))
                    is StrDD ->  AstLeaf(model, Quantity(quantity.value as StrDD))
                    is BDD ->  AstLeaf(model, Quantity(quantity.value as BDD))
                    else -> throw SemanticError("Unsupported type for $quantity.")
                }
            }

            FLOAT_LIT -> {              // Floating point literal of kind number unit
                val min = token.number
                var unit = ""
                consume(FLOAT_LIT)
                // optional: Extension to range by
                val max = if (tokenIs(DOTDOT)) {
                    consume(DOTDOT)
                    consume(FLOAT_LIT)
                    consumedToken.number
                } else min

                if (tokenIs(LCBRACE, NAME_LIT, PERCENT)) {
                    if (tokenIs(LCBRACE)) {
                        consume(LCBRACE)
                        parseUnit().also { unit = it }
                        consume(RCBRACE)
                    } else {
                        unit = token.string
                        consume(NAME_LIT)
                    }
                }
                astNode = AstLeaf(model, Quantity(model.builder.range(min, max), unit))
            }

            INTEGER_LIT -> {            // Integer literal
                val min = token.number.toLong()
                consume(INTEGER_LIT)
                // optional: Extension to range by
                val max = if (tokenIs(DOTDOT)) {
                    consume(DOTDOT)
                    consume(INTEGER_LIT)
                    consumedToken.number.toLong()
                } else min
                astNode = AstLeaf(model, Quantity(model.builder.rangeIDD(min, max)))
            }

            STRING_LIT -> {            // A string literal
                astNode = AstLeaf(model, Quantity(StrDD.Leaf(model.builder, token.string)))
                nextToken()
            }

            TRUE -> {               // True literal
                astNode = AstLeaf(model, Quantity(model.builder.True))
                nextToken()
            }

            FALSE -> {              // False literal
                astNode = AstLeaf(model, Quantity(model.builder.False))
                nextToken()
            }
            LBRACE -> {             // ( Expr )
                nextToken()
                var expression = parseExpression()
                try { //test if expression can be used as a Vector
                    val values = mutableListOf<DD>()
                    expression.evalUp()
                    values.add(expression.dd.clone())
                    while (consumeIfTokenIs(COMMA)) {  // Iterate through all vector elements
                        expression = parseExpression()
                        if(expression is AstLeaf || expression is AstUnaryOp) {
                            expression.evalUp()
                            values.add(expression.dd.clone())
                        }else
                            throw SemanticError("Vectors with expression, which are no values, is not allowed")
                    }
                    //Parse Unit
                    consume(RBRACE)
                    var unit = ""
                    if (tokenIs(LCBRACE, NAME_LIT, PERCENT)) {
                        if (tokenIs(LCBRACE)) {
                            consume(LCBRACE)
                            parseUnit().also { unit = it }
                            consume(RCBRACE)
                        } else {
                            unit = token.string
                            consume(NAME_LIT)
                        }
                    }
                    astNode = AstLeaf(model, VectorQuantity(values, Unit(unit)))
                } catch (e:Exception) { // is a simple expression, no vector
                    astNode = expression
                    consume(RBRACE)
                }
            }
            HAS_A -> {              // hasA(partName)
                nextToken()
                consume(LBRACE)
                val ownerName = parseQualifiedName()
                consume(COMMA)
                val ownedName = parseQualifiedName()
                consume(RBRACE).also { astNode = AstHasA(model, semantics.namespace, ownerName, ownedName) }
            }
            IS_A -> {               // isA(typeName)
                nextToken()
                consume(LBRACE)
                val subclassName = parseQualifiedName()
                consume(COMMA)
                val superclassName = parseQualifiedName()
                consume(RBRACE).also { astNode = AstIsA(model, semantics.namespace, subclassName, superclassName) }
            }
            NAME_LIT -> {           // qualifiedName [( parameters )]
                val name = parseQualifiedName()
                val params = parseParameter()
                astNode = if (params == null)
                    AstLeaf(semantics.namespace, name, model)   // an identifier
                else
                    semantics.mkFuncCall(name, params)     // a function call
            }
            IF -> { astNode = parseConditionalExpression() }
            else ->
                throw Exception("Syntax Error: expected value, but read: $token")
//                throw SyntaxError(this, "expected value, but read: $token")
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
            } else AstBinOp(AstLeaf(model, Quantity(model.builder.scalar(0.0),"?")), MINUS, astNode)
        }
        else astNode
    }


    /** A number literal (Int or Float) or a property with known value */
    private fun parseConstReal(): Double {
        val neg = consumeIfTokenIs(MINUS) // Sign of negative value.
        return when (token.kind) {
            INTEGER_LIT,
            FLOAT_LIT -> {           // Number literal
                val value = token.number
                nextToken()
                if (neg) -value else value
            }
            NAME_LIT -> {
                val name = parseQualifiedName()
                val p = semantics.namespace.resolveName<Expression>(name) ?:
                throw Exception("Element not found.")
//                throw ElementNotFoundException(this, name)
                if (neg) -p.rangeSpecs[0].min else p.rangeSpecs[0].min //ConstReal is never a vector
            }
            TIMES ->  {
                nextToken()
                return if(neg) model.settings.minReal else model.settings.maxReal
            }
            else ->
                throw Exception("Semantic Error: Expected real constant (number literal, defined name, or '*'.")
//                throw SemanticError("Expected real constant (number literal, defined name, or '*'.")
        }
    }

    /** A number literal (Int or Float) or a property with known value */
    private fun parseNumber(): String {
        val neg = consumeIfTokenIs(MINUS) // Sign of negative value.
        return when (token.kind) {
            INTEGER_LIT, FLOAT_LIT -> {           // Number literal
                val value = if (token.number.rem(1).equals(0.0))
                    token.number.toLong().toString()  // No ".0" as in Double.toString ...
                else
                    token.number.toString()
                nextToken()
                if (neg) "-$value" else value
            }
            TIMES ->  {
                nextToken()
                "*"
            }
            else ->
                throw Exception("Semantic Error: Expected integer constant (number literal, defined name, or '*'.")
//                throw SemanticError("Expected number literal or '*'.")
        }
    }

    /** A number literal (Int) or a property with known value */
    private fun parseConstInt(): Long {
        val neg = consumeIfTokenIs(MINUS) // Sign of negative value.
        when (token.kind) {
            INTEGER_LIT -> {           // Number literal
                val value = token.number
                nextToken()
                return if (neg) -value.toLong() else value.toLong()
            }
            NAME_LIT -> {
                val name = parseQualifiedName()
                val p = semantics.namespace.resolveName<Expression>(name)
                    ?: throw ElementNotFoundException(this.textualRepresentation, element=null, name=name, token=consumedToken)
                return if (neg) -p.intSpecs[0].min else p.intSpecs[0].min // ConstInt is never a vector
            }
            TIMES -> {
                nextToken()
                return if(neg) model.settings.minInt else model.settings.maxInt
            }
            else ->
                throw Exception("Syntax Error: Expected integer constant (number literal, defined name, or '*'.")
//                throw SyntaxError(this,"Expected integer constant (number literal, defined name, or '*'.")
        }
    }


    /**
     * We allow some flexibility in the preceding keywords of the Element productions;
     * this turns the "keywords" in line with the ISO26262 vocabulary.
     */
    fun Token.toKind(): Token.Kind = when (this.kind) {
        DEF       -> DEF
        PART      -> PART
        PACKAGE   -> PACKAGE
        ATTRIBUTE -> ATTRIBUTE
        ASSOC     -> ASSOC
        NAME_LIT  -> when(string) {
            in setOf("Def")  -> DEF
            in setOf("Component", "Function", "Part", "System", "Software", "Processor", "Feature") -> PART
            in setOf("Package") -> PACKAGE
            in setOf("Value", "Quantity") -> ATTRIBUTE
            in setOf("Relationship", "Relation", "Link") -> ASSOC
            else -> ERROR
        }
        else     -> ERROR
    }


    /**
     * Helper function that checks if the token is start, and if so executes production rule,
     * otherwise it returns default
     */
    private fun <T> optionalWhen(start: Token.Kind, default: T, rule: ParserSysMD.() -> T): T =
        if (token.kind == start) rule() else default


    /**
     * Helper function. If there is the token start in the input stream,
     * consume it and apply the following production rule. The rule produces a result of type T.
     * The default result is given as parameter of the same type.
     */
    private fun <T> optionalConsume(start: Token.Kind, default: T? = null, rule: ParserSysMD.() -> T? = { null }): T? =
        if (this.token.kind == start) {
            consume(start)
            rule()
        } else
            default

    /**
     * Consumes a token and returns its kind.
     */
    private fun consumeToken(): Token.Kind {
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
        while (token.kind != DOT && token.kind != EOF)
            consumeToken()
        consumeToken()
    }
}
