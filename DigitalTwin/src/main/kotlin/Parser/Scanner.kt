package Parser

import ImportedSysMDExceptions.LexicalError
import Parser.Scanner.Definitions.Token.Kind.*
import kotlin.math.pow

/**
 * The scanner gets as input a string and recognizes the main keywords of SysML v2 textual
 * as well as those of SysMD. The input is translated to a stream of tokens that can be
 * read sequentially. The fields
 *  - token
 *  - previousToken
 *  - nextToken
 * contain the current token, the previous and the next token.
 * The function
 * - nextToken(): advances one token forward.
 * - nextTokenIs(Set of TokenKind): checks if a token is present and
 *   advances if the token is in the set of token passed as parameter.
 */
open class Scanner(
    input: String = "",
    var mode: Mode = Mode.SYSMD,
    var debugScanner: Boolean = true,
) {
    enum class Mode { SYSMD, SYSML }

    var token = Token(EOF, "", lineNo = 0, columnNo = 0)
    protected var consumedToken: Token = Token(EOF, "", lineNo = 0, columnNo = 0)
    protected var comment: String = ""
    protected var keywords = if (mode == Mode.SYSML) keywordsSysML else keywordsSysMD

    /** The index of the character where the current token began. */
    private var startPosition = 0

    /** The index of the current character in the overall input string. */
    protected var position = 0

    /** The index of the current character in the line. */
    private var columnNo: Int = 1

    /** The current line number of the scanner incl. lookahead */
    private var lineNo: Int = 1

    /** the line of the current token; will be copied into the Token, once it is accepted. */
    private var tokenLine: Int = 1

    /**
     * The input as a String. Setting it will reset i, lineNo, columnNo, token, etc.:
     * however, it will not change the mode.
     **/
    var input = input           // Giving it a new input resets all states.
        set(it) {
            position = 0
            lineNo = 1
            columnNo = 1
            token = Token(EOF, "", lineNo = 0, columnNo = 0)
            consumedToken = Token(EOF, "", lineNo = 0)
            keywords = if (mode == Mode.SYSML) keywordsSysML else keywordsSysMD
            field = it
            token = nextToken()
        }

    init {
        lineNo = 1
        columnNo = 1
        token =  nextToken()
    }

    /** gets the current character */
    private val curChar: Char
        get() = if (position in input.indices) input[position] else 0.toChar()

    /** gets the next character */
    private val nextChar: Char
        get() = if (position + 1 in input.indices) input[position + 1] else 0.toChar()

    /** moves forward to the next character */
    private fun nextChar(): Char {
        position++
        columnNo++
        return if (position in input.indices) {
            if (curChar == '\n') {
                lineNo++
                columnNo = 1
            }
            curChar
        } else
            0.toChar()
    }

    /**
     * Main API function of the Scanner; reads a token from the input string and
     * modifies position, line, and column.
     * @return the token read.
     */
    fun nextToken(): Token {
        consumedToken = token
        tokenLine = lineNo

        // Skip white space
        while (curChar in WHITESPACE) {
            nextChar()
        }

        // remember where we started.
        startPosition = position

        // Handle token that start with letter or digit
        when {
            // Identifier, path-identifier, or keyword, or Unit
            curChar.isLetter() || curChar == '_' || curChar == '°' -> {
                var string = "" + curChar
                nextChar()
                while (curChar == '_' || curChar.isDigit() || curChar.isLetter()) {
                    string += curChar; nextChar()
                }
                val kind = keywords[string] ?: NAME_LIT
                token = Token(kind, string = string, lineNo = tokenLine, columnNo = columnNo)
                // if (debugScanner) println("scanner token: $this")
                return token
            }

            // Name literal in quotes.
            curChar == '\'' -> {
                nextChar()
                var string = ""
                while (curChar != '\'') {
                    string += curChar
                    nextChar()
                    if (curChar == 0.toChar()) throw LexicalError(this, "Unmatched quotes")
                }
                nextChar()
                token = Token(NAME_LIT, string, lineNo = tokenLine, columnNo = columnNo)
                if (debugScanner) println("scanner token: $this")
                return token
            }

            // Number, either INTEGER or REAL Literal
            curChar.isDigit() -> {
                var kind = INTEGER_LIT
                var mantissa = 0.0
                var fract = 0.0
                var exponent = 0.0
                var esign = 1.0
                while (curChar.isDigit()) {
                    mantissa = mantissa * 10.0 + (curChar - '0').toDouble()
                    nextChar()
                }

                if ((curChar == '.') && (nextChar.isDigit())) {
                    kind = FLOAT_LIT
                    nextChar()
                    var i = 1.0
                    while (curChar in '0'..'9') {
                        i /= 10.0
                        fract += (curChar - '0').toDouble() * i
                        nextChar()
                    }
                }

                if (curChar in setOf('e', 'E')) {
                    kind = FLOAT_LIT
                    nextChar()
                    if (curChar == '-') {
                        esign = -1.0
                        nextChar()
                    }
                    if (curChar == '+')
                        nextChar()
                    while (curChar in '0'..'9') {
                        exponent = exponent * 10.0 + (curChar - '0').toDouble()
                        nextChar()
                    }
                }
                token = buildToken(kind, number = (mantissa + fract) * 10.0.pow(exponent * esign))
                // if (debugScanner) println("scanner token: $this")
                return token
            }
        }

        // Other token
        when (curChar) {
            // String literal: " ... "
            '"' -> {
                nextChar()
                var string = ""
                while (curChar != '"') {
                    string += curChar
                    nextChar()
                    if (curChar == 0.toChar()) throw LexicalError(this, "Unmateched double-quotes")
                }
                nextChar()
                token = Token(STRING_LIT, string, lineNo = tokenLine, columnNo = columnNo)
            }

            // ".", ".."
            '.' -> {
                if (nextChar() == '.') {
                    token = buildToken(DOTDOT)
                    nextChar()
                } else
                    token = buildToken(DOT)
            }

            // "<", "<="
            '<' -> {
                if (nextChar() == '=') {
                    token = buildToken(LE)
                    nextChar()
                } else
                    token = buildToken(LT)
            }

            // >=
            '>' -> {
                if (nextChar() == '=') {
                    token = buildToken(GE)
                    nextChar()
                } else
                    token = buildToken(GT)
            }

            '/' -> {
                when (nextChar()) {
                    '/' -> {
                        val consumedBackup = consumedToken
                        token = buildToken(DIVDIV)
                        while (curChar != '\n' && curChar != 0.toChar())
                            nextChar()
                        token = nextToken()
                        consumedToken = consumedBackup
                    }
                    '*' -> {
                        comment = ""
                        do {
                            comment += nextChar()
                        } while (!((curChar == '*') && (nextChar == '/')))
                        comment = comment.take(comment.length - 1)
                        nextChar()
                        nextChar()
                        token = nextToken()
                    }
                    else -> token = buildToken(DIV)
                }
            }

            '-' -> {
                if (nextChar() == '>') {
                    token = buildToken(ARROW)
                    nextChar()
                } else
                    token = buildToken(MINUS)
            }

            '=' -> {
                if (nextChar() == '=') {
                    token = buildToken(EE)
                    nextChar()
                } else
                    token = buildToken(EQ)
            }

            // : or :> or ::
            ':' -> {
                if (nextChar() == '>') {
                    token = buildToken(SPECIALIZE)
                    nextChar()
                } else
                    if (curChar == ':') {
                        token = buildToken(DPDP)
                        nextChar()
                    } else
                        token = buildToken(DP)
            }


            0.toChar() -> token = buildToken(EOF)

            else -> {
                if (charTokens[curChar] == null) {
                    nextChar()
                    throw LexicalError(this,"Unknown character in input: $curChar ")
                } else {
                    val charToStr = curChar
                    nextChar()
                    token = buildToken(charTokens[charToStr]!!)
                }
            }
        }
        if (debugScanner) println("scanned token: $this")
        return token
    }


    /**
     * Checks if the next token is one of its arguments; if so, the function
     * consumes it and returns true; otherwise, it returns false and does not consume the token.
     * The consumed token is saved in the variable consumedToken.
     * @param accept Token kind that is consumed
     * @return true if the current token was in accept and was consumed; else false.
     */
    protected fun consumeIfTokenIs(vararg accept: Token.Kind): Boolean =
        if (token.kind in accept) {
            nextToken()
            true
        } else
            false

    /**
     * Checks if the token is one of the arguments; if so, the function
     * consumes it and returns it; otherwise, it returns the token.
     * @param accept Token kind that is consumed
     * @return the consumed token kind
     */
    protected fun consumeIf(vararg accept: Token.Kind): Token.Kind {
        for (kind in accept) {
            if (token.kind == kind) {
                nextToken()
                return kind
            }
        }
        return token.kind
    }

    /**
     * Checks if the current token is in the argument.
     * @param options tokens that will be checked
     * @return true, if current token is in options, else false
     */
    protected fun tokenIs(vararg options: Token.Kind): Boolean {
        for (t in options) {
            if (token.kind == t) {
                return true
            }
        }
        return false
    }


    /**
     * Checks for an expected token.
     * If the token is there, it is consumed and the next token is read.
     * If not, it is a syntax error.
     * @param accept expected tokens
     * @return token, if current token is in expected tokens and was consumed
     */
    fun consume(vararg accept: Token.Kind): Token.Kind {
        for (kind in accept) {
            if (token.kind == kind) {
                nextToken()
                return kind
            }
        }
        var acceptStr: String? = null
        accept.forEach { acceptStr = "${acceptStr?:""} '$it'"  }
        throw LexicalError(this, "after '$consumedToken': expected $acceptStr but read '$this' ")
    }

    /**
     * Returns the current token (token) as a string.
     */
    override fun toString() = token.toString()

    /**
     * Builds a Token object with the context information line number, column number, and string that was used.
     * @param kind Kind of the token
     * @param number value, represented as a Double (including integers)
     */
    private fun buildToken(kind: Token.Kind, number: Double = 0.0): Token =
        Token(
            kind = kind,
            string = input.subSequence(startIndex = startPosition, position).toString(),
            number = number,
            lineNo = lineNo,
            columnNo = this.columnNo
        )

    /** Constants for abbreviating some keywords */
    companion object Definitions {

        val WHITESPACE = setOf(' ', '\t', '\n', '\r', 13.toChar())

        val charTokens: HashMap<Char, Token.Kind> = hashMapOf(
            '(' to LBRACE,
            ')' to RBRACE,
            '[' to LCBRACE,
            ']' to RCBRACE,
            '{' to LCURBRACE,
            '}' to RCURBRACE,
            '=' to EQ,
            '+' to PLUS,
            '-' to MINUS,
            '*' to TIMES,
            '^' to EXP,
            '&' to AND,
            '|' to OR,
            '/' to DIV,
            '<' to LT,
            '>' to GT,
            'x' to CROSS,
            '.' to DOT,
            ',' to COMMA,
            ':' to DP,
            ';' to SEMICOLON,
            '#' to HASHTAG,
            '%' to PERCENT,
            '€' to EURO,
            '?' to QUESTION
        )

        val keywordsSysMD = hashMapOf(
            "true" to TRUE,
            "false" to FALSE,
            "has" to HAS_A,
            "hasA" to HAS_A,
            "isA" to IS_A,
            "and" to AND,
            "or" to OR,
            "not" to NOT,
            "cross" to CROSS,
            "dot" to DOTProduct,
            "uses" to USES,
            "imports" to IMPORTS,
            "defines" to DEFINES,
            "all" to ALL,
            "one" to ONE,
            "if" to IF,
            "else" to ELSE,
            "from" to FROM,
            "to" to TO,
            "in" to IN,
            "Package" to PACKAGE,
            "Relationship" to RELATIONSHIP,
            "Feature" to FEATURE,
            "Value" to VALUE,
            "Connector" to CONNECTOR,
            "measurable" to MEASURABLE,
            "input" to INPUT,
            "output" to OUTPUT
            // No keywords , but convenient for implementation
        )

        val keywordsSysML = hashMapOf(
            "Real" to REAL,
            "Boolean" to BOOL,
            "Integer" to INT,
            "String" to STRING,
            "true" to TRUE,
            "false" to FALSE,
            "package" to PACKAGE,
            "value" to ATTRIBUTE,
            "attribute" to ATTRIBUTE,
            "element" to ELEMENT,
            "relationship" to RELATIONSHIP,
            "true" to TRUE,
            "false" to FALSE,
            "and" to AND,
            "or" to OR,
            "part" to PART,
            "specialization" to SPECIALIZE,
            "def" to DEF,
            "from" to FROM,
            "to" to TO,
            "import" to IMPORT,
            "defines" to DEFINES,
            "redefines" to REDEFINES,
            "item" to ITEM,
        )

        /**
         * We represent tokens by this enum. The enum also has properties sVal and nVal in
         * which the string or numeric value of literals is saved.
         **/
        class Token(
            val kind: Kind,
            val string: String,
            val number: Double = 0.0,
            val lineNo: Int = 0,
            val columnNo: Int = 0
        ) {
            enum class Kind {
                // Just for passing information; not really tokens
                ERROR { override fun toString(): String = "" },
                INFO { override fun toString(): String = "" },

                // Literals
                INTEGER_LIT, FLOAT_LIT, STRING_LIT, COMMENT_LIT, NAME_LIT,

                // SysML & SysMD Keywords for Types and Identification
                REAL, BOOL, INT, STRING, IMPORTS, DEFINES, ONE, ALL,

                // SysML Keywords
                ATTRIBUTE, ELEMENT, RELATIONSHIP, DEF, REDEFINES, ITEM, PACKAGE, PART,
                IS_A, HAS_A, IF, ELSE, MEASURABLE, INPUT, OUTPUT,
                SPECIALIZE, FROM, TO, IMPORT, USES, FEATURE, VALUE, CONNECTOR, IN,
                LCURBRACE { override fun toString() = "{" },
                RCURBRACE { override fun toString() = "}" },
                GT { override fun toString() = ">" },
                LT { override fun toString() = "<" },
                GE { override fun toString() = ">=" },
                LE { override fun toString() = "<=" },
                EE { override fun toString() = "==" },
                EOF { override fun toString() = "EOF" },
                DOTDOT { override fun toString() = ".." },
                TRUE { override fun toString() = "true" },
                FALSE { override fun toString() = "false" },
                EQ { override fun toString() = "=" },
                PLUS { override fun toString() = "+" },
                MINUS { override fun toString() = "-" },
                ARROW { override fun toString() = "->" },
                TIMES { override fun toString() = "*" },
                CROSS { override fun toString() = "cross" }, //For cross product
                DOTProduct { override fun toString() = "dot" }, //For cross product
                DIV { override fun toString() = "/" },
                DIVDIV { override fun toString() = "//" },
                DPDP { override fun toString() = "::" },
                EXP { override fun toString() = "^" },
                AND { override fun toString() = "and" },
                OR { override fun toString() = "or" },
                NOT { override fun toString() = "not"},
                LBRACE {override fun toString() = "(" },
                RBRACE {override fun toString() = ")" },
                LCBRACE { override fun toString() = "[" },
                RCBRACE { override fun toString() = "]" },
                DOT { override fun toString() = "." },
                COMMA { override fun toString() = "," },
                DP { override fun toString() = ":" },
                SEMICOLON { override fun toString() = ";" },
                HASHTAG { override fun toString(): String = "#" },
                PERCENT { override fun toString(): String = "%" },
                EURO { override fun toString(): String = "€" },
                QUESTION { override fun toString(): String = "?" }
            }

            override fun toString() = this.string
        }
    }

    private val eof = 0.toChar()
}
