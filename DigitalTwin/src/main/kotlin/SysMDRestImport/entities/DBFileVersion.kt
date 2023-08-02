package SysMDRestImport.entities

import com.fasterxml.jackson.annotation.JsonProperty
import java.util.*

/**
 * @author Ihsn Hadri
 *
 * Class for a documentation file Versions
 */
//@NoArgsConstructor
//@Getter
//@Setter
//@Document("FilesVersion")
class DBFileVersion(
    private val originalFileId: String,
    private var fileName: String,
    private var fileType: String,
    private var data: ByteArray,
    private val version: Int
) {
    //    @Id
    @JsonProperty("@id")
    private val id: String? = null
    fun updateFile(fileName: String, fileType: String, data: ByteArray) {
        this.fileName = fileName
        this.fileType = fileType
        this.data = data
    }

    override fun toString(): String {
        return "DBFileVersion{" +
                "id='" + id + '\'' +
                ", fileName='" + fileName + '\'' +
                ", fileType='" + fileType + '\'' +
                ", version='" + version + '\'' +
                ", data=" + Arrays.toString(data) +
                '}'
    }
}
