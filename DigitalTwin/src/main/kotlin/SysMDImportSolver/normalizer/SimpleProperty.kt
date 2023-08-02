package SysMDImportSolver.normalizer

import com.github.tukcps.jaadd.DDcond
import com.github.tukcps.jaadd.values.XBool
import com.github.tukcps.jaadd.values.IntegerRange
import com.github.tukcps.jaadd.values.Range

/**
 * Class to store a simple property.
 */
data class SimpleProperty<T : Any>(
    val name: String,
    val expression : String,
    val dd : DDcond<T>,
    val simpleAst : SimpleAstRoot? = null, // store a simpler version of the AST

    var unitSpec:   String = "",        // Specified unit as string
    var valueSpec:  Any? = null,
) {

    /** access methods for the valueSpec field; returns different types */
    val rangeSpec: Range
        get() = (valueSpec?: Range.Reals) as Range

    val boolSpec: XBool
        get() = (valueSpec?: XBool.X) as XBool

    val intSpec: IntegerRange
        get() = (valueSpec?: IntegerRange.Integers) as IntegerRange

    override fun toString(): String =
        "${this.javaClass.simpleName} { name = $name, " +
                when(valueSpec) {
                     is XBool -> {
                         "type = Boolean, boolSpec = $boolSpec" +
                                 if (expression.isNotEmpty()) ", expression = \"$expression\"" else ""
                    }
                    is IntegerRange -> {
                        "type = Integer, intSpec = $intSpec" +
                                if (expression.isNotEmpty()) ", expression = \"$expression\"" else ""
                    }
                    is Range -> {
                        "type = Real, rangeSpec = $rangeSpec" +
                                (if (unitSpec.isNotEmpty()) ", unitSpec=$unitSpec" else "")+
                                if (expression.isNotEmpty()) ", expression = \"$expression\"" else ""
                    }
                    is String -> {
                        "type = String, dependency=\"$expression\""
                    }

                    else -> {"Unidentified type: $valueSpec"}
                } + ", DD = ${dd.toIteString()} }"

}