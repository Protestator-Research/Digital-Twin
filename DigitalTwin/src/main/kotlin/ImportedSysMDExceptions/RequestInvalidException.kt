package ImportedSysMDExceptions

class RequestInvalidException(message: String):
    RuntimeException(message) {
    var errorMessage: String? = message

    override fun toString(): String {
        return "AGILA: $message"
    }
}
