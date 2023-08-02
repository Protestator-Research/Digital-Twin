package BaseEntities

interface MetadataFeature: Feature, AnnotatingElement {
    override fun clone(): MetadataFeature
}