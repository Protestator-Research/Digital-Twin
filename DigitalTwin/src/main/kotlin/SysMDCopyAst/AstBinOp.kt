package SysMDCopyAst


import BaseEntities.ValueFeature
import ImportedSysMDExceptions.SemanticError
import ImportedSysMDQuantities.Quantity
import ImportedSysMDQuantities.VectorQuantity
import Parser.Scanner
import Parser.Scanner.Definitions.Token.*
import Parser.Scanner.Definitions.Token.Kind.*
import com.github.tukcps.jaadd.AADD
import com.github.tukcps.jaadd.BDD
import com.github.tukcps.jaadd.IDD
import java.util.*


/**
 * @class AstBinOp
 * A binary operation.
 *
 * @author Christoph Grimm, Jack D. Martin
 */
class AstBinOp(
    val l: AstNode,
    val op: Kind,
    val r: AstNode
) : AstNode(l.model) {

    /** Initialization */
    init {
        l.parent = this
        r.parent = this
    }

    /** Initialization; starts from bottom-up */
    override fun initialize() {
        upQuantity = if (op in setOf(GE, LE, EE, GT, LT, AND, OR)) VectorQuantity(mutableListOf(model.builder.Bool))
        else if (l.isReal) VectorQuantity(mutableListOf(model.builder.Reals),"?")
        else if (l.isInt) VectorQuantity(mutableListOf(model.builder.Integers))
        else VectorQuantity(mutableListOf(model.builder.Reals),"?")
        evalUp()
        downQuantity = upQuantity.clone()
    }

    /** Root is next-higher statement of other kind or null if this is overall root */
    override var root: AstNode? = null
        set(value) {
            field = value; l.root = value; r.root = value
        }

    /** Computes operands from leaves upwards */
    override fun evalUpRec() {
        l.evalUpRec()
        r.evalUpRec()
        evalUp()
    }

    /** Computes one level upwards, from children to parent */
    override fun evalUp() {
        upQuantity = when (op) {
            PLUS -> l.upQuantity + r.upQuantity
            MINUS -> l.upQuantity - r.upQuantity
            TIMES -> l.upQuantity * r.upQuantity
            DIV -> l.upQuantity / r.upQuantity
            CROSS -> l.upQuantity cross r.upQuantity
            DOTProduct -> l.upQuantity dot r.upQuantity
            GT -> l.upQuantity gt r.upQuantity
            LT -> l.upQuantity lt r.upQuantity
            GE -> l.upQuantity ge r.upQuantity
            LE -> l.upQuantity le r.upQuantity
            AND -> l.upQuantity and r.upQuantity
            OR -> l.upQuantity or r.upQuantity
            EXP -> l.upQuantity pow r.upQuantity
            EE -> (l.upQuantity ge r.upQuantity) and (l.upQuantity le r.upQuantity)
            else -> throw SemanticError("Operation $op resp. $op not supported here.")
        }
    }


    /** Computes one level downwards, from parent to children */
    override fun evalDown() {
        val prevL = l
        val prevR = r
        // Only for arithmetic operations; boolean are handled via BDD only bottom-up.

        when (op) {
            AND -> {
                l.downQuantity = downQuantity and r.upQuantity
                r.downQuantity = downQuantity and l.upQuantity
            }

            OR -> {
                // 0 1 -> 1 resp. 0, 1 - 0
                // 1 0 -> 1 resp. 0, 0,- 0
                // 1 1 -> 1 resp. 1, 1, X
                // 0 0 -> 0 resp. 1, 0, 1
                val resultsR = mutableListOf<BDD>()
                val resultsL = mutableListOf<BDD>()
                downQuantity.values.indices.forEach {
                    resultsR.add( downQuantity.values[it].asBdd().ite(
                        l.upQuantity.values[it].asBdd().ite(model.builder.Bool, model.builder.False),
                        l.upQuantity.values[it].asBdd().ite(model.builder.False, model.builder.True)
                    ))
                    resultsL.add( downQuantity.values[it].asBdd().ite(
                        r.upQuantity.values[it].asBdd().ite(model.builder.Bool, model.builder.False),
                        r.upQuantity.values[it].asBdd().ite(model.builder.False, model.builder.True)
                    ))
                }
                r.downQuantity = VectorQuantity(resultsR)
                l.downQuantity = VectorQuantity(resultsL)

            }

            PLUS -> {
                l.downQuantity = downQuantity - prevR.upQuantity
                r.downQuantity = downQuantity - prevL.upQuantity
            }
            MINUS -> {
                l.downQuantity = downQuantity + prevR.upQuantity
                r.downQuantity = prevL.upQuantity - downQuantity
            }
            TIMES -> {
                l.downQuantity = downQuantity / prevR.upQuantity
                r.downQuantity = downQuantity / prevL.upQuantity
            }
            DIV -> {
                l.downQuantity = downQuantity * prevR.upQuantity
                r.downQuantity = prevL.upQuantity / downQuantity
            }
            EXP -> {
                l.downQuantity = when(prevR.upQuantity.values[0]){
                    is AADD -> {
                        val results = mutableListOf<AADD>()
                        downQuantity.values.indices.forEach { results.add(downQuantity.values[it].asAadd() power model.builder.scalar(1.0).div(prevR.upQuantity.values[it] as AADD) ) }
                        VectorQuantity(results,downQuantity.unit,downQuantity.unitSpec)
                    }
                    is IDD ->{
                        val results = mutableListOf<IDD>()
                        downQuantity.values.indices.forEach { results.add(downQuantity.values[it].asIdd().root(prevR.upQuantity.values[it] as IDD) ) }
                        VectorQuantity(results)
                    }
                    else -> throw SemanticError("Expect base of type Real or Integer")
                }
                r.downQuantity = when(prevL.upQuantity.values[0]){
                    is AADD -> {
                        val results = mutableListOf<AADD>()
                        downQuantity.values.indices.forEach {
                            if (prevL.aadds[it].min == 1.0 && prevL.aadds[it].max  == 1.0) //Log with 1 not possible. All resulting values allowed
                                results.add(model.builder.Reals.clone())
                            else
                                results.add((downQuantity.values[it] as AADD).log()/prevL.aadds[it].log())
                        }
                        VectorQuantity(results,downQuantity.unit,downQuantity.unitSpec)
                    }
                    is IDD  -> {
                        val results = mutableListOf<IDD>()
                        downQuantity.values.indices.forEach {
                            if (prevL.idds[it].min == 1L && prevL.idds[it].max  == 1L) //Log with 1 not possible. All resulting values allowed
                                results.add(model.builder.Integers.clone())
                            else
                                results.add((downQuantity.values[it] as IDD).log(prevL.idds[it]))
                        }
                        VectorQuantity(results)
                    }
                    else -> throw SemanticError("Expect base of type Real or Integer")
                }
            }
            EE -> {
                if (prevL.isReal && prevR.isReal) {
                    val results = mutableListOf<AADD>()
                    prevR.aadds.indices.forEach { results.add(prevL.aadds[it] intersect prevR.aadds[it]) }
                    l.downQuantity = VectorQuantity(results, prevL.downQuantity.unit)
                    l.upQuantity = VectorQuantity(results, prevL.upQuantity.unit)
                    r.upQuantity = VectorQuantity(results, prevL.upQuantity.unit)
                    r.downQuantity = VectorQuantity(results, prevL.downQuantity.unit)
                } else if (prevL.isInt && prevR.isInt) {
                    val results = mutableListOf<IDD>()
                    prevR.idds.indices.forEach { results.add(prevL.idds[it] intersect prevR.idds[it]) }
                    l.downQuantity = VectorQuantity(results)
                    l.upQuantity = VectorQuantity(results)
                    r.upQuantity = VectorQuantity(results)
                    r.downQuantity = VectorQuantity(results)
                } else
                    throw SemanticError("Comparison only defined between Int-Int and Real-Real.")
            }

            GT -> {
                if (l.isReal && r.isReal) {
                    val downLs = mutableListOf<AADD>()
                    val downRs = mutableListOf<AADD>()
                    downQuantity.values.indices.forEach {
                        downLs.add(
                            downQuantity.values[it].asBdd().ite(
                                (l.aadds[it] greaterThan r.aadds[it]).ite(l.aadds[it], r.aadds[it]),
                                (l.aadds[it] lessThanOrEquals r.aadds[it]).ite(l.aadds[it], r.aadds[it])
                            )
                        )
                        downRs.add(
                            downQuantity.values[it].asBdd().ite(
                                (l.aadds[it] lessThanOrEquals r.aadds[it]).ite(l.aadds[it], r.aadds[it]),
                                (l.aadds[it] greaterThan r.aadds[it]).ite(l.aadds[it], r.aadds[it])
                            )
                        )
                    }
                    //var downL = downQuantity.bdd().ite(prevL.aadd, prevR.aadd)
                    //var downR = downQuantity.bdd().ite(prevR.aadd, prevL.aadd)
                    l.downQuantity = VectorQuantity(downLs, prevL.upQuantity.unit,prevL.upQuantity.unitSpec)
                    r.downQuantity = VectorQuantity(downRs, prevR.upQuantity.unit,prevR.upQuantity.unitSpec)
                } else if (l.isInt && r.isInt) {
                    val downLs = mutableListOf<IDD>()
                    val downRs = mutableListOf<IDD>()
                    downQuantity.values.indices.forEach {
                        downLs.add(
                            downQuantity.values[it].asBdd().ite(
                                (l.idds[it] greaterThan r.idds[it]).ite(l.idds[it], r.idds[it]),
                                (l.idds[it] lessThanOrEquals r.idds[it]).ite(l.idds[it], r.idds[it])
                            )
                        )
                        downRs.add(
                            downQuantity.values[it].asBdd().ite(
                                (l.idds[it] lessThanOrEquals r.idds[it]).ite(l.idds[it], r.idds[it]),
                                (l.idds[it] greaterThan r.idds[it]).ite(l.idds[it], r.idds[it])
                            )
                        )
                    }
                    //var downL = downQuantity.bdd().ite(prevL.idd, prevR.idd)
                    //var downR = downQuantity.bdd().ite(prevR.idd, prevL.idd)
                    l.downQuantity = VectorQuantity(downLs)
                    r.downQuantity = VectorQuantity(downRs)
                }
            }

            GE -> {
                if (l.isReal && r.isReal) {
                    val downLs = mutableListOf<AADD>()
                    val downRs = mutableListOf<AADD>()
                    downQuantity.values.indices.forEach {
                        downLs.add(
                            downQuantity.values[it].asBdd().ite(
                                (l.aadds[it] greaterThanOrEquals  r.aadds[it]).ite(l.aadds[it], r.aadds[it]),
                                (l.aadds[it] lessThan r.aadds[it]).ite(l.aadds[it], r.aadds[it])
                            )
                        )
                        downRs.add(
                            downQuantity.values[it].asBdd().ite(
                                (l.aadds[it] lessThan r.aadds[it]).ite(l.aadds[it], r.aadds[it]),
                                (l.aadds[it] greaterThanOrEquals r.aadds[it]).ite(l.aadds[it], r.aadds[it])
                            )
                        )
                    }
                    //var downL = downQuantity.bdd().ite(prevL.aadd, prevR.aadd)
                    //var downR = downQuantity.bdd().ite(prevR.aadd, prevL.aadd)
                    l.downQuantity = VectorQuantity(downLs, prevL.upQuantity.unit,prevL.upQuantity.unitSpec)
                    r.downQuantity = VectorQuantity(downRs, prevR.upQuantity.unit,prevR.upQuantity.unitSpec)
                } else if (l.isInt && r.isInt) {
                    val downLs = mutableListOf<IDD>()
                    val downRs = mutableListOf<IDD>()
                    downQuantity.values.indices.forEach {
                        downLs.add(
                            downQuantity.values[it].asBdd().ite(
                                (l.idds[it] greaterThanOrEquals r.idds[it]).ite(l.idds[it], r.idds[it]),
                                (l.idds[it] lessThan r.idds[it]).ite(l.idds[it], r.idds[it])
                            )
                        )
                        downRs.add(
                            downQuantity.values[it].asBdd().ite(
                                (l.idds[it] lessThan  r.idds[it]).ite(l.idds[it], r.idds[it]),
                                (l.idds[it] greaterThanOrEquals r.idds[it]).ite(l.idds[it], r.idds[it])
                            )
                        )
                    }
                    //var downL = downQuantity.bdd().ite(prevL.idd, prevR.idd)
                    //var downR = downQuantity.bdd().ite(prevR.idd, prevL.idd)
                    l.downQuantity = VectorQuantity(downLs)
                    r.downQuantity = VectorQuantity(downRs)
                }
            }

            LT -> {
                if (l.isReal && r.isReal) {
                    /**
                    val downL = downQuantity.bdd().ite(
                        (l.aadd lessThan r.aadd).ite(l.aadd, r.aadd),
                        (l.aadd greaterThanOrEquals r.aadd).ite(l.aadd, r.aadd)
                    )
                    val downR = downQuantity.bdd().ite(
                        (l.aadd greaterThanOrEquals r.aadd).ite(l.aadd, r.aadd),
                        (l.aadd lessThan r.aadd).ite(l.aadd, r.aadd)
                    )**/
                    val downLs = mutableListOf<AADD>()
                    val downRs = mutableListOf<AADD>()
                    downQuantity.values.indices.forEach {
                        downLs.add(downQuantity.values[it].asBdd().ite(prevL.aadds[it], prevR.aadds[it]))
                        downRs.add(downQuantity.values[it].asBdd().ite(prevR.aadds[it], prevL.aadds[it]))
                    }
                    l.downQuantity = VectorQuantity(downLs, prevL.upQuantity.unit, prevL.upQuantity.unitSpec)
                    r.downQuantity = VectorQuantity(downRs, prevR.upQuantity.unit, prevR.upQuantity.unitSpec)
                } else if (l.isInt && r.isInt) {
                    /**
                    val downL = downQuantity.bdd().ite(
                        (l.idd lessThan r.idd).ite(l.idd, r.idd),
                        (l.idd greaterThanOrEquals r.idd).ite(l.idd, r.idd)
                    )
                    val downR = downQuantity.bdd().ite(
                        (l.idd greaterThanOrEquals r.idd).ite(l.idd, r.idd),
                        (l.idd lessThan r.idd).ite(l.idd, r.idd)
                    )**/
                    val downLs = mutableListOf<IDD>()
                    val downRs = mutableListOf<IDD>()
                    downQuantity.values.indices.forEach {
                        downLs.add(downQuantity.values[it].asBdd().ite(prevL.idds[it], prevR.idds[it]))
                        downRs.add(downQuantity.values[it].asBdd().ite(prevR.idds[it], prevL.idds[it]))
                    }
                    l.downQuantity = VectorQuantity(downLs)
                    r.downQuantity = VectorQuantity(downRs)
                }
            }

            LE -> {
                if (l.isReal && r.isReal) {
                    /**
                    val downL = downQuantity.bdd().ite(
                    (l.aadd lessThanOrEquals r.aadd).ite(l.aadd, r.aadd),
                    (l.aadd greaterThan r.aadd).ite(l.aadd, r.aadd)
                    )
                    val downR = downQuantity.bdd().ite(
                    (l.aadd greaterThan r.aadd).ite(l.aadd, r.aadd),
                    (l.aadd lessThanOrEquals r.aadd).ite(l.aadd, r.aadd)
                    )**/
                    val downLs = mutableListOf<AADD>()
                    val downRs = mutableListOf<AADD>()
                    downQuantity.values.indices.forEach {
                        downLs.add(downQuantity.values[it].asBdd().ite(prevL.aadds[it], prevR.aadds[it]))
                        downRs.add(downQuantity.values[it].asBdd().ite(prevR.aadds[it], prevL.aadds[it]))
                    }
                    l.downQuantity = VectorQuantity(downLs, prevL.upQuantity.unit, prevL.upQuantity.unitSpec)
                    r.downQuantity = VectorQuantity(downRs, prevR.upQuantity.unit, prevR.upQuantity.unitSpec)
                } else if (l.isInt && r.isInt) {
                    /**
                    val downL = downQuantity.bdd().ite(
                    (l.idd lessThanOrEquals r.idd).ite(l.idd, r.idd),
                    (l.idd greaterThan r.idd).ite(l.idd, r.idd)
                    )
                    val downR = downQuantity.bdd().ite(
                    (l.idd greaterThan r.idd).ite(l.idd, r.idd),
                    (l.idd lessThanOrEquals r.idd).ite(l.idd, r.idd)
                    )**/
                    val downLs = mutableListOf<IDD>()
                    val downRs = mutableListOf<IDD>()
                    downQuantity.values.indices.forEach {
                        downLs.add(downQuantity.values[it].asBdd().ite(prevL.idds[it], prevR.idds[it]))
                        downRs.add(downQuantity.values[it].asBdd().ite(prevR.idds[it], prevL.idds[it]))
                    }
                    l.downQuantity = VectorQuantity(downLs)
                    r.downQuantity = VectorQuantity(downRs)
                }
            }
            else -> {
                // Else we do nothing; job of Axels solver.
            }
        }
    }


    override fun toExpressionString(): String {
        return l.toExpressionString() + " " + op + " " + r.toExpressionString()
    }

    override fun evalDownRec() {
        evalDown()
        l.evalDownRec()
        r.evalDownRec()
    }

    /**
     * @method evalFromVar
     * @detail re-calculates the result (root) and all other leaf nodes (all other variables) from the variable of
     * interest.  This can proceed from a leaf node, variable.  Capabilities have been extended in this algorithm to
     * allow for calculations beginning at intermediate nodes.
     * @param tok = the token from which to start all calculations
     * NOTE:  Errors in AADD declarations, also evalDownRecHelper is missing due to Java => Kotlin conversion
     * How to get evalDownRecHelper functionality back?  Changed: evalDownRecHelper => evalDownRec
     * parent.id How to get this functionality back?
     * parent.id is the error that remains, again... I think this is supposed to be the lexeme of interest
     * Concern: Prior Java versions used evalDownRecHelper.  Is this broken now?
     * Still not certain how to handle parent.id & sibling.result
     * Also variable.getAADD() method calls require updating syntax
     */
    override fun evalFromVar(tok: UUID) {
        var markedSide: String
        var curr: AstNode? = getSymbolWithLexeme(tok) // here is the missing code
        var sibling: AstNode // can also be leaf!!! or Function!!!
        var parent: AstNode?
        var sibOldVal: AADD// this is extremely necessary, because internal (here) must be different
        try {
            parent = curr?.parent
            if (parent is AstBinOp) {
                if (curr === parent.l) { // if current is on LHS
                    // ToDo: Check. No cast needed.
                    sibling = parent.r
                    sibOldVal = parent.r.aadd // use this for LHS as well?
                    markedSide = "left"
                } else {
                    sibling = parent.l
                    sibOldVal = parent.l.aadd // use this for LHS as well?
                    markedSide = "right"
                } //end if
                //this becomes moot if curr becomes an AstLeaf
                // if (curr is AstBinOp) // if located node has children, evalDown (reverse operations)
                //    curr.evalDownRec()
                // This function is absent after conversion Java => Kotlin, changed this
                // sibling.result needs new syntax, so does parent.id (this should be the lexeme of interest)
                // ToDo: Check what I did: instead of checking if null, just check if AstBinOp Type
                // THis looks troubling!!!
                if (sibling is AstBinOp) {
                    // ToDo: result => value?
                    // parent.id => parent.op,  this should not break
                    sibling.upQuantity = Quantity(calcSibling(parent.aadd, parent.op, curr!!.aadd, markedSide),"")
                    // is a method call of object sibling, not a function!
                    sibling.evalDownRec()
                    // This function is absent after conversion Java => Kotlin, changed this
                } // end if
                // recalculate sibling, if on the bottom of the tree (it is a leaf node)
                // ToDo: is this OK like this?
                // This will work
                if (curr is AstLeaf && sibling is AstLeaf)
                // parent.id => parent.op, this should not break
                    sibling.upQuantity = Quantity(calcSibling(parent.aadd, parent.op, curr.aadd, markedSide),"") //end if (at bottom of tree)
                // first, recalculate the parent node of the bottom two leaf nodes (an operator)
                // this operator must have a change in value to propagate changes through the rest of the tree
                // these do not necessarily need to be bottom two leaf nodes (projecting need for this in the future)
                if (curr?.parent != null) {
                    parent = curr.parent as AstBinOp
                    if (curr === parent.l) { // current node is on LHS
                        markedSide = "left"
                        sibling = parent.r
                    } //end if
                    else { // current node is on RHS (curr == parent.r)
                        markedSide = "right"
                        sibling = parent.l
                    } //end else
                    // parent.result needs new syntax & parent.id also (this should be the lexeme of interest
                    // parent.id => parent.op, this should not break
                    parent.upQuantity = Quantity(calcParent(curr.aadd, parent.op, sibOldVal, markedSide),"")
                } //end if
                // the parent node (an operator) value has been recalculated from curr & sibling
                // with its new value, propagate the change through the tree
                if (curr?.parent is AstBinOp)
                // Next line is trouble
                    curr = curr.parent as AstBinOp //advance up one to that changed operator, then use that as the current node for future calculations
                if (curr?.parent is AstBinOp) {
                    parent = curr.parent as AstBinOp
                    // Again, trouble!!!
                    while (curr is AstBinOp) { // move up, through parent nodes to root
                        if (parent is AstBinOp && curr == parent.r) {
                            sibling = parent.l
                            sibOldVal = parent.l.aadd
                            markedSide = "right"
                        } //end if
                        if (parent != null && parent is AstBinOp && curr == parent.l) {
                            sibling = parent.r
                            sibOldVal = parent.r.aadd
                            markedSide = "left"
                        } //end if
                        if (parent is AstBinOp) { // if (curr.parent != null)
                            // ToDo: result => value OK?
                            // parent.id => parent.op, this should not break
                            sibling.upQuantity = Quantity(calcSibling(parent.aadd, parent.op, curr.aadd, markedSide),"")
                            parent.upQuantity = Quantity(calcParent(curr.aadd, parent.op, sibOldVal, markedSide),"")
                            // ToDo: check
                            if (sibling is AstBinOp)
                            // ToDo: as method call???
                                sibling.evalDownRec() // this helper function is missing after Java => Kotlin conversion, changed this
                            curr = curr.parent as AstBinOp // Trouble!!!
                            parent = if (curr.parent != null) curr.parent as AstBinOp // this is an operator
                            else break
                        } //end if
                        else break
                    } //end while
                } // end if
            } // end if
        } //end try
        catch (ee: SemanticError) { // need an error handler here, I used something called an ExprError
            ee.printStackTrace()
        } // end catch
    } // end method evalFromVar


    /**
     * @method calcParent
     * @detail AADD is not the new data type?  Need to change these var declarations, and AADD.newLeaf
     * @param currVal
     * @param op
     * @param sibOldVal
     * @result AADD parentResult
     */
    private fun calcParent(currVal: AADD, op: Scanner.Definitions.Token.Kind, sibOldVal: AADD, markedSide: String): AADD {
        // ToDo --> we need to use factory now.
        var parentResult = model.builder.scalar(0.0)
        if (markedSide.compareTo("left") == 0) { // current is on LHS
            when (op) {
                PLUS -> parentResult = currVal.plus(sibOldVal)
                MINUS -> parentResult = currVal.minus(sibOldVal) // unique
                TIMES -> parentResult = currVal.times(sibOldVal)
                DIV -> parentResult = currVal.div(sibOldVal) // unique
                else -> {}
            }
        } //end if
        else { // current is on RHS
            when (op) {
                PLUS -> parentResult = sibOldVal.plus(currVal)
                MINUS -> parentResult = sibOldVal.minus(currVal) // unique
                TIMES -> parentResult = sibOldVal.times(currVal)
                DIV -> parentResult = sibOldVal.div(currVal) // unique
                else -> {}
            }
        } //end else
        return parentResult
    } //end method calcParent

    /**
     * @method calcSibling
     * @detail AADD is not the new data type?  Need to change these var declarations, and AADD.newLeaf
     * @param parentVal
     * @param op
     * @param markedSide
     * @return AADD siblingResult
     */
    private fun calcSibling(parentVal: AADD, op: Scanner.Definitions.Token.Kind, currVal: AADD, markedSide: String): AADD {
        val siblingResult: AADD
        if (markedSide.compareTo("left") == 0) {
            siblingResult = when (op) {
                PLUS -> parentVal.minus(currVal)
                MINUS -> currVal.minus(parentVal)
                TIMES -> parentVal.div(currVal)
                DIV -> currVal.div(parentVal)
                else -> throw InternalError("jAADD: Internal error")
            }
        } //end if
        else { // markedSide == Right
            siblingResult = when (op) {
                PLUS -> parentVal.minus(currVal)
                MINUS -> parentVal.plus(currVal)
                TIMES -> parentVal.div(currVal)
                DIV -> parentVal.times(currVal)
                else -> throw InternalError("jAADD: Internal error")
            }
        } //end else
        return siblingResult
    } //end method calcSibling

    /**
     * @method getSymbolWithLexeme
     * @param lexeme = the target lexeme to search for
     * @return
     */
    private fun getSymbolWithLexeme(lexeme: UUID): AstLeaf {
        // method only works for old "1-ast" assumption.
        // needs to fix fore newer method.
        return (model.get(lexeme)!! as ValueFeature).ast!!.leaves.first()
    } //end method getSymbolWithLexeme


    /** Executes a lambda on each AstNode in an Ast and returns its result */
    override fun <R> runDepthFirst(block: AstNode.() -> R): R {
        l.runDepthFirst(block)
        r.runDepthFirst(block)
        return this.run(block)
    }

    /** Executes a lambda on each AstNode in an Ast and returns its result */
    override fun <R> withDepthFirst(receiver: AstNode, block: AstNode.() -> R): R =
        with(receiver) {
            withDepthFirst(l, block)
            withDepthFirst(r, block)
            return block()
        }

    override fun toString() = "AstBinOp($l $op $r)"
    override fun clone(): AstBinOp = AstBinOp(l.clone(), op, r.clone())
}
