package ImportedSysMDServices


import BaseEntities.Namespace
import BaseEntities.ValueFeature
import BaseEntities.implementation.SpecializationImplementation
import BaseEntities.implementation.ValueFeatureImplementation
import ImportedSysMDExceptions.ElementNotFoundException
import ImportedSysMDExceptions.SemanticError
import ImportedSysMDQuantities.Quantity
import ImportedSysMDServices.resolveName
import com.github.tukcps.jaadd.*
import com.github.tukcps.jaadd.values.XBool


/**
 * Direct interface to the internal model:
 * some extensions and definitions for convenience.
 * AVOID ITS USE; MOSTLY INTENDED FOR TESTING.
 */
fun AgilaSession.letVar(value: ValueFeature, dd: DD): ValueFeature { //TODO not for Vectors
    require(dd.builder === builder)

    when (dd) {
        is AADD -> value.valueSpecs = mutableListOf(dd.getRange())
        is IDD  -> value.valueSpecs = mutableListOf(dd.getRange())
        is BDD  -> value.valueSpecs = mutableListOf(dd.value)
        else -> throw SemanticError("parameter must be of subtype of DD")
    }

    value.vectorQuantity = when(dd){
        is AADD ->  Quantity(dd, value.unitSpec)
        is BDD ->  Quantity(dd)
        is IDD ->  Quantity(dd)
        else -> {throw SemanticError("Unsupported type for ${value.vectorQuantity}.")
        }
    }


    //Sync with Conditions
    if (builder.conds.indexes[value.elementId.toString()] != null)
        builder.conds.x[builder.conds.indexes[value.elementId.toString()]!!] = dd

    return value
}


/**
 * Updates a value in the model.
 */
fun AgilaSession.letVar(qualifiedName: String, value: DD): ValueFeature {
    require(value.builder == builder)
    val property = global.resolveName<ValueFeature>(qualifiedName)
        ?: throw ElementNotFoundException("Not found in scope Global:  $qualifiedName")
    return letVar(property, value)
}


/**
 * Creates a value feature in the model.
 */
fun AgilaSession.defVectorVar(name: String, values: List<DD>, unitStr: String = "", namespace: Namespace = global) {
    require(values[0].builder === builder)
    val property: ValueFeature
    when (values[0]) {
        // this does not look consistent here.  BDD assigns value.  AADD & IDD do NOT.  Is this intentional?
        is AADD -> {
            val ranges = mutableListOf<Any?>()
            values.forEach { ranges.add(it.asAadd().getRange()) }
            property = ValueFeatureImplementation(
                name = name,
                valueSpecs = ranges,
                unitSpec = unitStr
            )
            create(property, namespace)
            create(SpecializationImplementation(property, repo.realType!!), property)
        }

        is BDD -> {
            val boolSpecs = mutableListOf<Any?>()
            values.forEach {
                boolSpecs.add(
                    when (it) {
                        builder.False -> XBool.False
                        builder.True -> XBool.True
                        else -> XBool.X
                    }
                )
            }

            property = ValueFeatureImplementation(name = name, valueSpecs = boolSpecs)
            create(property, namespace)
            create(SpecializationImplementation(property, repo.booleanType!!), property)
        }

        is IDD -> {
            val ranges = mutableListOf<Any?>()
            values.forEach { ranges.add(it.asIdd().getRange()) }
            property = ValueFeatureImplementation(name = name, valueSpecs = ranges)
            create(property, namespace)
            create(SpecializationImplementation(property, repo.integerType!!), property)
        }

        else -> throw SemanticError("parameter must be of subtype of DD")
    }
}

fun AgilaSession.defScalarVar(name: String, value: DD, unitStr: String = "", namespace: Namespace = global) =
    defVectorVar(name, mutableListOf(value),unitStr,namespace)