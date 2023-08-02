package SysMDRestImport.entities

import java.util.UUID


/**
 * Abstract baseclass that represents any data that is consumed or produced
 * by the Agila Backend API.
 */
interface Record {
    /**
     * The UUID
     */
    var id: UUID
    /**
     * Human-friendly unique identifier
     */
    var name: String
    /**
     * A statement that provides Details
     */
    var description: String
//    var alias: ArrayList<String>
}