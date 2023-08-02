package SysMDImportSolver


import BaseEntities.ValueFeature
import SysMDImportSolver.analyzer.SetOfSolutions
import com.github.tukcps.jaadd.DD
import com.github.tukcps.jaadd.DDBuilder
import com.github.tukcps.jaadd.values.XBool



open class UnitMap(private val builder: DDBuilder) {
    private val unitMap: HashMap<ValueFeature, HashMap<Int, DD>> = hashMapOf()
    val keys
        get() = unitMap.keys
    val values
        get() = unitMap.values
    val entries
        get() = unitMap.entries
    private val reversed
        get() = unitMap.entries.associate { (k, v) -> v to k }

    fun getKey(element: HashMap<Int, DD>) : ValueFeature {
        return reversed[element]!!
    }

    @Suppress("UNCHECKED_CAST")
    operator fun iterator() : MutableIterator<HashMap<ValueFeature, HashMap<Int, DD>>> {
        return unitMap.iterator() as MutableIterator<HashMap<ValueFeature, HashMap<Int, DD>>>
    }

    fun update(value: ValueFeature, units: HashMap<Int, DD>, dontcares: HashSet<Int>) {
        val filteredUnitMap = units
        for (e in filteredUnitMap.keys) {
            if (dontcares.contains(e)) filteredUnitMap[e] = XBool.X.bddLeafOf(builder)
        }
        if (filteredUnitMap.isEmpty()) {
            for (i in dontcares) {
                filteredUnitMap[i] = XBool.X.bddLeafOf(builder)
            }
        }
        unitMap[value]=filteredUnitMap
    }

    operator fun get(i: ValueFeature): HashMap<Int, DD> {
        return unitMap[i] as HashMap<Int, DD> /* = java.util.HashMap<kotlin.Int, com.github.tukcps.jaadd.DD> */
    }

    override fun toString(): String {
        return unitMap.toString()
    }

}


/**
 * Stores paths that lead to infeasible result within Inequations
 */
open class InfeasibilityMap(private val builder: DDBuilder) {
    private val infeasibilityMap: HashMap<ValueFeature, SetOfSolutions> = hashMapOf()
    private val reversed
        get() = infeasibilityMap.entries.associate { (k, v) -> v to k }

    val keys
        get() = infeasibilityMap.keys
    val values
        get() = infeasibilityMap.values
    val entries
        get() = infeasibilityMap.entries



    operator fun get(i: ValueFeature): SetOfSolutions? {
        return infeasibilityMap[i]
    }

    @Suppress("UNCHECKED_CAST")
    operator fun iterator() : MutableIterator<HashMap<ValueFeature, HashSet<HashMap<Int, DD>>>> {
        return infeasibilityMap.iterator() as MutableIterator<HashMap<ValueFeature, HashSet<HashMap<Int, DD>>>>
    }

    fun update(uid: ValueFeature, infeasiblePaths: SetOfSolutions) {
        infeasibilityMap[uid] = infeasiblePaths
    }

    fun getKey(element: SetOfSolutions): ValueFeature {
        return reversed[element]!!
    }

    override fun toString(): String {
        return infeasibilityMap.toString()
    }
}

/**
Intended mostly as data class to store current state of the breath first search

Units: Boolean properties and their respective allocations that fullfill for conditions for the current evaluation
UnitMap: Intersection of units over all properties with the current evaluation
IntroducedProperties: Properties that have been introduced through analysis of units and unitmap
DontCares: Conditions that do not matter for the respective property (but exist in at least one path to the evaluation.
Conflicts: Possible conflicts detected up until now.

TODOs: - Reasons: Detailed reasoning for changes made from last step
 **/
open class DiscreteSolverStep(val stepNumber: Int) {
    lateinit var unitMap: UnitMap
    lateinit var infeasibilityMap: InfeasibilityMap
    val introducedProperties = mutableListOf<ValueFeature>()
    val conflicts = hashMapOf<Int, HashSet<DD>>()
    val updatedProperties = mutableListOf<ValueFeature>()
}

open class DiscreteSolverDepth(var depthLevel: Int, var currentState: DiscreteSolverHistoryTreeNode) {
    //lateinit var currentState: DiscreteSolverHistoryTreeNode
    lateinit var tantativeStates: MutableSet<DiscreteSolverHistoryTreeNode>
}


data class DiscreteSolverHistoryTreeNode(val id: Int, val parent: DiscreteSolverHistoryTreeNode?) {
    val assignments = mutableSetOf<MutableMap<Int, DD>>()
    val noGoods = mutableSetOf<Pair<Int, DD>>()
    val children = mutableSetOf<DiscreteSolverHistoryTreeNode>()
}

//enum class Trail {
//    FREE, GUESSED, RELATED, NOGOOD
//}