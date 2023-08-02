package SysMDImportSolver.propagator

import BaseEntities.ValueFeature
import ImportedSysMDServices.AgilaSession


abstract class Propagator(override val model: AgilaSession):PropagatorIF {

    abstract override fun execute(updatedValueFeature: ValueFeature)

}