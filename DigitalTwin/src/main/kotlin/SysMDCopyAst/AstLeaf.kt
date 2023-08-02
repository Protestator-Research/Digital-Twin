package SysMDCopyAst

import BaseEntities.Feature
import BaseEntities.Namespace
import BaseEntities.ValueFeature
import ImportedSysMDExceptions.ElementNotFoundException
import ImportedSysMDQuantities.VectorDimensionError
import ImportedSysMDQuantities.VectorQuantity
import ImportedSysMDServices.AgilaSession
import ImportedSysMDServices.reportInfo
import ImportedSysMDServices.resolveName
import Parser.QualifiedName
import com.github.tukcps.jaadd.AADD
import com.github.tukcps.jaadd.IDD


/**
 * @class AstLeaf
 * A leaf of the abstract syntax tree with the following properties:
 * - its main information is in a property data type
 * - it is entered in the symbol table
 *   (unless it is a number/bool literal)
 */
class AstLeaf(model: AgilaSession, vectorQuantity: VectorQuantity? = null) : AstNode(model) {

    /**
     * The elementId and the path from this element to the property.
     * These two variables are initialized by the constructor and used
     * in initialize after a complete file has been read to get the property
     * of the AstLeaf.
     */
    var literalVal: VectorQuantity? = vectorQuantity // The value, if a literal

    // Iff not a literal:
    private var namespace: Namespace? = null      // Owning namespace
    var qualifiedName: QualifiedName? = null
    var property: ValueFeature? = null       // Reference to property after initialization.

    /**
     * If the leaf is a literal, we can initialize the values now.
     */
    init {
        if (vectorQuantity != null) {
            upQuantity = vectorQuantity.clone()
            downQuantity = vectorQuantity.clone()
        }
    }

    override fun initialize() {
        if (qualifiedName != null) {
            // A variable that must be found in the properties
            property = if(namespace==null) {
                property?.owningNamespace?.resolveName(qualifiedName!!)
                    ?: throw ElementNotFoundException(qualifiedName!!)
            } else {
                namespace!!.resolveName<ValueFeature>(qualifiedName!!)
                    ?: throw ElementNotFoundException(qualifiedName!!)
            }
            upQuantity = property!!.vectorQuantity
            downQuantity = property!!.vectorQuantity
        } else {
            // No variable, hence we MUST have a literal.
            if(literalVal!=null) {
                upQuantity = literalVal!!
                downQuantity = literalVal!!
            }
        }
        evalUp()
    }

    /**
     * A constructor that saves the scope information from parsing for later initialization
     * after the complete file has been read. The constructor saves element id and name of the
     * property, and creates the property entries value etc. later during lateInit().
     * @param namespace the element in which the property is queried.
     * @param qualifiedName the path of the property, relative from the element.
     */
    constructor(namespace: Namespace, qualifiedName: QualifiedName, model: AgilaSession) : this(model) {
        require(qualifiedName != "")
        this.namespace = namespace
        this.qualifiedName = qualifiedName
    }

    /**
     * A constructor that is directly invoked with an available property.
     * NOTE: Must remain in memory.
     */
    constructor(model: AgilaSession, property: ValueFeature) : this(model) {
        this.property = property
        this.qualifiedName = property.effectiveName
    }


    // Eval down stops recursion.
    override fun evalDownRec() {
        evalDown()
    }

    /**
     * Eval down computes intersection with specified range as value.
     * Before computing intersection with the range specification, both down value and
     * value must be converted to the same unit (SI Unit). (is now checked at the beginning)
     * At leaves, the evalDown computes the INTERSECTION between the down-propagated value and the
     * already constrained value.
     */
    override fun evalDown() {
        if (property != null) {
            if (isReal) {
                property!!.vectorQuantity = downQuantity.constrain(
                    property!!.vectorQuantity,
                    property!!.rangeSpecs,
                    property!!.unitSpec
                )
                if (property!!.direction == Feature.FeatureDirectionKind.OUT) {
                    if (property!!.rangeSpecs.size != downQuantity.values.size && property!!.rangeSpecs.size != 1)
                        throw VectorDimensionError("Vector size of ${downQuantity.values.size} does not match Constraint size of ${property!!.rangeSpecs.size}")
                    if (property!!.rangeSpecs.size == downQuantity.values.size)
                        if (property!!.rangeSpecs.indices.any { property!!.rangeSpecs[it] !in (downQuantity.values[it] as AADD).getRange() })
                            model.reportInfo(property!!, "Cannot be satisfied for all values.")
                }
                property!!.checkEvent()
            }
            if (isInt) {
                property!!.vectorQuantity = downQuantity.constrain(property!!.vectorQuantity)
                if (property!!.direction == Feature.FeatureDirectionKind.OUT) {
                    if (property!!.intSpecs.size != downQuantity.values.size && property!!.rangeSpecs.size != 1)
                        throw VectorDimensionError("Vector size of ${downQuantity.values.size} does not match Constraint size of ${property!!.rangeSpecs.size}")
                    if (property!!.intSpecs.size == downQuantity.values.size)
                        if (property!!.intSpecs.indices.any { property!!.intSpecs[it] !in (downQuantity.values[it] as IDD).getRange() })
                            model.reportInfo(property!!, "Cannot be satisfied for all values.")
                }
                property!!.checkEvent()
            }
        }
    }


    /**
     *  EvalUp just stops recursion as we are at a leaf.
     */
    override fun evalUpRec() {
        evalUp()
    }

    /**
     * Propagates the variable or literal value up.
     * The value is initialized whenever a user specifies a value;
     * constraint propagation only restricts this value to an intersection of it.
     * All values are in the unit in SI representation.
     */
    override fun evalUp() {
        if (property != null) {
            upQuantity = property!!.vectorQuantity
        }
    }


    /** Executes a block of statements on each AstNode in an Ast */
    override fun <R> runDepthFirst(block: AstNode.() -> R): R =
        this.run(block)

    override fun <R> withDepthFirst(receiver: AstNode, block: AstNode.() -> R): R =
        receiver.block()


    override fun clone(): AstLeaf {
        return if(property == null && qualifiedName == null){ //if there is a upQuantity, but no property/QualifiedName is defined
            AstLeaf(model).also {
                it.qualifiedName = qualifiedName
                it.namespace = namespace
                it.upQuantity = upQuantity
                it.downQuantity = downQuantity
            }
        }else AstLeaf(model).also {
            it.qualifiedName = qualifiedName
            it.namespace = namespace
            it.property = property
        }
    }

    // Returns AST as simple expression string
    override fun toExpressionString(): String {
        return if (qualifiedName == null) { // A literal ...
            if (literalVal!!.values[0] is AADD) {
                var valStr = literalVal!!.values.toString()
                if (valStr.split(".").size == 1) valStr += ".0"
                var unitStr = literalVal!!.unit.toString()
                unitStr = if (unitStr != "1") unitStr else ""
                "$valStr $unitStr"
            } else
                literalVal.toString()
        } else
            qualifiedName!!
    }


    override fun toString(): String {
        return try {
            // Literal?
            if (literalVal != null) "AstLeaf($upQuantity)"
            else {
                // Property/variable, defined by identification & ElementId.
                // After initialization, we also have upQuantity initialized with value.
                if (qualifiedName == null) "AstLeaf($upQuantity)"
                else "AstLeaf($qualifiedName = $upQuantity)"
            }
        } catch (e: Exception) {
            if (literalVal != null) "AstLeaf($literalVal)" else "AstLeaf(${namespace?.effectiveName} : ${qualifiedName!!})"
        }
    }
}
