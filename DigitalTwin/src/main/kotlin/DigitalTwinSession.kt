import com.github.tukcps.jaadd.DDBuilder
import com.github.tukcps.sysmd.cspsolver.DiscreteSolver
import com.github.tukcps.sysmd.entities.*
import com.github.tukcps.sysmd.entities.implementation.PackageImplementation
import com.github.tukcps.sysmd.parser.QualifiedName
import com.github.tukcps.sysmd.rest.ElementDAO
import com.github.tukcps.sysmd.services.*
import java.util.*

class DigitalTwinSession(
    id: UUID = UUID.randomUUID(),
    status: SessionStatus = SessionStatus(),
    settings: SessionSettings = SessionSettings(),
    builder: DDBuilder = DDBuilder()
) : AgilaSessionImpl(id, status, settings, builder) {
    operator fun set(elementId: UUID, value: Element) {
        repo.elements.set(elementId,value)
    }

}