package BaseEntities


import ImportedSysMDServices.resolveName
import Parser.QualifiedName
import java.util.*


/**
 * The identity of an element, given by
 * - id, a UUID for communication outside a running session
 * - reference, a reference in a running session
 * - name, as given by a user and processed in the parser
 */
class Identity<out T: Element>(
    var id: UUID?,
    var ref: @UnsafeVariance T?,
    var str: QualifiedName?
) {
    enum class RefType { ELEMENT, TYPE, NAMESPACE }
    constructor() : this(null, null, null)

    /**
     * Copy-Constructor that copies the reference of the target.
     * @param identity the identity that will be copied.
     */
    constructor(identity: Identity<T>) : this(id=identity.id, ref=identity.ref, str=identity.str)
    constructor(id: UUID?): this(id=id, ref=null, str=null)

    /**
     * Returns a new Identity object of a given Element of type T.
     * All fields are set, including reference.
     */
    constructor(ref: T): this(id=ref.elementId, ref=ref, str=ref.qualifiedName)
    constructor(uid: UUID, ref: T?): this(uid, ref, null)
    constructor(str: QualifiedName): this(null, null, str)

    override fun toString() = when {
        ref != null -> if (ref!!.name != null) ref!!.qualifiedName else  "(unnamed!) " + ref!!.toString()
        id  != null -> id.toString()
        str != null -> "$str?"
        else -> "invalid identity!"
    }


    /**
     * The method resolves the name in the given namespace of the model.
     * @param namespace the namespace where the name resolution starts.
     * @return true, if the reference was changed and updated.
     */
    @Suppress("UNCHECKED_CAST")
    fun resolveIdentity(namespace: Namespace, expectedType: RefType = RefType.ELEMENT): Boolean {
        // First, let's try to find reference via the given name.
        if (str != null) {
            if (str == "it") {
                if (ref !== namespace) {
                    ref = namespace as? T
                    id = ref?.elementId
                    return true
                }
            }
            else {
                // Identifiable via qualified name?
                val found = namespace.resolveName<Element>(str!!)
                if (found !== ref && found != null) {
                    checkTypeOfT(found, expectedType)
                    ref = found as? T
                    id = ref?.elementId
                    return true
                }
            }
        } else if (id != null) {   // Identifiable via the id?
            val found = namespace.model!!.get(id!!) as T?
            if (found !== ref && found != null ) {
                ref = found
                return true
            }
        }
        if (ref != null)
            id = ref?.elementId
        return false
    }

    override operator fun equals(other: Any?) =
        when(other) {
            is Identity<*> -> this.ref === other.ref || this.id == other.id
            is Element -> this.ref === other
            is UUID -> this.id === other
            else -> false
        }

    override fun hashCode(): Int {
        var result = id?.hashCode() ?: 0
        result = 31 * result + (ref?.hashCode() ?: 0)
        result = 31 * result + (str?.hashCode() ?: 0)
        return result
    }

    companion object Factory {
        fun copyOfIdentityList(list: MutableCollection<Identity<Element>>): MutableList<Identity<Element>> {
            val copy = mutableListOf<Identity<Element>>()
            list.forEach { copy += (Identity(it)) }
            copy.forEach { it.ref = null }
            return copy
        }
    }
}

fun checkTypeOfT(ref: Element, expected: Identity.RefType) {
    when(expected) {
        Identity.RefType.TYPE -> {
            if (ref !is Type)
                throw Exception("Wrong type, expecting ${ref.effectiveName} to be a type")
        }
        Identity.RefType.NAMESPACE -> {
            if (ref !is Namespace)
                throw Exception("Wrong type, expecting ${ref.effectiveName} to be a namespace")
        }
        else -> return
    }
}


/**
 * Function that converts a collection of Qualified Names to a Mutable List of Qualified Name Infos.
 */
@JvmName("StringToIdentityList")
fun Collection<QualifiedName>.toIdentityList(): MutableList<Identity<Element>> {
    val result: MutableList<Identity<Element>> = mutableListOf()
    forEach { result.add(Identity(str = it)) }
    return result
}

@JvmName("UuidToIdentityList")
fun Collection<UUID>.toIdentityList(): MutableList<Identity<Element>> {
    val result: MutableList<Identity<Element>> = mutableListOf()
    forEach { result.add(Identity(id = it)) }
    return result}
