package SysMDCopyAst.functions

import ImportedSysMDExceptions.SemanticError
import ImportedSysMDQuantities.Quantity
import ImportedSysMDServices.AgilaSession
import SysMDCopyAst.AstLeaf
import SysMDCopyAst.AstNode
import SysMDCopyAst.AstRoot
import com.github.tukcps.jaadd.AADD
import com.github.tukcps.jaadd.BDD
import com.github.tukcps.jaadd.IDD

class AstEnum(model: AgilaSession, args: ArrayList<AstNode>, val enumCount: Int = args.size) :
    AstFunction("enum", model, enumCount, args) {

    lateinit var introducedDecisionVariables: ArrayList<AstNode>
        private set

    internal var ites = arrayListOf<AstIte>()
    internal lateinit var lastIte: AstIte
    lateinit var propertyUUIDString: String

    init {}

    override fun initialize() {



        val rootRoot = this.root!! as AstRoot
        propertyUUIDString = rootRoot.property.elementId.toString()

        if (findDuplicateElements()) throw SemanticError("Enumeration $propertyUUIDString contains duplicate elements.")


        val tmpItes = mutableListOf<AstIte>()
        val numConds = enumCount-1 //we need one condition less than enumerated values
        val newConds = arrayListOf<AstNode>()


        var last = parameters[numConds]
        var secondToLast = parameters[numConds-1]

        newConds.add(AstLeaf(model, Quantity(model.builder.variable(propertyUUIDString+"Condition1"))))

        val firstIte = AstIte(model, arrayListOf<AstNode>().also { it.add(newConds.last()) }.also { it.add(secondToLast) }.also { it.add(last) })
        firstIte.initialize()
        ites.add(firstIte)
        tmpItes.add(firstIte)

        for (i in numConds-2 downTo 0) {
            val newCond = newConds.last()
            val e = tmpItes.last()
            val t = parameters[i]
            newConds.add(AstLeaf(model, Quantity(model.builder.variable(propertyUUIDString+"Condition${enumCount-i}"))))
            tmpItes.add(AstIte(model, arrayListOf<AstNode>().also { it.add(newConds.last()) }.also { it.add(t) }.also { it.add(e) }).also { it.initialize() })
        }


        ites.addAll(tmpItes)
        lastIte = tmpItes.last()
        introducedDecisionVariables = newConds

        lastIte.root = this

        //Type checking for arguments! => Should be done by the astites

        when (lastIte.upQuantity.value) {
            is BDD -> upQuantity = Quantity(model.builder.Bool)
            is IDD -> upQuantity = Quantity(model.builder.Integers)
            is AADD -> upQuantity = Quantity(model.builder.Reals,"?")
            else -> throw Exception("TODO")
        }
        evalUp()
        downQuantity = upQuantity.clone()
        //println("done initializing")
    }

    override fun evalUp() {
        lastIte.evalUp()
        upQuantity = lastIte.upQuantity
    }

    override fun evalUpRec() {
        lastIte.evalUpRec()
    }

    override fun evalDown() {
        lastIte.evalDown()
        downQuantity = lastIte.downQuantity
    }

    override fun evalDownRec() {
        lastIte.evalDownRec()
    }

    override fun clone(): AstEnum {
        val parClone = ArrayList<AstNode>()
        for (p in parameters) parClone.add(p.clone())
        return AstEnum(model, parClone)
    }

    override fun toExpressionString(): String {
        return lastIte.toExpressionString()
    }

    private fun findDuplicateElements() : Boolean {
        val findDuplicateParams = mutableListOf<String>()
        parameters.mapTo(findDuplicateParams) { it.dd.toString()}

        return findDuplicateParams.distinct().count() != parameters.size
    }
}