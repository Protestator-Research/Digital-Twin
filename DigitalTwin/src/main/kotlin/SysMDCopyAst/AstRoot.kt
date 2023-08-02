
package SysMDCopyAst


import BaseEntities.Feature
import BaseEntities.ValueFeature
import ImportedSysMDExceptions.SemanticError
import ImportedSysMDQuantities.VectorDimensionError
import ImportedSysMDQuantities.VectorQuantity
import ImportedSysMDServices.AgilaSession
import ImportedSysMDServices.reportInfo
import com.github.tukcps.jaadd.*
import com.github.tukcps.jaadd.values.XBool
import java.util.*
import kotlin.collections.ArrayList
import kotlin.collections.HashMap

typealias DDLeaf=DDcond.Leaf<*>
typealias DDInternal=DDcond.Internal<*>

/**
 * This AST node holds information for the constraint propagation.
 * It holds the root of the AST for a given property that has to be computed.
 * @param model the overall data model as dependency injected via constructor parameter.
 * @param property the property that is computed by the AST
 * @param dependency the AST that describes the dependency of the property from other properties.
 */
class AstRoot(
    model: AgilaSession,
    val property: ValueFeature,
    val dependency: AstNode
) : AstNode(model) {

    // Leaves of the AST with this as a root
    internal var leaves: Collection<AstLeaf> = ArrayList()

    /** initialization */
    init {
        dependency.root = this // recursion in setter methods will set it down to leaves.
    }


    override fun initialize() {
        upQuantity = dependency.upQuantity.clone()
        downQuantity = dependency.upQuantity.clone()
        leaves = dependency.getLeaves()
        property.ast = this
        property.vectorQuantity = upQuantity
    }

    /**
     *  Takes value and unit from the dependency.
     *  - Transforms the value/unit if possible to the SI unit
     */
    override fun evalUp() {
        when {
            this.dependency.isReal -> {
                if (property.ofClass?.ref?.isSubclassOf(model.repo.realType) == true
                    && property.direction == Feature.FeatureDirectionKind.IN) {
                    val quantityWithTransformedUnit = dependency.upQuantity.clone()
                    upQuantity = VectorQuantity(
                        quantityWithTransformedUnit.values,
                        quantityWithTransformedUnit.unit,
                        property.vectorQuantity.unitSpec
                    )
                    property.vectorQuantity = upQuantity.constrain(property.vectorQuantity, property.rangeSpecs, property.unitSpec)
                    if(property.intSpecs.size!=dependency.upQuantity.values.size && property.intSpecs.size!=1)
                        throw VectorDimensionError("Vector size of ${dependency.upQuantity.values.size} does not match Constraint size of ${property.rangeSpecs.size}")
                    if (property.vectorQuantity.values.any { it == model.builder.Empty })
                        model.reportInfo(property, "Inconsistency - dependency for ${property.effectiveName} is not satisfiable")
                } else if (property.ofClass?.ref?.isSubclassOf(model.repo.realType) == true
                    && property.direction == Feature.FeatureDirectionKind.OUT) {
                    // Convert the rangeSpecs to a VectorQuantity
                    val values = mutableListOf<AADD>()
                    property.rangeSpecs.forEach{values.add(model.builder.range(it))}
                    upQuantity = VectorQuantity(values, property.unitSpec)
                    property.vectorQuantity = upQuantity
                    if(property.rangeSpecs.size!=dependency.upQuantity.values.size && property.rangeSpecs.size!=1)
                        throw VectorDimensionError("Vector size of ${dependency.upQuantity.values.size} does not match Constraint size of ${property.rangeSpecs.size}")
                    if(property.rangeSpecs.size == dependency.upQuantity.values.size)
                        if (property.rangeSpecs.indices.any{property.rangeSpecs[it] !in (dependency.upQuantity.values[it] as AADD).getRange()})
                            model.reportInfo(property, "Dependency for ${property.effectiveName} cannot be satisfied for all values of range.")
                } else
                    throw SemanticError("${property.qualifiedName}: expect expression of type Real")
            }

            this.dependency.isBool -> {
                upQuantity = dependency.upQuantity.constrain(property.boolSpecs, true)
                property.vectorQuantity = upQuantity
            }

            this.dependency.isInt -> {
                if (property.ofClass?.ref?.isSubclassOf(model.repo.integerType) == true
                    && property.direction == Feature.FeatureDirectionKind.IN) {
                    upQuantity = dependency.upQuantity
                    property.vectorQuantity = upQuantity.constrain(property.intSpecs)
                } else if (property.ofClass?.ref?.isSubclassOf(model.repo.integerType) == true
                    && property.direction == Feature.FeatureDirectionKind.OUT) {
                    upQuantity = dependency.upQuantity
                    val values = mutableListOf<IDD>()
                    property.intSpecs.forEach { values.add(model.builder.rangeIDD(it)) }
                    property.vectorQuantity = VectorQuantity(values)
                    if(property.rangeSpecs.size!=dependency.upQuantity.values.size && property.rangeSpecs.size!=1)
                        throw VectorDimensionError("Vector size of ${dependency.upQuantity.values.size} does not match Constraint size of ${property.rangeSpecs.size}")
                    if(property.rangeSpecs.size == dependency.upQuantity.values.size)
                        if (property.rangeSpecs.indices.any{property.intSpecs[it] !in (dependency.upQuantity.values[it] as IDD).getRange()})
                            model.reportInfo(property, "Dependency for ${property.effectiveName} cannot be satisfied for all values of range.")
                } else
                    throw SemanticError("${property.qualifiedName}: expect expression of type Integer")
            }
        }
    }


    /** This may be causing errors with the IntegerRange / IDD datatype(s) */
    override fun evalUpRec() {
        dependency.evalUpRec()
        evalUp()
    }


    override fun evalDown() {
        when {
            this.isReal -> {
                property.vectorQuantity = property.vectorQuantity.constrain(property.vectorQuantity, property.rangeSpecs, property.unitSpec)
                downQuantity = property.vectorQuantity.clone()
                dependency.downQuantity = downQuantity
            }

            this.isBool -> {
                property.vectorQuantity = property.vectorQuantity.constrain(property.boolSpecs, true)
                downQuantity = property.vectorQuantity.clone()
                dependency.downQuantity = downQuantity
            } // Nothing to be done for booleans

            // Units does not support the IDD datatype
            this.isInt -> {
                property.vectorQuantity = property.vectorQuantity.constrain(property.intSpecs)
                downQuantity = downQuantity.intersect(property.vectorQuantity)
                dependency.downQuantity = downQuantity.clone()
            }
        }
    }

    override fun toExpressionString(): String = dependency.toExpressionString()

    override fun evalDownRec() {
        evalDown()
        dependency.evalDownRec()
    }

    override fun toString() =
        if (property.isVectorQuantityInitialized) "AstRoot: ${property.vectorQuantity}"
        else "AstRoot: (uninitialized quantity)"

    fun solve(iterations: Int = 1) {
        for (i in 1..iterations) {
            evalUpRec()
            evalDownRec()
        }
    }

    @Deprecated("Will be phased out") //TODO not for Vectors implemented
    fun solveAst(): DD {
        val conditions = if (this.isBool) this.bdd.evaluate() else if (this.isReal) this.aadd.evaluate() else this.idd.evaluate()

        if (conditions is DDcond.Leaf) return conditions
        if (conditions.isInfeasible) return model.builder.Infeasible //TODO: InfeasibleB?

        //Find the shortest path!
        val path = findShortestPath(conditions, property.boolSpecs[0])

        for (index in path) {
            if (model.builder.conds.getCondition(index.key) is AADD) continue
            val cond = if (index.value) model.builder.True else model.builder.False
            model.builder.conds.setVariable(index.key, cond) //TODO: What if cond != boolSpec?
        }

        return if (this.isBool) this.bdd.evaluate() else this.aadd.evaluate()
    }

    @Deprecated("Will be phased out") //TODO not for Vectors implemented
    fun solveAstWithAlternatives(): DD {
        val conditions = if (this.isBool) this.bdd.evaluate() else if (this.isReal) this.aadd.evaluate() else this.idd.evaluate()

        if (conditions is DDcond.Leaf) return conditions
        if (conditions.isInfeasible) return model.builder.Infeasible //TODO: InfeasibleB?

        val paths = findAllPaths(conditions, property.boolSpecs[0])
        var trialState = model.builder.conds.x.toMutableMap()
        var boolSpecOk = true //break condition


        paths@ for (path in paths) {
            indexes@ for (index in path) {

                if (model.builder.conds.getCondition(index.key) is AADD) continue

                val cond = if (index.value) model.builder.True else model.builder.False

                //Check against boolSpec!
                val spec = getConditionBoolSpec(index.key)
                if (!checkAgainstBoolSpec(index.value, spec[0])) {
                    //BREAK CASE!
                    trialState = model.builder.conds.x.toMutableMap() //reset to start point
                    paths.removeFirst()
                    boolSpecOk = false
                    break@indexes
                } else {
                    trialState[index.key] = cond
                }
            }
            if (boolSpecOk) {
                break@paths
            } else
                boolSpecOk = true
        }
        model.builder.conds.x = trialState as HashMap<Int, DD>

        return if (isBool) bdd.evaluate() else aadd.evaluate()
    }

    private fun findShortestPath(dd: DD, target: XBool = XBool.True, path: MutableMap<Int, Boolean> = mutableMapOf()): MutableMap<Int, Boolean> {
        val targetLeaf = when (target) {
            XBool.True, XBool.X -> model.builder.True //default case: if not specified => make it true
            XBool.False -> model.builder.False
            else -> {
                throw Exception("findShortestPath: XBool-case not yet implemented: $target")
            }
        }

        when (dd) {
            is DDLeaf -> {
                return if (dd === targetLeaf) path
                else mutableMapOf()
            }
            is DDInternal -> {
                val pt = path.toMutableMap()
                pt[dd.index] = true
                val t = findShortestPath(dd.T, target, pt)
                val pf = path.toMutableMap()
                pf[dd.index] = false

                val f = findShortestPath(dd.F, target, pf)

                val min = if (t.isNotEmpty() && f.isNotEmpty()) {
                    if (t.size > f.size) f
                    else t
                } else if (t.isNotEmpty()) t
                else if (f.isNotEmpty()) f
                else mutableMapOf()

                for (entry in min)
                    path[entry.key] = entry.value

                return path
            }
            else -> throw Exception("Must not be reached.")
        }
    }


    /*private*/ fun findAllPaths(
        dd: DD,
        target: XBool = XBool.True,
        path: MutableMap<Int, Boolean> = mutableMapOf(),
        paths: MutableList<MutableMap<Int, Boolean>> = mutableListOf()
    ): MutableList<MutableMap<Int, Boolean>> {
        val targetLeaf = when (target) {
            XBool.True, XBool.X -> model.builder.True //default case: if not specified => make it true
            XBool.False -> model.builder.False
            else -> {
                throw Exception("findAllPaths: XBool-case not yet implemented: $target")
            }
        }

        if (dd is DDcond.Leaf) {
            if (dd === targetLeaf) {
                paths.add(path)
            }
        } else {
            val pt = path.toMutableMap()
            pt[dd.index] = true
            // val t = findAllPaths(dd.T!!, target, pt, paths)
            val pf = path.toMutableMap()
            pf[dd.index] = false
            // val f = findAllPaths(dd.F!!, target, pf, paths)
        }

        return paths
    }

    /** Executes a block of statements on each AstNode in an Ast */
    override fun <R> runDepthFirst(block: AstNode.() -> R): R {
        val result: R = dependency.runDepthFirst(block)
        this.run(block)
        return result
    }

    /** Executes a block of statements on each AstNode in an Ast */
    override fun <R> withDepthFirst(receiver: AstNode, block: AstNode.() -> R): R {
        return withDepthFirst(dependency, block)
    }

    /** Clone method, creates deep copy */
    override fun clone(): AstRoot =
        AstRoot(model, property, dependency.clone())

    /**
     * Returns the (Boolean) Value from the jAADD builder that corresponds to a Boolean Expression
     * @param index the index in the BDD/AADD
     * @return the currently set value (true, false, X)
     */
    private fun getConditionBoolSpec(index: Int): MutableList<XBool> {
        val id = model.builder.conds.indexes.keys.first { index == model.builder.conds.indexes[it] }
        return model.getProperty(id = UUID.fromString(id)).boolSpecs
    }

    private fun checkAgainstBoolSpec(value: Boolean, spec: XBool): Boolean {
        return when (spec) {
            XBool.True -> value
            XBool.False -> value.not()
            XBool.X -> true
            else -> throw Exception("checkAgainstBoolSpec: NaB, AF")
        }
    }
}
