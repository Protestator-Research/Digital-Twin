package SysMDRestImport.entities

import com.fasterxml.jackson.annotation.JsonProperty
import java.util.*

/**
 * @author Khushnood Adil Rafique, Christoph Grimm
 *
 * Class for a documentation file to be stored in the db.
 */
//@NoArgsConstructor
//@Getter
//@Setter
//@Document("Files")
class DBFile(private var fileName: String, private var fileType: String, private var data: ByteArray) {
    @JsonProperty("@id")
    private val id: String? = null
    fun updateFile(fileName: String, fileType: String, data: ByteArray) {
        this.fileName = fileName
        this.fileType = fileType
        this.data = data
    }

    override fun toString(): String {
        return "DBFile{" +
                "id='" + id + '\'' +
                ", fileName='" + fileName + '\'' +
                ", fileType='" + fileType + '\'' +
                ", data=" + Arrays.toString(data) +
                '}'
    }
}
