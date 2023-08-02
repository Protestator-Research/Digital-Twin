package SysMDRestImport.entities.requests.commitData

class CommitType {
    companion object {
        const val ELEMENT: String = "Element"
        const val RELATIONSHIP: String = "Relationship"
        const val PACKAGE: String = "Package"
        const val ANNOTATION: String = "Annotation"
        const val NAMESPACE_IMPORT: String = "NamespaceImport"
        const val MEMBERSHIP_IMPORT: String = "MembershipImport"
        const val MULTIPLICITY: String = "Multiplicity"
        const val SPECIALIZATION: String = "Specialization"
        const val CLASSIFIER: String = "Classifier"
        const val CLASS: String = "Class"
        const val TYPE: String = "Type"
        const val FEATURE: String = "Feature"
        const val FEATURE_VALUE: String = "FeatureValue"
        const val NAMESPACE: String = "Namespace"
        const val DOCUMENTATION: String = "Documentation"
        const val ANNOTATING_ELEMENT: String = "AnnotatingElement"
        const val TEXTUAL_REPRESENTATION: String = "TextualRepresentation"
        const val COMMENT: String = "Comment"
        const val ASSOCIATION: String = "Association"
    }
}