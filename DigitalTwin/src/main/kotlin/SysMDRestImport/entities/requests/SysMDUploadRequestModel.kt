package SysMDRestImport.entities.requests

/**
 * Request model for the REST API for the upload of SysMD files.
 * The request model requires a mode 'parserModeDescription' that
 * specifies if the code starts in Markdown-mode or SysMD mode.
 * If true, the parser assumes that the code is Markdown with embedded SysMD;
 * if false, the parser expects SysMD code.
 * The code is a string with the SysMD or Markdown code that will be parsed.
 */
class SysMDUploadRequestModel(
    var parserModeDescription: Boolean = true,
    var code: String = "",
    var filename: String = "Specify filename!"
)