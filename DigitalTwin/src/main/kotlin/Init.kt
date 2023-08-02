
import java.io.File
import kotlin.system.exitProcess

object Init {
    val version = this::class.java.classLoader.getResource("version")?.readText()

    fun initProjects() {

    }
}
