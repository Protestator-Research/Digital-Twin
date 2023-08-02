package SysMDImportSolver.normalizer

import ImportedSysMDServices.AgilaSession
import com.github.tukcps.jaadd.values.XBool

/**
 * Class to store the normalized properties produced by the [CNNormalizer]
 */
data class NormalizedProperties(

    /**
     * Normalized properties
     */
    val properties: MutableList<SimpleProperty<XBool>>,

    /**
     * The model for which the properties were normalized
     */
    val model : AgilaSession
    ) : Cloneable {

    //override fun copy()
    public override fun clone(): NormalizedProperties {
        return NormalizedProperties(this.properties.toMutableList(), this.model)
    }
}