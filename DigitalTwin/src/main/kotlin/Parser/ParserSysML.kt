package Parser

import BaseEntities.*
import BaseEntities.Annotation
import BaseEntities.implementation.AnnotationImplementation
import BaseEntities.implementation.SpecializationImplementation
import BaseEntities.implementation.ValueFeatureImplementation
import ImportedSysMDExceptions.ElementNotFoundException
import ImportedSysMDExceptions.SemanticError
import ImportedSysMDExceptions.SyntaxError
import ImportedSysMDQuantities.Quantity
import ImportedSysMDQuantities.VectorQuantity
import ImportedSysMDServices.AgilaSession
import ImportedSysMDServices.AgilaSessionImpl
import ImportedSysMDServices.resolveName
import Parser.Scanner.Definitions.Token.Kind.*
import Parser.Scanner.Definitions.Token.*
import com.github.tukcps.jaadd.StrDD
import com.github.tukcps.jaadd.values.IntegerRange
import com.github.tukcps.jaadd.values.Range
import com.github.tukcps.jaadd.values.XBool
import SysMDCopyAst.AstBinOp
import SysMDCopyAst.AstLeaf
import SysMDCopyAst.AstNode
import SysMDCopyAst.AstRoot


/**
 * This class provides a parser for the language SysMD.
 * The parser uses the recursive descent method.
 * Examples are in the unit test.
 */
class ParserSysML(
    val model: AgilaSession = AgilaSessionImpl(),
    input: String = ""
) : Scanner(input = input, mode = Mode.SYSML) {
    /**
     * A class that implements the Semantic Actions on the Agila Model.
     */
    private val semanticActions = Semantics(model)
    private val namespace
        get() = semanticActions.namespace


    // The element in the model with the textual representation in SysMD.
    private var textualRepresentation: TextualRepresentation? = null

    // The annotation (=Relationship!) that links the textual model with the
    // generated elements.
    private var generatedElementsAnnotation: Annotation? = null

    /** Constructor for call from notebook with textualRepresentation as integrated code annotation */
    constructor(
        model: AgilaSession,
        textualRepresentation: TextualRepresentation,
    ) : this(model, textualRepresentation.body) {
        require(textualRepresentation.language == "SysML")
        this.textualRepresentation = textualRepresentation
        val exists = textualRepresentation.getOwnedElement("Generated elements")
        if (exists != null) model.delete(exists)
        generatedElementsAnnotation = AnnotationImplementation(
            name = "Generated elements",
            sources = mutableListOf(Identity(ref = textualRepresentation)),
            targets = mutableListOf()
        )
        generatedElementsAnnotation = model.create(generatedElementsAnnotation!!, textualRepresentation)
        semanticActions.generatedElementsAnnotation = generatedElementsAnnotation
    }


    /** Parses the AST of a parsed dependency-expression from a property */
    fun parseDependency(): AstNode {
        nextToken()
        return parseExpression()
    }

    /**
     * Parses a SysML code statement. This is used, if the parser is in the SysML mode.
     */
    fun parseSysML(): AgilaSession {
        parseElements()
        consume(EOF)
        return model
    }


    /**
     * Quantity :-    id ':'
     *                (   "String"                                            ['=' STR_LIT]
     *                  | "Attrib"                                            ['=' STR_LIT]
     *                  | "Real"  ['[' Value [".." Value]  ']'] [Unit]        ['=' Expr]
     *                  | "Integer"   ['[' Value [".." Value]  ']']               ['=' Expr]
     *                  | "Bool"  ['[' ["P" ':' Value] (True | False ) ']']   ['=' CExpr<Bool>]
     *                )
     */
    private fun parseQuantity(): ValueFeature {
        val property: ValueFeature

        parseIdentification().also { property = model.create(ValueFeatureImplementation(name = it.name, shortName = it.shortName), namespace) }
        consume(DP)

        // The different types of a declaration
        when (token.kind) {

            // "string" '=' STR_LIT
            STRING -> {
                consume(STRING).also { model.create(SpecializationImplementation(property, "ScalarValues::String"), property) }
                consume(EQ)
                if (token.kind == STRING_LIT)
                    property.valueSpecs = mutableListOf(model.builder.string(token.string))
                else
                    throw SyntaxError(null, "Expected string literal")
                nextToken()
                semanticActions.hasFeature(property, namespace.qualifiedName)
            }

            //  RealRange [Unit] ['=' CExpr]
            REAL -> {
                model.create(SpecializationImplementation(property, "ScalarValues::Real"), property)
                parseRealRange().also { property.rangeSpec(it) }
                parseUnit().also { property.unitSpec(it) }
                if (consumeIfTokenIs(EQ))
                    parseExpression().also { property.ast = AstRoot(model, property, it) }
                else property.ast = null
                semanticActions.hasFeature(property, namespace.qualifiedName)
            }

            // IntRange ['=' Expr]
            INT -> {
                model.create(SpecializationImplementation(property, "ScalarValues::Integer"), property)
                parseIntSpec().also { property.intSpec(it) }
                if (consumeIfTokenIs(EQ))
                    parseExpression().also { property.ast = AstRoot(model, property, it) }
                else property.ast = null
                semanticActions.hasFeature(property, namespace.qualifiedName)
            }

            // "Bool"  ['(' ( True | False | X ) ')']       ['=' CExpr<Bool>]
            BOOL -> {
                nextToken()
                model.create(SpecializationImplementation(property, "ScalarValues::Boolean"), property)
                parseBoolSubType().also { property.valueSpecs = it }
                property.initVectorQuantity()
                if (token.kind == EQ) {
                    nextToken()
                    val expr = parseExpression()
                    property.ast = AstRoot(model, property, expr)
                } else {
                    property.vectorQuantity = Quantity(model.builder.variable(property.elementId.toString()))
                    when (property.boolSpecs) {
                        XBool.True -> model.builder.conds.constrainVariable(property.elementId.toString(), model.builder.True)
                        XBool.False -> model.builder.conds.constrainVariable(property.elementId.toString(), model.builder.False)
                        XBool.X -> model.builder.conds.constrainVariable(property.elementId.toString(), model.builder.Bool)
                        else -> TODO("cover else also ... ")
                    }
                }
                semanticActions.hasFeature(property, namespace.qualifiedName)
            }

            else -> throw SyntaxError(null,  "Invalid statement at token ${toString()}")
        } // Property is parsed

        // Save everything after equal into dependency string.
        /*
        line.split("=", ignoreCase = false, limit = 2).also {
            if (it.size == 2) property.dependency = it[1]
            else property.dependency = ""
        } */

        return property
    }


    /** IntRange :- "Int" ["(" NUM_LIT [.. NUM_LIT] ")"] */
    private fun parseIntSpec(): IntegerRange {
        consume(INTEGER_LIT)
        optionalWhen(LBRACE, default = IntegerRange(IntegerRange.Integers)) {
            consume(LBRACE)
            val min = parseConstInt()
            val max = optionalWhen(DOTDOT, min) {
                nextToken() // DotDot
                val max = parseConstInt()
                max
            }
            consume(RBRACE)
            IntegerRange(min, max)
        }.also { return it }
    }

    /** RealRange :- "Real" ["(" Value [.. Value] ")"] */
    private fun parseRealRange(): Range {
        consume(REAL)
        optionalWhen(LBRACE, default = Range(Range.Reals)) {
            consume(LBRACE)
            val min = parseConstReal()
            val max = optionalWhen(DOTDOT, default = min) {
                consume(DOTDOT)
                val max = parseConstReal()
                max
            }
            consume(RBRACE)
            Range(min..max)
        }.also { return it }
    }

    /** BoolSubType :- "Bool ["(" ( "True" | "False" ) ")" ] */
    private fun parseBoolSubType(): MutableList<Any?> {
        if (consumeIfTokenIs(LBRACE)) {
            return if (consumeIfTokenIs(TRUE)) {
                consume(RBRACE)
                 mutableListOf(XBool.True)
            } else if (consumeIfTokenIs(FALSE)) {
                consume(RBRACE)
                mutableListOf(XBool.False)
            } else if (consumeIfTokenIs(BOOL)) {
                consume(RBRACE)
                mutableListOf(XBool.X)
            } else throw SyntaxError(null,  "Expected boolean subtype")
        }
        return mutableListOf(XBool.X)
    }

    /**
     * CExpr computes an expression and returns the AST as result.
     *
     * Expression :- Sum [ relop Sum]
     */
    private fun parseExpression(): AstNode {
        var result = parseSum()
        if (consumeIfTokenIs(GT, LT, EQ, GE, LE, EE)) {
            val op = consumedToken.kind
            val t2 = parseSum()
            result = AstBinOp(result, op, t2)
        }
        return result
    }

    /**
     * Sum :- Product ( ("+"|"-"|"|") Product )*
     */
    private fun parseSum(): AstNode {
        var s1 = parseProduct()
        while (consumeIfTokenIs(PLUS, MINUS, OR)) {
            val op = consumedToken.kind
            val s2 = parseProduct()
            s1 = AstBinOp(s1, op, s2)
        }
        return s1
    }

    /**
     * Product :- Value ( ("*"|"/"|"&") Value )*
     */
    private fun parseProduct(): AstNode {
        var f1 = parseValue()
        while (consumeIfTokenIs(TIMES, DIV, AND)) {
            val op = consumedToken.kind
            val f2 = parseValue()
            f1 = AstBinOp(f1, op, f2)
        }
        return f1
    }

    /**
     *     Params   :- "(" CExpr ("," CExpr)* ")"
     *              |  // nothing.
     */
    private fun parseParams(): ArrayList<AstNode>? =
        ifToken(LBRACE) {
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
     *    UNIT_LIT      // "[" UnitId "]", from scanner.
     *  | NAME          // Just a single unit identifier
     *  |               // nothing
     **/
    private fun parseUnit(): String =
        when (token.kind) {
            NAME_LIT -> {
                val unit = token.string
                nextToken()
                unit
            }

            else -> ""
        }


    /**
     * Value :- NUM_LIT | TRUE | FALSE
     * |  '(' ITE ("," ITE)* ')'
     * |  ID that is a constant or variable.
     * |  ID '(' ITE ("," ITE)* ')' where word is a function on AADD.
     */
    private fun parseValue(): AstNode {
        val astNode: AstNode  // Value or expression
        val neg = consumeIfTokenIs(MINUS) // Sign of negative value.
        when (token.kind) {
            REAL -> {               // Real range literal
                val range = model.builder.range(parseRealRange())
                parseUnit().also { astNode = AstLeaf(model, Quantity(range, it)) }
            }

            FLOAT_LIT -> {           // Floating point literal
                val range = model.builder.scalar(token.number)
                nextToken()
                parseUnit().also { astNode = AstLeaf(model, Quantity(range, it)) }
            }

            INTEGER_LIT -> {            // Integer literal
                val range = model.builder.rangeIDD(token.number.toLong())
                nextToken()
                parseUnit().also { astNode = AstLeaf(model, Quantity(range)) }
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
                astNode = parseExpression()
                consume(RBRACE)
            }

            NAME_LIT -> {               // A sequence of identifiers separated by . or ->
                val name = parseQualifiedName()
                val params = parseParams()
                astNode = if (params == null)
                    AstLeaf(namespace, name, model)   // an identifier
                else
                    semanticActions.mkFuncCall(name, params)     // a function call
            }

            else ->
                throw SyntaxError(null,  "expect value, but read: $token")
        }
        return if (neg) {
            if ((astNode is AstLeaf) && astNode.property == null) {
                astNode.literalVal = VectorQuantity(
                    astNode.literalVal!!.negate().values, astNode.literalVal!!.unit.clone(),
                    astNode.literalVal!!.unitSpec
                )
                val remember = astNode.upQuantity.negate().values
                astNode.upQuantity = VectorQuantity(astNode.downQuantity.negate().values, astNode.upQuantity.unit.clone(), astNode.upQuantity.unitSpec)
                astNode.downQuantity = VectorQuantity(remember, astNode.downQuantity.unit.clone(), astNode.downQuantity.unitSpec)
                astNode
            } else AstBinOp(AstLeaf(model, Quantity(model.builder.scalar(0.0), "?")), MINUS, astNode)
        } else astNode
    }

    /**
     * Parses a qualified name of the form "NAME(::NAME)*
     */
    private fun parseQualifiedName(): String {
        if (token.kind != NAME_LIT)
            throw SyntaxError(null,  "Expected name for identifier, but read ${toString()}")
        var path = token.string
        nextToken()

        while (token.kind == DPDP) {
            nextToken()
            if (token.kind != NAME_LIT)
                throw SyntaxError(null,  "Expected name for identifier, but read ${toString()}")
            path += "::" + token.string
            nextToken()
        }
        return path
    }

    /**
     * PropertyId :-  NAME
     *              | NAME "." NAME
     *              | Relation "." NAME "." NAME
     */
    @Suppress("unused")
    private fun parsePropertyPath(): String {
        var path = ""
        when (token.kind) {
            NAME_LIT -> {
                path += token.string
                nextToken()
                if (token.kind != DOT) return path
                nextToken()
                if (token.kind == NAME_LIT) {
                    path += ".${token.string}"
                    nextToken()
                    return path
                }
                return path
            }

            else -> throw SyntaxError(null,  "other relations are todo")
        }
    }


    /** Just a simple NAME Token */
    private fun parseName(): String {
        if (token.kind != NAME_LIT)
            throw SyntaxError(null,  "Expected a name (so not a identifier), but read ${toString()}")
        val n = token.string
        nextToken()

        return n
    }

    /** A number literal (Int or Float) or a property with known value */
    private fun parseConstReal(): Double {
        val neg = consumeIfTokenIs(MINUS) // Sign of negative value.
        when (token.kind) {
            INTEGER_LIT,
            FLOAT_LIT -> {           // Number literal
                val value = token.number
                nextToken()
                return if (neg) -value else value
            }

            NAME_LIT -> {
                val name = parseQualifiedName()
                val p = namespace.resolveName<ValueFeature>(name)
                    ?: throw ElementNotFoundException(name)
                return if (neg) -p.rangeSpecs[0].min else p.rangeSpecs[0].min //no Vectors for Const Real
            }

            TIMES -> {
                nextToken()
                return if (neg) model.settings.minReal else model.settings.maxReal
            }

            else -> throw SemanticError("Expected real constant (number literal, defined name, or '*'.")
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
                val p = namespace.resolveName<ValueFeature>(name)
                    ?: throw ElementNotFoundException(name)
                return if (neg) -p.intSpecs[0].min else p.intSpecs[0].min //no Vectors for constInt
            }

            TIMES -> {
                nextToken()
                return if (neg) model.settings.minInt else model.settings.maxInt
            }

            else -> throw SemanticError("Expected int constant (number literal, defined name, or '*'.")
        }
    }


    private fun parseElements() {
        while (token.kind in setOf(PACKAGE, PART, IMPORT, RELATIONSHIP, ATTRIBUTE)) {
            parseElement()
        }
    }


    private fun parseElement() {
        when (token.kind) {
            PACKAGE -> {
                nextToken()
                val pkg: Namespace
                parseIdentification().also { pkg = semanticActions.declarePackage(it) }
                parseBody(pkg)
            }

            PART -> {
                val identification: Identification
                var clazz: QualifiedName? = null
                var def = false

                nextToken() // PART consumed
                if (token.kind == DEF)
                    consume(DEF).also { def = true } // DEF consumed
                parseIdentification().also { identification = it }  // Identification
                if (token.kind == DP)
                    consume(DP).also {
                        parseQualifiedName().also { clazz = it }
                    }
                val declaredId = semanticActions.definePart(def, identification, clazz)
                // TODO: Add multiplicity to SysML V2 textual parser.
                val multiplicity = parseMultiplicity()
                parseBody(declaredId, multiplicity)
            }

            IMPORT -> {
                nextToken()
            }

            RELATIONSHIP -> {
                nextToken()
            }

            ATTRIBUTE -> {
                nextToken()
                parseQuantity()
                consume(SEMICOLON)
            }

            else -> throw SyntaxError(null,  "Expected package, part, import, relation, attribute but read $token")
        }
    }


    /**
     * Accepts either a simple semicolon or full body.
     */
    private fun parseBody(namespace: Namespace, multiplicity: IntegerRange?=null) {
        when (token.kind) {
            SEMICOLON -> {
                nextToken()
                return
            }
            LCURBRACE -> {
                semanticActions.enterScope(namespace)
                nextToken()
                parseElements()
                consume(RCURBRACE)
                semanticActions.leaveScope()
                return
            }
            else -> throw SyntaxError(null,  "Expect body or semicolon")
        }
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
     * IntegerRange :- ConstInt [".." ConstInt]
     */
    private fun parseIntegerRange(): IntegerRange {
        val result = IntegerRange(IntegerRange.Integers)
        parseConstInt().also { result.min = it; result.max = it }
        if (consumeIfTokenIs(DOTDOT)) {
            parseConstInt().also { result.max = it }
        }
        if (result.min > result.max)
            throw SyntaxError(message = "max of range must be larger or equal min")
        return result
    }


    /**
     * An identification, following the conventions of SysML v2 textual:
     *   < NAME_LITERAL > | NAME_LITERAL
     * We use the qualified name, saved together with the scope as unique identification.
     */
    private fun parseIdentification(): Identification {
        val identification = Identification()
        when (token.kind) {
            LT -> {
                nextToken()
                parseId().also { identification.shortName = it }
                consume(GT)
                return identification
            }

            NAME_LIT -> {
                parseName().also { identification.name = it }
                return identification
            }

            else ->
                throw SyntaxError(null,  "Expected either id or name, but read $token")
        }
    }


    /** Id :- NAME ["." NAME ["." NAME]] */
    private fun parseId(): String {
        if (token.kind != NAME_LIT)
            throw SyntaxError(null,  "Expected name for identifier, but read ${toString()}")
        var path = token.string
        nextToken()

        while (token.kind == DOT) {
            nextToken()
            if (token.kind != NAME_LIT)
                throw SyntaxError(null,  "Expected name for identifier, but read ${toString()}$")
            path += "." + token.string
            nextToken()
        }
        return path
    }


    /**
     * Helper function that checks if the token is start, and if so executes production rule,
     * otherwise it returns default
     */
    private fun <T> optionalWhen(start: Definitions.Token.Kind, default: T, rule: ParserSysML.() -> T): T =
        if (token.kind == start) {
            rule()
        } else
            default


    private fun <T> ifToken(start: Definitions.Token.Kind, default: T? = null, rule: ParserSysML.() -> T): T? =
        if (token.kind == start) {
            rule()
        } else
            default

}
