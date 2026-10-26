//---------------------------------------------------------
// Blocks (Markdown, code, YaML) of a digital twin document and their order.
//
// The server returns the elements of a commit sorted by UUID. The order of the blocks of a document is therefore kept
// in a root Package: it owns one OwningMembership per block, in document order (Package.ownedRelationship), and each
// membership owns its block (OwningMembership.ownedRelatedElement).
//---------------------------------------------------------
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../cpp_digital_twin_lib_global.h"

namespace KerML::Entities {
    class Element;
    class TextualRepresentation;
}

namespace SysMLv2::REST {
    class CommitRequest;
}

namespace DigitalTwin::Model {
    /**
     * Returns the blocks that are part of the digital twin model code, in document order: the TextualRepresentations
     * whose language is none of "Markdown", "YaML" and "HTML" (an empty language, e.g. a fence without info string, is code) and whose body is not empty. Elements of any other type (Package,
     * OwningMembership, ...) and null pointers are skipped.
     *
     * Order: The root Package (the Package without owner) lists its OwningMemberships in document order; the blocks
     * follow this order. Blocks the Package does not reference are appended in the order they were given.
     * Fallbacks: without a root Package (older projects) the given order (the server sorts by UUID) is kept; with
     * several root Packages the first one by UUID is used. Both cases are reported as a warning.
     * @param elements The elements of a commit, in any order.
     * @param warnings If not null, the warnings are appended; otherwise they are written to std::cerr.
     */
    CPPDIGITALTWINLIB_EXPORT std::vector<std::shared_ptr<KerML::Entities::TextualRepresentation>>
    codeBlocksInOrder(const std::vector<std::shared_ptr<KerML::Entities::Element>>& elements,
                      std::vector<std::string>* warnings = nullptr);

    /**
     * The model code of a commit: the bodies of codeBlocksInOrder() concatenated. This is the text the digital twin
     * model is parsed from (DigitalTwinModel::buildDigitalTwinModel).
     */
    CPPDIGITALTWINLIB_EXPORT std::string
    assembleModelCode(const std::vector<std::shared_ptr<KerML::Entities::Element>>& elements,
                      std::vector<std::string>* warnings = nullptr);

    /**
     * Result of createDocumentCommit.
     */
    struct CPPDIGITALTWINLIB_EXPORT DocumentCommit {
        /// The elements that are sent: the root Package, then per block its OwningMembership and the block.
        std::vector<std::shared_ptr<KerML::Entities::Element>> Elements;
        /// The commit request that creates all elements.
        std::shared_ptr<SysMLv2::REST::CommitRequest> Request;
    };

    /**
     * Creates the first commit of a document: a root Package with the given name and, for each block in the given
     * order, an OwningMembership and the block (with the references owner and owningRelationship). The blocks are
     * copied, the given objects stay unchanged.
     * @param packageName Name of the root Package, normally the project name.
     * @param blocks All blocks of the document (Markdown, code, YaML) in document order.
     * @param description Description of the commit.
     * @throws std::invalid_argument if there is no block (an empty commit is rejected by the server).
     */
    CPPDIGITALTWINLIB_EXPORT DocumentCommit
    createDocumentCommit(const std::string& packageName,
                         const std::vector<std::shared_ptr<KerML::Entities::Element>>& blocks,
                         const std::string& description = "Initial import of the document");
}
