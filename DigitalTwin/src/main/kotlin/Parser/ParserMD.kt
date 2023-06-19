package com.github.tukcps.sysmd.parser

import com.github.tukcps.sysmd.entities.AnnotatingElement
import com.github.tukcps.sysmd.entities.Element
import com.github.tukcps.sysmd.entities.implementation.TextualRepresentationImplementation
import com.github.tukcps.sysmd.services.AgilaSession
import org.commonmark.Extension
import org.commonmark.ext.front.matter.YamlFrontMatterBlock
import org.commonmark.ext.front.matter.YamlFrontMatterExtension
import org.commonmark.ext.gfm.tables.TablesExtension
import org.commonmark.node.*
import org.commonmark.parser.IncludeSourceSpans
import org.commonmark.parser.Parser
import java.io.File


/**
 * This class maintains the currently active section headings
 * and the owning elements that store the MD Nodes.
 */
class HeadingMgr(private val document: Element) {
    private val activeHeadings = HashMap<Int, Element>()

    /**
     * This method must be called when a new heading / section level is entered.
     * @param level the level o the heading as given by MD
     */
    fun enterHeadingLevel(level: Int): Element {
        var smallestSection: Element = document
        for(i in 1 .. 5) {
            if (activeHeadings[i] != null) {
                if (i > level) activeHeadings.remove(i)
                else if (i < level) smallestSection = activeHeadings[i]!!
            }
        }
        return smallestSection
    }

    fun getHeadingElement(): Element = enterHeadingLevel(5)

    fun setHeadingElement(level: Int, element: Element) {
        activeHeadings[level] = element
    }
}


/**
 * Walks over the MD AST that is a tree of the syntactical elements;
 * it does NOT have hierarchy leaning towards headings etc.
 * However, we create a hierarchical level whenever a Heading is read.
 * This function gets a source span between Heading/CodeBlock a separate Textual elements.
 * @param node
 * @param inputLines A list with all input lines.
 * @return
 */
fun getMdSource(node: Node, inputLines: List<String>): String {
    // Str collects the processed textual representation from MD.
    var str = ""
    val firstLineIndex = node.sourceSpans.first().lineIndex

    if (node is YamlFrontMatterBlock) {
        val lineIndex = node.sourceSpans.first().lineIndex
        val lastLineIndex = node.sourceSpans.last().lineIndex
        for (i in lineIndex .. lastLineIndex)
            str += inputLines[i] + "\n"
        return str
    }

    var iter: Node? = node.next
    var last: Node = node
    while(iter != null) {
        when (iter) {
            // overall document; top-level.
            // We just iterate over the different syntactical elements of an MD document.
            is Heading, is FencedCodeBlock -> {
                val lineIndex = iter.sourceSpans.first().lineIndex-1
                for (i in firstLineIndex .. lineIndex)
                    str += inputLines[i] + "\n"
                return str
            }

            // other Blocks, we just collect lines of its sourceSpans and add it to str ...
            else -> {}
        }
        last = iter
        iter = iter.next
    }
    val lineIndex = last.sourceSpans.last().lineIndex
    for (i in firstLineIndex .. lineIndex)
        str += inputLines[i] + "\n"
    return str
}

fun AgilaSession.importMD(file: File, owningAnnotation: AnnotatingElement) {
    val inputString = file.bufferedReader().use { it.readText() }
    importMD(inputString, owningAnnotation)
}


/**
 * Parses the input given as string and adds Annotation Elements in the model.
 * After reading the Markdown input, there will be Annotation Elements that carry
 * SysMD-Code or Documentation in Markdown format.
 * @param input The input in Markdown format.
 * @param owningAnnotation The annotation
 */
fun AgilaSession.importMD(input: String, owningAnnotation: AnnotatingElement) {
    val inputLines = input.lines()

    // For evaluation: we can generate hierarchically structured models, or flat models.
    // Intermediate result:
    // - Flat models are better for UI / Model communication (easier view model - model synchronization);
    //   no huge changes in model when re-structuring e.g. from subsection to section.
    // - Hierarchical models however are easier to analyze as the hierarchical structure is
    //   available
    val hierarchical = false
    val extensions: List<Extension> = listOf(TablesExtension.create(), YamlFrontMatterExtension.create())

    // We use the Commonmark Markdown-Parser and include SourceSpans.
    val parser: Parser = Parser.builder()
        .extensions(extensions)
        .includeSourceSpans(IncludeSourceSpans.BLOCKS_AND_INLINES).build()

    val document: Node = parser.parse(input)

    // The currently edited heading level. As a hashmap Level -> Namespace.
    val headings = HeadingMgr(owningAnnotation)

    require(document is Document)
    var node = document.firstChild
    var afterCodeBlock = false
    var beforeFirstHeading = true
    while(node != null) {
        when(node) {

            is FencedCodeBlock -> {
                val language = node.info.ifEmpty { "SysMD" }
                if (hierarchical)
                    create(TextualRepresentationImplementation(language = language, body = node.literal.trim('\n')), headings.getHeadingElement())
                else
                    create(TextualRepresentationImplementation(language = language, body = node.literal.trim('\n')), owningAnnotation)
                afterCodeBlock = true
            }
            is Heading -> {
                afterCodeBlock = false
                beforeFirstHeading = false
                val str = getMdSource(node, inputLines)
                if (hierarchical) {
                    val newHeading = create(
                        TextualRepresentationImplementation(language = "Markdown", body = str),
                        headings.enterHeadingLevel(node.level)
                    )
                    headings.setHeadingElement(node.level, newHeading)
                } else
                create(TextualRepresentationImplementation(language = "Markdown", body = str), owningAnnotation)
            }
            is YamlFrontMatterBlock,
            is Block -> {
                // FencedCodeBlock and Heading include all respective
                if (afterCodeBlock || beforeFirstHeading) {
                    val str = getMdSource(node, inputLines)
                    if (hierarchical)
                        create(TextualRepresentationImplementation(language = "Markdown", body=str), headings.getHeadingElement())
                    else
                        create(TextualRepresentationImplementation(language = "Markdown", body=str), owningAnnotation)
                    afterCodeBlock = false
                }
            }
        }
        node = node.next
    }
}
