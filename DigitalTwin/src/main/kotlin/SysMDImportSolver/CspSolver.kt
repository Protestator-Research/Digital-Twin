package SysMDImportSolver

import BaseEntities.ValueFeature
import ImportedSysMDServices.AgilaSession
import Parser.parseDependency
import SysMDCopyAst.AstRoot


/**
 * @author Christoph Grimm, Jack D. Martin
 *
 * The class CspSolver implements methods that compute the dependencies among properties
 * in different directions and with different methods, depending on changes and type of
 * variables.
 *
 * The class requires the following data:
 * - The symbol table that holds all property variables.
 */
class CspSolver(val model: AgilaSession) {
    // Access to the relevant API via the appelModel:
     val builder = model.builder

    /**
     * a list of dependent variable identifiers (strings) for the current set of expressions being evaluated
     */
    var depvaridlist: MutableList<String> = mutableListOf()

    /**
     * The current ast, a shared object
     */
    lateinit var ast : AstRoot

    /*
    /**
     * A collection of Abstract Syntax Trees (ASTs)
     * Each AST corresponds to a parsed mathematical expression.
     * Like the localsymtabs above, this has a (K, V) pair, the key being the identity of the dependent variable.
     * (K, V) = (id, AST)
     */
    var ASTs : LinkedHashMap<UUID, AstRoot> = LinkedHashMap()

    /**
     * The key, String, is a property/variable name
     * the MutableList<> is a list of occurrences, in which ASTs, the property/variable occurs
     * This would be: (K, V) = (id, List<dependent-vars>) specifying which variables´ values can be calculated using that particular independent variable
     * NOTE:  This corresponds to  (K, V) = (independent-variable-name, dependent-variable-name-list)
     * e.g. r = a + b - c * d / ee, where r = result (dependent var) & {a, b, c, d, ee} is a set of independent variables within a dependency expression
     */
    var occurrencemap : HashMap<UUID, MutableList<UUID>> = HashMap()

    /**
     * This would be: (K, V) = (dep-var, List<independent-vars>)
     * variable
     */
    var leafmap : HashMap<UUID, List<AstLeaf>> = HashMap()

    /**
     * A Map of a ValueFeature Name (a String & the key), which is the calculation result & dependent variable, to another string, which is the dependency
     * expression.
     * In shorthand, (K, V) = (String, String) = (id, dependency-expr) = (dependent-var, independent-vars)
     * This will allow parsing and storage of multiple dependency expressions.
     */
    var exprmap : HashMap<UUID, UUID> = HashMap()

    // var invexprmap : HashMap<String, String> = HashMap()

    /**
     * (K, V) = (String, ValueFeature) = (id, val) = (dep-var-id, dep-var-val)
     */
    var depvarval : HashMap<UUID, ValueFeature> = HashMap()

    //val discSolver: DiscreteSolverIF by lazy {DiscreteSolver(this)} // = DiscreteSolver(this) //FIXME: Correct ctor needed

    /**
     * Takes a property having an initial calculation (having previously been evaluated), then...
     * Given a changed independent variable (variable in a dependency expression), it calculates the new value of the dependent variable (result)
     * AND it calculates new values for all other independent variables in the dependency expression
     * Then...
     * It takes one of the new re-calculated values for one of the variables, and treats that as the new changed independent variable.  It proceeds to
     * perform this calculation repeatedly for all of the independent variables in the dependency expression.
     *
     * @param depvar : ValueFeature?
     * @param VoI : ValueFeature?
     * @param newValOfVoI : DD
     * @return DD
     */
    fun calcDepVarValfromSerialDeltaofIndepVars(depvar: ValueFeature, VoI: ValueFeature, newValOfVoI: DD) : DD {
        val leaflist : List<AstLeaf>?
        lateinit var updatedVoI : ValueFeature

        ast = model.global.resolveName<ValueFeature>(depvar.name!!)!!.ast!!

        // simply setting the value
        if (newValOfVoI is AADD) {
            updatedVoI = ValueFeatureImplementation(VoI.name!!, Quantity(newValOfVoI.asAadd(),""))
            model.letVar(updatedVoI.name!!, updatedVoI.aadd()) // updates the value in the symbol table of the AST
        }
        if (newValOfVoI is IDD) {
            updatedVoI = ValueFeatureImplementation(VoI.name!!, Quantity(newValOfVoI.asIdd()))
            model.letVar(updatedVoI.name!!, updatedVoI.idd()) // updates the value in the symbol table of the AST
        }
        // finished setting the value

        // algorithm really begins here
        ast.evalFromVar(updatedVoI.id!!) // will update the values of all independent variables in dependency expression & dependent
        // variable (root)
        leaflist = ast.getLeaves().toList() // returns a list of all dependency variables in the expression (actually, its AST)
        for (currVoI in leaflist)  // re-evaluate values for variables in AST as each new leafnode/variable is encountered
            if (currVoI.property != null)
                ast.evalFromVar(currVoI.property!!.id!!)

        return ast.dependency.upQuantity.value
    } // end fxn. calcIdValfromSerializedDeltaofIndepVars

    /**
     * Takes a property with an initial calculation, then...
     * Given a changed independent variable (variable in a dependency expression), it calculates the new value of the dependent variable (result)
     * AND it calculates new values for all other independent variables in the dependency expression
     * Then...
     * It takes one of the new re-calculated values for one of the variables, and treats that as the new changed independent variable.  It proceeds to
     * perform this calculation repeatedly for all the independent variables in the dependency expression.
     *
     * @param depvar : ValueFeature?
     * @param VoI : ValueFeature?
     * @param newValOfVoI : DD
     * @return DD
     */
    fun calcDepVarValfromSerialDeltaofIndepVars(fullqualprefix: String, depvar : ValueFeature, VoI : ValueFeature, newValOfVoI : DD) : DD {
        val leaflist : List<AstLeaf>?
        lateinit var updatedVoI: ValueFeature
        val fullqualname : String = fullqualprefix + VoI.name!!
        /*
         Must determine which AST contains the VoI, which is not necessarily the root of an AST.
         Then, get its key (the root of some algebraic expression).
         Having found the key, return the appropriate AST.
         Passing in the VoI here is an error because it may not return the correct AST, or it may return null.
         So, perform the search for the VoI first, identify its key (root), & return the AST per previous discussion
         */
        ast = ASTs[depvar.id!!]!! // MUST be corrected!  Requires more searching before this statement.

        if (newValOfVoI is AADD) {
            updatedVoI = ValueFeatureImplementation(VoI.name!!, Quantity(newValOfVoI.asAadd(),"?"))
            model.letVar(fullqualname, updatedVoI.aadd()) // updates the value in the symbol table of the AST
        }
        if (newValOfVoI is IDD) {
            updatedVoI = ValueFeatureImplementation(VoI.name!!, Quantity(newValOfVoI.asIdd()))
            model.letVar(fullqualname, updatedVoI.idd()) // updates the value in the symbol table of the AST
        }
        ast.evalFromVar(updatedVoI.id!!) // will update the values of all independent variables in the dependency expression and the
        // dependent variable (root)
        leaflist = ast.getLeaves().toList() // returns a list of all dependency variables in the expression (actually, its AST)
        for (currVoI in leaflist)
            ast.evalFromVar(updatedVoI.id!!)

        return ast.property.quantity.value
    } // end fxn. calcIdValfromSerializedDeltaofIndepVars

    /**
     * Evaluates the dependency string associated with a property and updates its values bi-directionally
     * TODO: Make certain that all independent variables have values before evaluating
     * @return ValueFeature
     */
    fun eval(id : UUID, iters : Int) : ValueFeature {
        val prop = model.get(id) as ValueFeature
        lateinit var result: DD

        if (prop == null)
            throw SemanticError("ValueFeature not found in symbol table.")
        // ASTs[id] = parser.parseDependency(id, model.resolveName<ValueFeature>(id]!!.dependency!!)
        ast = ASTs[id]!!
        if (prop.ofClass?.ref?.qualifiedName in setOf("ScalarValues::Real", "ScalarValues::Performance", "ScalarValues::Integer")) {
            for (i in 1..iters) {
                ast.evalUp()
                result = ast.property.quantity.value
                prop.quantity.value = result
                ast.evalDown()
            }
        }

        model.create(prop, prop.owner.ref as Namespace)  // maybe, no longer correct!

        return prop
    } // end fxn. eval */


    /** Evaluates the dependency string in the property and updates its values top-down */
    @Deprecated("Replace with value.evalDown()", ReplaceWith("value.evalDown()"))
    fun evalDown(value: ValueFeature) {
        if ( value.ast == null )
            value.parseDependency()
        value.ast?.evalDownRec()
    }

    /**
     * Evaluates the dependency in the property with key and updates its values bottom up.
     * If the AST is not yet built, it is parsed from the dependency expression string.
     */
    @Deprecated("Replace with value.evalUp()", ReplaceWith("value.evalUp()"))
    fun evalUp(value: ValueFeature): ValueFeature {

        if (value.dependency.isEmpty()) {
            return value
        }

        if (value.ast == null)
            value.parseDependency()

        value.ast?.evalUpRec()
        return value
    }


    /*
    /** Updates all values that depend on the property with the id/name key */
    fun evalFromVar(value: ValueFeature, keyLeaf: UUID) {
        if ( value.ast == null ) {
            value.parseDependency()
        }
        value.ast?.evalFromVar(keyLeaf)
    }

    /**
     * eval methods for 1+ expressions that the user wishes to evaluate
     * presumes the following:
     * (1) the expressions must be loaded into some type of collection (list), from where is irrelevant
     * (2) the expressions must all have the correct syntax, currently something like this: "ValueFeature I: Real = V / R"
     * Formerly: "context y: derive: a-b+c*d/e"
     * (3) all variables must have initial values from which to calculate a result
     */
    fun evalAllExpressions()  {
        for ((k, _) in exprmap)
            depvarval.putIfAbsent(k, model.get(k) as ValueFeature)
    } // end fxn evalAllExpressions

    /**
     * evaluates a set of expressions via the depvarids (list of dep-var-ids) passed in
     */
    fun evalExpressions(depvarids : MutableList<UUID>) {
        lateinit var prop : ValueFeature

        for (elem in depvarids) {
            if (ASTs[elem] != null) {
                ast = ASTs[elem]!!
                prop = model.get(elem) as ValueFeature
                ast.evalUp()
                prop.quantity.value = ast.property.quantity.value
                depvarval[elem] = prop
                model.create(prop, prop.owner.ref as Namespace)
            }
        }
    } // end fxn evalAllExpressions

    /** Updates all values that depend on the property with the key(id) */
    fun evalFromVar(id : UUID) {
        if (ASTs[id] != null) {  // if the id corresponds to a dep-var, then update based upon that dep-var's AST
            ast = ASTs[id]!!
            ast.evalFromVar(id)
        }
        for ((indepvar, depvars) in occurrencemap)
            if (indepvar == id)
                for (depvar in depvars) {
                    ast = ASTs[depvar]!!
                    ast.evalFromVar(id)
                    // depvarval[depvar] = ast?.root!!.prop!!
                    depvarval[depvar] = ast.property
                }
    }

    /**
     * Same as fxn: "calcIdValfromSerializedDeltaofIndepVars", except it takes a dependency expression as a parameter instead of
     * a property / variable
     */
    fun evalExprBasedonSerializedDeltaofIndepVars(@Suppress("UNUSED_PARAMETER") expr : String, VoI : ValueFeature, newValOfVoI : DD) : DD {
        val leaflist : List<AstLeaf>?
        lateinit var updatedVoI : ValueFeature
        /*
         Must determine which AST contains the VoI, which is not necessarily the root of an AST.
         Then, get its key (the root of some algebraic expression).
         Having found the key, return the appropriate AST.
         Passing in the VoI here is an error because it may not return the correct AST, or it may return null.
         So, perform the search for the VoI first, identify its key (root), & return the AST per previous discussion

         WARNING: Type inference failed. The value of the type parameter K should be mentioned in input types
         (argument types, receiver type or expected type).
         Try to specify it explicitly.
         */
        ast = ASTs[VoI.id!!]!! // MUST be corrected!  Requires more searching before this statement.

        if (newValOfVoI is AADD) {
            updatedVoI = ValueFeatureImplementation(VoI.name!!, Quantity(newValOfVoI.asAadd(),""))
            model.letVar(updatedVoI.name!!, updatedVoI.aadd()) // updates the value in the symbol table of the AST
        }
        if (newValOfVoI is IDD) {
            updatedVoI = ValueFeatureImplementation(VoI.name!!, Quantity(newValOfVoI.asIdd()))
            model.letVar(updatedVoI.name!!, updatedVoI.idd()) // updates the value in the symbol table of the AST
        }
        ast = model.global.resolveName<ValueFeature>(VoI.name!!)!!.ast!!
        ast.evalFromVar(updatedVoI.id!!) // will update the values of all independent variables in the dependency expression and the dependent variable (root)
        leaflist = ast.getLeaves().toList() // returns a list of all dependency variables in the expression (actually, its AST)
        for (currVoI in leaflist)
            ast.evalFromVar(currVoI.property!!.id!!)

        return ast.upQuantity.value
    } // end fxn. evalExprBasedonSerializedChangesofIndependentVars

    /**
     * Insert the (K, V) pair into the AST Hashmap
     * (K, V) = (identifier, expression)
     * 'stmt' should have parsed it to an identifier & its dependency expression
    // The identifier corresponds to the 'root' or AstRoot node, which is sought here
     */
    private fun insertExprIntoASTMap(id: UUID) {
        ASTs.putIfAbsent(id, (model.get(id) as ValueFeature).ast!!)
    }

    /**
     * Insert the (K, V) pair into the AST Hashmap
     * (K, V) = (identifier, expression)
     * 'stmt' should have parsed it to an identifier & its dependency expression
     * The identifier corresponds to the 'root' or AstRoot node, which is sought here
     */
    private fun insertExprIntoASTMap(name: UUID, fullqualprefix: String) {
        ASTs.putIfAbsent(name, model.global.resolveName<ValueFeature>(fullqualprefix + name)!!.ast!!)
    }

    /**
     * Insert the (K, V) pair into the AST Hashmap
     * (K, V) = (identifier, expression)
     * 'stmt' should have parsed it to an identifier & its dependency expression
    // The identifier corresponds to the 'root' or AstRoot node, which is sought here
     */
    private fun insertExprIntoASTMap(name: UUID, fullqualprefix: String, model: AgilaSession) {
        ASTs.putIfAbsent(name, model.resolveName<ValueFeature>(model.global,fullqualprefix + name)!!.ast!!)
    }

    /**
     * load the expression map, (K, V) = (String, String) = (dep-var, indep-vars) = (dep-var, expr)
     * e.g.: ValueFeature <property-name> : <property-type> = <mathematical expression>
     */
    fun loadExprMap(stmt : String) {
        val regex = Regex("(?=[:=])|(?<=[:=])")
        val mathstmt = stmt.removePrefix("ValueFeature")
        val depvarandexpr = mathstmt.split(regex).toTypedArray()
        val limit = depvarandexpr.size - 1

        for (i in 0..limit)
            depvarandexpr[i] = depvarandexpr[i].replace("\\s".toRegex(), "")
        exprmap.putIfAbsent(depvarandexpr[0], depvarandexpr[4])
    }// end fxn. loadExprMap

    /**
     * takes a list of mathematical statements, parses them and inserts them into a Hashmap of ASTs
     */
    fun loadAllMathStmts(mathstmts : MutableList<String>) {
        for(mathstmt in mathstmts)
            loadExprMap(mathstmt)
        for ((key, _) in exprmap)
            insertExprIntoASTMap(key)
        for ((k, _) in ASTs)
            depvaridlist.add(k)
    } // end fxn. loadAllMathStmts

    /**
     * takes a list of mathematical statements, parses them and inserts them into a Hashmap of ASTs
     */
    fun loadAllMathStmts(mathstmts: MutableList<String>, fullqualprefix: String) {
        for(mathstmt in mathstmts)
            loadExprMap(mathstmt)
        for ((k, _) in exprmap)
            insertExprIntoASTMap(k, fullqualprefix)
        for ((k, _) in ASTs)
            depvaridlist.add(k)
    } // end fxn. loadAllMathStmts

    /**
     * takes a list of mathematical statements, parses them and inserts them into a Hashmap of ASTs
     */
    fun loadAllMathStmts(mathstmts: MutableList<String>, fullqualprefix: String, model: AgilaSession) {
        for(mathstmt in mathstmts)
            loadExprMap(mathstmt)
        for ((k, _) in exprmap)
            insertExprIntoASTMap(k, fullqualprefix, model)
        for ((k, _) in ASTs)
            depvaridlist.add(k)
    } // end fxn. loadAllMathStmts

    /**
     * loads all properties into ASTs
     */
    fun loadAllPropertiesIntoASTs(propertylist : MutableList<ValueFeature>) {
        for (elem in propertylist)
            insertExprIntoASTMap(elem.name!!)
        for ((id) in ASTs)
            depvaridlist.add(id)
    } // end fxn. loadAllPropertyASTs

    /**
     * Populates a list of occurrences with the ASTs in which that actual variable occurs
     * the MutableList<> is a list of occurrences, in which ASTs, the property/variable occurs
     * This would be: (K, V) = (id, List<dependent-vars> specifying which variables´ values can be calculated using that particular ValueFeature/variable
     * NOTE:  This corresponds to  (K, V) = (independent-variable-name, dependent-variable-name-list)
     * OLD: fun populateOccurrenceList(occlist : HashMap<String, MutableList<String>> ) : HashMap<String, MutableList<String>> {
     */
    fun populateLeafMap(depvarlist : MutableList<String>) {
        for (depvarid in depvarlist) {
            ast = ASTs[depvarid]!!
            /*if (ast != null)
               leafmap[depvarid] = ast.getLeaves().toList() as MutableList<AstLeaf>*/

            /*
           print("uncleanleaflist(" + depvarid + ") = [")
           for (i in 1 .. uncleanleaflist.size - 1) {
               if (uncleanleaflist[i].identification!!.name != null)
                   print(uncleanleaflist[i].identification!!.name)
               if (uncleanleaflist[i] != uncleanleaflist.last())
                   print(", ")
           }
           println("]")
           */
            var uncleanleaflist: MutableList<AstLeaf>
            val cleanleaflist : MutableList<AstLeaf> = mutableListOf()
            uncleanleaflist = ast.getLeaves().toList() as MutableList<AstLeaf>
            for (elem in uncleanleaflist)
               if (elem.qualifiedName != null && elem.qualifiedName != "")
                   cleanleaflist.add(elem)
            leafmap[depvarid] = cleanleaflist
        }
    } //end fxn. populateLeafMap

    /**
     * Populates a list of occurrences with the ASTs in which that actual variable occurs
     * the MutableList<> is a list of occurrences, in which ASTs, the property/variable occurs
     * This would be: (K, V) = (id, List<dependent-vars>) specifying which variables´ values can be calculated using that particular ValueFeature/variable
     * NOTE:  This corresponds to  (K, V) = (independent-variable-name, dependent-variable-name-list)
     */
    fun populateOccurrenceMap() {
        for ((depvarid, indepvars) in leafmap) { // loop through hashmap of (depvarids, list<indepvars>)
            for (indepvar in indepvars) { // loop through list of independent variables
                var depvarlist : MutableList<String> = mutableListOf() // a list of dependent vars for whom a particular independent var exists
                // check if the independent var is in the occurrence map,
                // if it is NOT, then build the list
                // if (occurrencemap[indepvar.identification!!.name] == null && !validateNumericPattern(indepvar.identification!!.name)) {
                if (occurrencemap[indepvar.qualifiedName] == null) {
                    if (!depvarlist.contains(depvarid)) // check to see if the depvarid is in the depvar list, if NOT
                        depvarlist.add(depvarid) // add it to the depvar list
                    occurrencemap.putIfAbsent(indepvar.qualifiedName!!, depvarlist) // update the occurrence map
                } // end if
                else { // it EXISTS in the occurrence map
                    if (occurrencemap[indepvar.qualifiedName] != null) {
                        depvarlist = occurrencemap[indepvar.qualifiedName]!!
                        if (!depvarlist.contains(depvarid)) // check to see if the depvarid is in the depvar list, if NOT
                            depvarlist.add(depvarid) // add it to the depvar list
                        occurrencemap[indepvar.qualifiedName!!] = depvarlist
                    }
                } // end else the independent variable already EXISTS in the occurrence map
            } // end for
        } // end for
    } //end fxn. populateOccurrenceMap

    /**
     * @function printRangeVar
     * @detail This function is a wrapper for print statements, allowing printing of Range Variables with shortened syntax
     * @param qualifiedName : String = the variable identifier
     * This method is very convenient!
     * Use it frequently!
     */
    fun printRangeVar(qualifiedName : QualifiedName) {
        when (model.global.resolveName<ValueFeature>(qualifiedName)?.quantity?.value) {
            is AADD -> println(qualifiedName + " = [" + model.global.resolveName<ValueFeature>(qualifiedName)!!.aadd().getRange().min + ", " + model.global.resolveName<ValueFeature>(qualifiedName)!!.aadd().getRange().max + "]")
            is IDD -> println(qualifiedName + " = [" + model.global.resolveName<ValueFeature>(qualifiedName)!!.idd().getRange().min + ", " + model.global.resolveName<ValueFeature>(qualifiedName)!!.idd().getRange().max + "]")
            else -> println("Could not print $qualifiedName")
        }
    } // end fxn. printRangeVar

    /**
     * function: reEvalAllVarsDependentUponVoI
     * 1st GCNA!
     * This method evaluates a set of expressions with some value or values in common.  This can be a chain of expressions where the dependent variable of one
     * becomes an independent in another expression, &c.
     * 1) Requires a changed variable in one of the expressions, thus (2) & (3)
     * 2) A property/variable that was changed must have been identified, named here the Variable of Interest (VoI)
     * 3) An expression in which the changed variable, the VoI, occurs
     * 4) locate the expression in which the Variable of Interest (VoI) occurs, search through various ASTs to locate the VoI
     * 5) evaluate that expression using the new value of the VoI, an earlier calculation must have already been performed
     * Note:  This assumes that all the expressions were already evaluated using the function "evalAllExpressions"
     * 6) evaluate all other expressions until convergence occurs, NOT yet implemented!!!
     */
    fun reEvalAllVarsDependentUponVoI(fullqualprefix: String, VoI : ValueFeature, newValOfVoI : DD, @Suppress("UNUSED_PARAMETER") iterations : Int = 1) {
        val depvarlist: MutableList<ValueFeature> = mutableListOf()
        lateinit var prop : ValueFeature
        val fullqualname = fullqualprefix + "::" + VoI.name

        // setting up the value of the VoI
        if (newValOfVoI is AADD) {
            VoI.rangeSpec(newValOfVoI.asAadd().getRange())
            model.letVar(fullqualname, VoI.aadd()) // updates the value in the symbol table of the AST
        }
        if (newValOfVoI is IDD) {
            VoI.intSpec(newValOfVoI.asIdd().getRange())
            model.letVar(fullqualname, VoI.idd()) // updates the value in the symbol table of the AST
        }
        // finished updating value of VoI

        //actual algorithm begins here!
        if (ASTs[VoI.name!!] != null) {  // if it is a dep-var
            ast = ASTs[VoI.name!!]!!  // retrieve its AST
            ast.evalFromVar(VoI.name!!) // re-evaluate all vars from new val of VoI for the current ast
            prop = model.global.resolveName<ValueFeature>(VoI.name!!)!!
            prop.quantity.value = calcDepVarValfromSerialDeltaofIndepVars(VoI, VoI, prop.quantity.value) // update depvarval
            // (K, V)
            depvarval[VoI.name!!] = prop
            model.letVar(VoI.name!!, prop.quantity.value) // also, update value in symtab
        }
        else {  // it is NOT a depvar, it IS an indep-var in one or more expressions
            depvaridlist = occurrencemap[VoI.name!!]!!  // get list of dep-var-ids where VoI occurs, these must be updated based upon new val of VoI
            for (elem in depvaridlist)  // loop through list of dep-var-ids
                if (model.global.resolveName<ValueFeature>(elem) != null)
                    depvarlist.add(model.global.resolveName<ValueFeature>(elem)!!)  // set list of Properties for those dep-var-ids
            for (elem in depvarlist) { // loop through list of properties for those dep-var-ids
                prop = model.global.resolveName<ValueFeature>(elem.name!!)!!
                val propVoI = model.global.resolveName<ValueFeature>(VoI.name!!)!!
                prop.quantity.value = calcDepVarValfromSerialDeltaofIndepVars(elem, VoI, propVoI.quantity.value) // up depvarval
                // (K, V)
                model.letVar(elem.name!!, depvarval[elem.name!!]!!.quantity.value) // also, update value in symtab
                depvarval[VoI.name!!] = prop
                model.letVar(VoI.name!!, prop.quantity.value) // also, update value in symtab
            }
        }
    } // end fxn. reEvalAllVarsDependentUponVoI */

}
