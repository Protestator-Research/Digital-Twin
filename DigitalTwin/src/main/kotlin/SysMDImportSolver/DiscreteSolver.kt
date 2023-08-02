//package SysMDImportSolver
//
//import BaseEntities.ValueFeature
//import ImportedSysMDServices.AgilaSession
//import SysMDImportSolver.analyzer.DDAnalyzer
//import SysMDImportSolver.analyzer.SemanticAnalyzer
//import SysMDImportSolver.analyzer.Solution
//import SysMDImportSolver.normalizer.CNNormalizer
//import SysMDImportSolver.normalizer.NormalizedProperties
//import SysMDImportSolver.normalizer.NormalizedPropertiesSolver
//import SysMDImportSolver.propagator.NetworkConsistencyPropagator
//import SysMDImportSolver.propagator.PropagatorIF
//import SysMDImportSolver.valuefeatures.GuardValueFeature
//import SysMDImportSolver.valuefeatures.HasAValueFeature
//import SysMDImportSolver.valuefeatures.IsAValueFeature
//import SysMDImportSolver.valuefeatures.RelatedValueFeature
//import BaseEntities.*
//import BaseEntities.implementation.SpecializationImplementation
//import ImportedSysMDExceptions.SemanticError
//import ImportedSysMDQuantities.Quantity
//import ImportedSysMDServices.report
//import com.github.tukcps.jaadd.BDD
//import com.github.tukcps.jaadd.DD
//import com.github.tukcps.jaadd.DDBuilder
//import com.github.tukcps.jaadd.functions.intersect
//import com.github.tukcps.jaadd.values.XBool
//
//import java.util.*
//import kotlin.collections.ArrayList
//import kotlin.math.absoluteValue
//
///**
// *  Handles discrete propagation. Currently only through one propagate method. Eventually will split in initialization and update.
// */
//
///**
// * TODO's: - ConflictTracer (own class?)
// *         - Human readable reasoning (string-msg?)
// */
//
////typealias UnitMap = MutableMap<UId, MutableMap<Int, DD>>
//
//class DiscreteSolver<ValueFeature>(
//    val model: AgilaSession
//) : DiscreteSolverIF {
//    //Done refactoring:
//    private var debugMode = false
//    private var allSolutions = false
//    var initLevel = 0
//        private set
//
//    private val schedule: ArrayList<ValueFeature> //TODO: refactor. Not sure if still needed.
//          get() = model.repo.schedule
//
//    val builder: DDBuilder
//        get() = model.builder
//
//
//    private lateinit var booleanConditions: MutableMap<Int, Pair<DD, UUID?>>//Map<Int, Triple<DD, Trail, UUID?>>
//        //get() = builder.conds.x.filter { it.value is BDD }
//
//    private var depth = DiscreteSolverDepth(0, DiscreteSolverHistoryTreeNode(0, null))
//
//    lateinit var ddAnalyzer: DDAnalyzer
//    lateinit var semanticAnalyzer: SemanticAnalyzer
//    private lateinit var unitMap: UnitMap //will be set in initialize function
//    private lateinit var infeasibilityMap: InfeasibilityMap //will be set in initialize function
//
//    private lateinit var networkConsistencyPropagator: NetworkConsistencyPropagator
//    private val propagators: HashSet<PropagatorIF> = hashSetOf()
//
//    private val evaluations: HashMap<ValueFeature, DD?> = hashMapOf()
//
//    private val updatedProperties: HashSet<Feature> = hashSetOf<Feature>()
//
//    //Normalizer + Boolean SAT Solver
//    private var activateNormalizedBooleanValueFeatureSolver = true
//    /*private*/ val booleanValueFeatureNormalizer = CNNormalizer()
//    /*private*/ lateinit var normalizedBooleanModel: NormalizedProperties //will be set in initialize function
//    /*private*/ var normalizedBooleanValueFeatureSolver = NormalizedPropertiesSolver()
//
//
//    //TODO: Refactor
//    //private lateinit var step: DiscreteSolverStep Replaced by Depth
//
//    //private var currentStepCount = 0 //planned for conflict detection to comprehend where conflicts could stem from
//    //private var history = mutableListOf<DiscreteSolverStep>()
//    private val noGoods = hashMapOf<ValueFeature, DD>()
//
//    private val relatedValueFeatures: HashSet<RelatedValueFeature> = hashSetOf()
//    private val guards: HashSet<GuardValueFeature> = hashSetOf()
//    private val hasAs: HashSet<HasAValueFeature> = hashSetOf()
//    private val isAs: HashSet<IsAValueFeature> = hashSetOf()
//
//    private val conflicts = hashMapOf<Int, HashSet<DD>>()
//    private val contradictions = hashSetOf<ValueFeature>()
//
//
//    private val conditionIndexes: MutableSet<Int>
//        get() = builder.conds.x.keys
//
//
//    //Done refactoring
//    fun initialize(model: AgilaSession, level: Int) {
//        //Copies structure from Agila-Session, to be consistent with the current state of initialization of the model
//        if (level > 0) {
//            unitMap = UnitMap(builder)
//            infeasibilityMap = InfeasibilityMap(builder)
//
//            propagators.clear()
//            initLevel = level
//        }
//
//        if (level > 1) {
//            //TODO!
//            initLevel = level
//            //initClassifiers()     // Calls initialize of classifiers that will add inherited properties.
//        }
//        if (level > 2) {
//            //TODO!
//            initLevel = level
//            //resolveNames()        // Again, update name resolution considering inheritance
//        }
//        if (level > 5) {
//            //TODO!
//            initLevel = level
//            //initRelationships()
//        }
//        if (level > 4) {
//            model.get().filterIsInstance<ValueFeature>().forEach { update(it) }
//            initLevel = level
//            //TODO!
//            //initProperties()
//        }
//
//        // Now, we only do checking and reporting of issues to the Agenda.
//        if (level > 6) {
//            //TODO: Nothing?
//            initLevel = level
//            //checkIdentifications()
//        }
//        if (level > 7) {
//            initBooleanConditions()
//            advanceState()
//            normalizedBooleanModel = normalizeBooleanValueFeatures()
//            if (activateNormalizedBooleanValueFeatureSolver) initNormalizedBooleanValueFeatureSolver(allSolutions, debugMode)
//            initLevel = level
//        }
//    }
//
//
//        fun resetNormalizedBooleanValueFeatureSolver(all: Boolean = true, debug: Boolean = false) {
//            normalizedBooleanValueFeatureSolver = NormalizedPropertiesSolver()
//            initNormalizedBooleanValueFeatureSolver(all, debug)
//        }
//
//    //fun isInitialized() : Boolean {
//    //    return initLevel > 0 //Possible to stop mid initialization e.g. at level = 3?
//    //}
//
//    /*private*/ fun normalizeBooleanValueFeatures() : NormalizedProperties {
//        return booleanValueFeatureNormalizer.normalizeBooleanConstraints(model)
//    }
//
//    fun normalizeBooleanValueFeatures(model: AgilaSession) : NormalizedProperties {
//        return booleanValueFeatureNormalizer.normalizeBooleanConstraints(model)
//    }
//
//    fun activateNormalizedBooleanValueFeatureSolver() {
//        activateNormalizedBooleanValueFeatureSolver = true
//    }
//
//    fun deactivateNormalizedBooleanValueFeatureSolver() {
//        activateNormalizedBooleanValueFeatureSolver = false
//    }
//
//    fun computeAllPartialSolutions(all: Boolean) {
//        if (all) normalizedBooleanValueFeatureSolver.findAllAssignments()
//        else normalizedBooleanValueFeatureSolver.findOnlyOneAssignment()
//    }
//
//    private fun initBooleanConditions() {
//        val boolConds = builder.conds.x.filter { it.value is BDD }
//        val booleanConditions = mutableMapOf<Int, Pair<DD, UUID?>>()//mutableMapOf<Int, Triple<DD, Trail, UUID?>>()
//        boolConds.forEach {
//            val uuid: String? = builder.conds.indexes.filter { (key, value) -> value == it.key }.keys.first()
//            booleanConditions[it.key] = Pair(it.value, UUID.fromString(uuid))
//        }//Triple(it.value, Trail.FREE, null) }
//        this.booleanConditions = booleanConditions
//    }
//
//    private fun initNormalizedBooleanValueFeatureSolver(allAssignments: Boolean = true, debugMode: Boolean = false) {
//        if (debugMode) normalizedBooleanValueFeatureSolver.activateDebugMode() else normalizedBooleanValueFeatureSolver.deactivateDebugMode()
//        if (allAssignments) normalizedBooleanValueFeatureSolver.findAllAssignments() else normalizedBooleanValueFeatureSolver.findOnlyOneAssignment()
//    }
//
//    /**
//     * NormalizedBooleanValueFeature returns satisfying assignments via CNF depicted as Set of Set of Ints
//     * The Function is intended to translate back those CNFs into our Model
//     */
//    /*private*/ fun translateCNFsBack(cnf: Set<Set<Int>>) : Set<MutableMap<Int, BDD>> {
//        val assignments = mutableSetOf<MutableMap<Int, BDD>>()
//
//        cnf.forEach { assignments.add(translateCNFBack(it)) }
//
//        return assignments
//    }
//
//    private fun translateCNFBack(cnf: Set<Int>) : MutableMap<Int, BDD> {
//        val assignment = mutableMapOf<Int, BDD>()
//
//        for (c in cnf) {
//            if (c < 0 )
//                assignment[c.absoluteValue] = builder.False //negative case
//            else
//                assignment[c] = builder.True //positive case
//        }
//        return assignment
//    }
//
//    private fun solveNormalizedNet() : Set<MutableMap<Int, BDD>>
//    {
//        return translateCNFsBack(normalizedBooleanValueFeatureSolver.solveNormalizedCN(this.normalizedBooleanModel))
//    }
//
//    private inline fun initDepth() {
//        val history = DiscreteSolverHistoryTreeNode(0, null)
//        history.assignments = TODO()
//        history.noGoods = TODO()
//        depth = DiscreteSolverDepth(0, history)
//    }
//
//    private fun updateFeature(feature: Feature) {
//        TODO()
//    }
//
//    private fun updateValueFeature(valueFeature: ValueFeature) {
//        if (containsInequation(valueFeature) && valueFeature.vectorQuantity.value !is BDD) {//(containsInequation(up)) {
//            infeasibilityMap.update(valueFeature, ddAnalyzer.getInfeasibleCombinations(valueFeature))
//        }
//        unitMap.update(valueFeature, ddAnalyzer.getUnits(valueFeature), ddAnalyzer.getDontCares(valueFeature))
//    }
//
//    private fun updateSpezialization(specialization: Specialization) {
//        //TODO()
//    }
//
//    private fun updateMultiplicity(multiplicity: Multiplicity) {
//        //TODO()
//    }
//
//    private fun updateRelationship(relationship: Relationship) {
//        //TODO()
//    }
//
//    private fun processUpdates() {
//        updatedProperties.forEach {
//                up -> when (up) {
//                    is Multiplicity -> updateMultiplicity(up)
//                    is ValueFeature -> updateValueFeature(up)
//                    is Specialization -> updateSpezialization(up)
//                    is Relationship -> updateRelationship(up)
//                    is Feature -> updateFeature(up)
//                    else -> throw Exception("Discrete Solver: ProcessUpdate: Unknown Entity $up") //should never be reached, but better be on the safe side
//                }
//        }
//    }
//
//    //TODO: Refactoring
//    @Deprecated("Will be phased out. Use initialize function with an Integer Param!")
//    override fun initialize(model: AgilaSession) {
//
//        unitMap = UnitMap(builder)
//        infeasibilityMap = InfeasibilityMap(builder)
//
//        propagators.clear()
//        ddAnalyzer = DDAnalyzer(model)
//        semanticAnalyzer = SemanticAnalyzer(model)
//        networkConsistencyPropagator = NetworkConsistencyPropagator(model)
//
//        //step = DiscreteSolverStep(0)
//        //step.unitMap = this.unitMap
//        //step.infeasibilityMap = this.infeasibilityMap
//        //history.add(step)
//
//        model.get().filterIsInstance<ValueFeature>().forEach { update(it) }
//        advanceState()
//    }
//
//    fun isInitialized(): Boolean = (this::ddAnalyzer.isInitialized && this::semanticAnalyzer.isInitialized && this::networkConsistencyPropagator.isInitialized && this::unitMap.isInitialized && this::infeasibilityMap.isInitialized)
//
//
//    fun getCreatingProperty(relatedProperty: ValueFeature): ValueFeature? {
//        return if (relatedProperty !is RelatedValueFeature) null
//        else relatedProperty.createdBy
//    }
//
//    fun getRelatedValueFeatures(id: UUID): HashSet<RelatedValueFeature> {
//        return relatedValueFeatures.filter { it.createdBy.elementId == id }.toHashSet()
//    }
//
//    fun getGuardValueFeatures(id: UUID): HashSet<GuardValueFeature> {
//        return guards.filter { it.createdBy.elementId == id }.toHashSet()
//    }
//
//    override fun update(scheduledProperties: List<ValueFeature>) {
//        for (s in scheduledProperties) {
//            if (s.updated) update(s)
//        }
//    }
//
//    override fun update(updatedProperty: ValueFeature) {
//        updateEvaluations(updatedProperty)
//        updateAnalyzer(updatedProperty)
//        updatedProperties.add(updatedProperty)
//    }
//
//    /**
//     * Called After Updates have been reported. Will analyze properties and advance search or return to previous step respectively
//     */
//    @Suppress("UNCHECKED_CAST")
//    override fun advanceState() {
//        //TODO: Sepparate into 1. analyzing part 2. propagation part 3. error checking/handling
//        //TODO: This goes into the analyzing part
//        processUpdates()
//
//
//
//        val introducedRelatedProperties = introduceRelatedValueFeatures(unitMap)
//        //val infeasibilityGuardingProperties = introduceGuardValueFeatures(infeasibilityMap) Commented out to be replaced by nogood recording
//
//
//        //recordNoGoods(infeasibilityGuardingProperties as MutableList<GuardValueFeature>)
//        enforceNodeConsistency(introducedRelatedProperties as MutableList<RelatedValueFeature>)
//
//        //verifyState(introducedRelatedProperties, infeasibilityGuardingProperties, possibleConflicts)
//
//        //assertConstraints()
//        updatedProperties.clear()
//    }
//
//    //FIXME: intersect with dependency!
//    fun assertConstraints() {
//        val booleanConstraints =
//            model.get().filterIsInstance<ValueFeature>().filter { it.ofClass!!.ref == model.repo.booleanType }
//        val booleanConstrained = booleanConstraints.filter { it.boolSpecs[0] in setOf(XBool.True, XBool.False) }
//
//        booleanConstrained.forEach {
//            val oldValue = /*evaluations[it] as DDcond // */ if (it.vectorQuantity.value.toString() == "Unknown") XBool.X.bddLeafOf(builder) else it.vectorQuantity.value
//            val specc = it.boolSpecs[0].bddLeafOf(builder)
//            it.vectorQuantity.values = mutableListOf(specc.intersect(oldValue))
//        }
//    }
//
//
//    private fun analyzeState():Pair<HashSet<RelatedValueFeature>, HashSet<GuardValueFeature>> {
//        //TODO
//        return Pair(hashSetOf(), hashSetOf())
//    }
//
//    private fun propagateState(analysis: Pair<HashSet<RelatedValueFeature>, HashSet<GuardValueFeature>>) {
//        //FIXME: Since generated value features are registered and scheduled input parameters still necessary?
//        schedule.forEach { ot -> propagators.forEach { it.execute(ot) } }
//    }
//
//    private fun verifyState(introducedRelatedProperties: List<ValueFeature>, introducedGuardProperties: List<ValueFeature>, possibleConflicts: HashMap<Int,  DD>) {
//        //TODO
//        val conflicts = checkForConflicts(possibleConflicts)
//        val errors = checkForErrors(updatedProperties as HashSet<ValueFeature>)
//        if (conflicts.isEmpty() && errors.isEmpty()) { //best case
//
//            //val newStep = increaseStep(currentStepCount, introducedRelatedProperties)
//            //history.add(newStep)
//        }
//        else {
//            if (conflicts.isNotEmpty() && errors.isEmpty()) { //conflict between two conditions, but not necessary unsatsfiable
//                for (c in conflicts) {
//                    //TODO
//                    //println("conflict: " + c.toString())
//                }
//            }
//            else if (conflicts.isEmpty() && errors.isNotEmpty()) { //no conflict, but somehow property got usnatisfiable
//                for (e in errors) {
//                    //TODO
//                    //println("errors: " + e.toString())
//                    if (e.ast != null) {
//                        val dependencies = e.ast!!.getDependencies()
//                        for (d in dependencies)
//                            d.ast!!.evalUpRec()
//                    }
//                }
//            }
//            else { // conflict that likely makes at least one property unsatisifable
//                //TODO!
//                for (c in conflicts) {
//                    //TODO
//                    //println("conflict: " + c.toString())
//                }
//                for (e in errors) {
//                    //TODO
//                    //println("errors: " + e.toString())
//                }
//            }
//            //determine wether conflict or error or both and return to previous step and record nogoods.
//        }
//    }
//
//    private fun checkForConflicts(possibleConflicts: HashMap<Int, DD>): List<Pair<Int, HashSet<ValueFeature>>> {
//        val result = mutableListOf<Pair<Int, HashSet<ValueFeature>>>()
//        for (i in possibleConflicts.keys) {
//            if (possibleConflicts[i].toString() == "Contradiction") {
//                val involvedProperties = hashSetOf<ValueFeature>()
//                for (u in unitMap.values) {
//                        if (u.containsKey(i)) {
//                            involvedProperties.add(unitMap.getKey(u))
//                        }
//                }
//                result.add(Pair(i, involvedProperties))
//            }
//        }
//        return result
//    }
//
//    private fun checkForErrors(updatedProperties: HashSet<ValueFeature>/*List<ValueFeature>*/): List<ValueFeature> {
//        //FIXME: Only checking updated properties sufficient?
//        val result = mutableListOf<ValueFeature>()
//        for (p in updatedProperties) {
//            if (p.boolSpecs != XBool.X) {
//                if (p.vectorQuantity.bdd().value != p.boolSpecs) result.add(p)
//            }
//        }
//        return result
//    }
//
//    private fun recordNoGoods(guards: MutableList<GuardValueFeature>) {
//        guards.forEach { recordNoGood(it.createdBy.elementId, it) }
//    }
//
//    private fun recordNoGood(id: UUID, guard: GuardValueFeature) {
//        //println(guard.quantity.value.toIteString())
//        //println(guard.dependency.toString())
//    }
//
//    /**
//     * This function will set Conditions of Builder according to introduced Related Properties
//     */
//    private fun enforceNodeConsistency(relatedValueFeatures: MutableList<RelatedValueFeature>) {
//        relatedValueFeatures.forEach { networkConsistencyPropagator.execute(it) }
//    }
//
//    private fun introduceRelatedValueFeatures(unitMap: UnitMap): MutableList<ValueFeature> {
//        val newProperties = mutableListOf<ValueFeature>()
//        val reversed = builder.conds.indexes.entries.associate { (key, value) -> value to key }
//
//        for (prop in unitMap.keys) {
//            if (prop.ofClass?.ref != model.repo.booleanType) continue
//            if (prop.ofClass?.ref != model.repo.booleanType && prop.boolSpecs[0] != builder.True && prop.boolSpecs[0] != builder.False ) continue
//            innerloop@ for (i in conditionIndexes) {
//                val unit = unitMap[prop][i]
//                if (unit != builder.True && unit != builder.False) continue
//                val relatedProperty = prop//model.getProperty(k)
//                var dependencyString: String? = null
//                if (reversed[i] != null ) {//reversed[i]?.let {
//                    try {
//                        var condIndex = ""
//                        var cutReversed = ""
//                        if (reversed[i]!!.toString().contains("Condition", true)) {
//                            cutReversed = reversed[i].toString().split("Condition").first()
//                            condIndex = reversed[i].toString().split("Condition")[1]
//                        }
//                        else cutReversed = reversed[i]!!
//                        dependencyString = model.getProperty(UUID.fromString(cutReversed)).name.toString() + "Condition"+ condIndex + " = " + unit.toString()
//                    }
//                    catch (s: SemanticError) {
//                        s.element = prop
//                        model.report(s)
//                        continue@innerloop
//                    }
//                }
//                val newRelatedProperty = createRelatedValueFeature(dependencyString, unit.toString(), i, relatedProperty)
//                if (relatedValueFeatures.contains(newRelatedProperty)) {
//                    val registered = registerOrUpdateProperty(newRelatedProperty, relatedProperty)
//                    relatedValueFeatures.add(registered as RelatedValueFeature)
//                    newProperties.add(registered)
//                }
//                else {
//                    val registered = registerOrUpdateProperty(newRelatedProperty, relatedProperty)
//                    relatedValueFeatures.add(registered as RelatedValueFeature)
//                    newProperties.add(registered)
//                }
//            }
//        }
//        return newProperties
//    }
//
//    /**
//     * Introduces new Related Properties through (simple) inference. Returns List of newly created Properties
//     */
//
//    private fun introduceGuardValueFeatures(infeasibilityMap: InfeasibilityMap): MutableList<ValueFeature> {
//        val introducedGuards = mutableListOf<ValueFeature>()
//        val reversedConditions = builder.conds.indexes.entries.associate { (k, v) -> v to k }
//        for (i in infeasibilityMap.entries) {
//                    //Construct dependency from each hashmap => those prop must not fulfilled
//            var  pathCounter = 0
//            for (hm in i.value) { //hashSet
//                var dependencyString = ""
//                for (index in hm.entries) {
//                    val valueFeatureName = reversedConditions[index.key]
//                    dependencyString += if (index.value)
//                        "$valueFeatureName and "
//                    else
//                        "not($valueFeatureName) and "
//                }
//                dependencyString = dependencyString.dropLast(4) //remove the last " and"
//                val guardianProperty = createGuardValueFeature(dependencyString, hm, pathCounter++, i.key)
//                guardianProperty.vectorQuantity = Quantity(builder.False)
//                //FIXME: Both cases the same?
//                if (introducedGuards.contains(guardianProperty)) {
//                    val registered = registerOrUpdateProperty(guardianProperty, i.key)
//                    introducedGuards.add(registered)
//                    guards.add(registered as GuardValueFeature)
//                }
//                else {
//                    val registered = registerOrUpdateProperty(guardianProperty, i.key)
//                    introducedGuards.add(registered)
//                    guards.add(registered as GuardValueFeature)
//                }
//            }
//        }
//        return introducedGuards
//    }
//
//    private fun createRelatedValueFeature(dependencyString: String?, valueSpecString: String, varIndex: Int, createdBy: ValueFeature): RelatedValueFeature {
//        dependencyString?:model.report(createdBy, "Erroneous dependency string (index = null)")
//
//        val dependency = if (dependencyString!!.contains("True")) "true" else "false"
//        val introducedProperty = RelatedValueFeature(
//            name = createdBy.elementId.toString()+".RelatedValueFeatureFor:"+dependencyString,
//            dependency = dependency,//dependencyString?:"Error",
//            valueSpecs = mutableListOf(XBool.valueOf(valueSpecString)),
//            relatedIndex = varIndex,
//            createdBy = createdBy
//        )
//        introducedProperty.vectorQuantity = if (dependency == "true") Quantity(builder.True) else Quantity(builder.False)//Quantity(builder.conds.getCondition(varIndex) as BDD)
//        introducedProperty.oldVectorQuantity = introducedProperty.vectorQuantity
//        introducedProperty.updated = true
//
//        assert(introducedProperty.isTransient)
//        return introducedProperty
//    }
//
//    private fun createGuardValueFeature(dependencyString: String, path: Solution, pathNr: Int, createdBy: ValueFeature): GuardValueFeature {
//        val introducedProperty = GuardValueFeature(
//            name = createdBy.elementId.toString()+".GuardValueFeature"+pathNr,
//            dependency = dependencyString,
//            valueSpecs = mutableListOf(XBool.valueOf("False")),
//            createdBy = createdBy,
//            paths = path.map
//        )
//        introducedProperty.vectorQuantity = Quantity(builder.False)
//        assert(introducedProperty.isTransient)
//        return introducedProperty
//    }
//
//    private fun registerOrUpdateProperty(property: ValueFeature, owningProperty: ValueFeature): ValueFeature {
//        assert(property.isTransient)
//        val result = model.create(property, owningProperty.owningNamespace!!)//createOrUpdateProperty(property, owningProperty.owningNamespace!!)
//        model.create(SpecializationImplementation(result, model.repo.booleanType!!).also { it.isTransient=true }, result)
//        scheduleProperty(result, owningProperty)
//        return result
//    }
//
//    private fun scheduleProperty(property: ValueFeature, owningProperty: ValueFeature) {
//        assert(property.isTransient)
//        val insertBefore = schedule.indexOf(owningProperty)
//        if (insertBefore <= 0) schedule.add(0, property)
//        else schedule.add(insertBefore-1, property)
//    }
//
//    private fun deleteProperty(property: ValueFeature) {
//        when (property) {
//            is RelatedValueFeature -> {
//                relatedValueFeatures.remove(property)
//            }
//            is GuardValueFeature -> {
//                guards.remove(property)
//            }
//            else -> throw Exception("${property.elementId} not in RelatedProperties or GuardProperties")
//        }
//        model.delete(property)
//        schedule.remove(property)
//    }
//
//    private fun isInequation(property: ValueFeature): Boolean {
//        return (property.dependency.contains(">") || property.dependency.contains("<"))
//    }
//
//    /**
//     * Returns true if property contains at least one inequation.
//     * For future updates: Maybe more efficient to just check dependencies for dd-subtype?
//     */
//    private fun containsInequation(property: ValueFeature): Boolean {
//        val dependecies = property.ast?.root?.getDependencies()
//        if (!dependecies.isNullOrEmpty())  {
//            for (dependency in dependecies) {
//                if (dependency.ast != null) {
//                    if (containsInequation(dependency)) return true
//                }
//                if (isInequation(dependency)) return true
//            }
//        }
//        return isInequation(property) //iif no inequation in dependenciey, maybe it itself is one
//    }
//
//    private fun updateEvaluations(updatedProperty: ValueFeature) {
//        evaluations[updatedProperty] = if (updatedProperty.ast != null) updatedProperty.ast!!.dependency.dd.evaluate() else updatedProperty.vectorQuantity.value.evaluate()
//    }
//
//    private fun updateAnalyzer(updatedProperty: ValueFeature) {
//        ddAnalyzer.updateProperty(updatedProperty)
//        semanticAnalyzer.updateProperty(updatedProperty)
//    }
//
//    //Update function called after conditions are set...
//    private fun updateNoGoods() {
//        noGoods.keys.forEach  {noGoods[it] = noGoods[it]!!.evaluate()}
//    }
//
//    override fun returnToLastState() {
//        //decreaseStep(currentStepCount--)
//    }
//
//    class DiscreteConflictDetectedException(msg: String): Exception(msg)
//    class DiscreteSolverInvalidStepOperationException(msg: String): Exception(msg)
//
//}
