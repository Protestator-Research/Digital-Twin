//
// Created by Moritz Herzog on 22.02.24.
//
//---------------------------------------------------------
// Constants, Definitions, Pragmas
//---------------------------------------------------------
#ifndef DIGITALTWIN_JSONENTITIES_H
#define DIGITALTWIN_JSONENTITIES_H
//---------------------------------------------------------
// External Classes
//---------------------------------------------------------
#include <string>
//---------------------------------------------------------
// Internal Classes
//---------------------------------------------------------

//---------------------------------------------------------
// Forwarding
//---------------------------------------------------------

namespace SysMLv2::Entities {
    //Generally used
    const std::string JSON_ID_ENTITY = "@id";
    const std::string JSON_TYPE_ENTITY = "@type";
    //Used for Record and the Superclasses
    const std::string JSON_NAME_ENTITY = "name";
    const std::string JSON_ALIAS_ENTITY = "alias";
    const std::string JSON_DESCRIPTION_ENTITY = "description";
    //Used for Project
    const std::string JSON_DEFAULT_BRANCH_ENTITY = "defaultBranch";
    const std::string JSON_CREATION_ENTITY = "created";
    const std::string JSON_BRANCHES_LIST = "branches_list";
    const std::string JSON_COMMIT_LIST = "commits";
    const std::string JSON_HEAD_ID_LIST = "heads";
    //Used for Digital Twin
    const std::string JSON_CONNECTED_MODELS = "connectedModels";
    const std::string JSON_PARENT_PROJECT = "parentProject";
    const std::string JSON_BRANCH_ID = "branchId";
    const std::string JSON_COMMIT_ID = "commitId";




    //Type Strings
    const std::string PROJECT_TYPE="project";
    const std::string DATA_IDENTITY_TYPE="dataidentity";
    const std::string BRANCH_TYPE="branch";
    const std::string TAG_TYPE = "tag";
    const std::string QUERY_TYPE = "query";

    const std::string COMMIT_TYPE = "commit";
    const std::string FEATURE_TYPE = "feature";
    const std::string FEATURE_TYPE_TYPE = "featuretyping";
    const std::string ASSOCIATION_TYPE = "association";
    const std::string ELEMENT_TYPE = "element";
    const std::string SPECIALIZATION_TYPE = "specialization";
    const std::string PACKAGE_TYPE = "package";
    const std::string EXPRESSION_TYPE = "expression";
    const std::string MULTIPLICITY_TYPE = "multiplicity";
    const std::string DATA_TYPE_TYPE = "datatype";
    const std::string RELATIONSHIP_TYPE="relationship";
    const std::string COMMENT_TYPE="comment";
    const std::string DOCUMENTATION_TYPE="documentation";
    const std::string CLASS_TYPE = "class";
    const std::string ANNOTATING_ELEMENT_TYPE = "annotatingelement";
    const std::string TEXTUAL_REPRESENTATION_TYPE = "textualrepresentation";
    const std::string CLASSIFIER_TYPE = "classifier";
    const std::string NAMESPACE_TYPE = "namespace";
    const std::string TYPE_TYPE = "type";
    const std::string DIGITAL_TWIN_TYPE = "digitaltwin";
}


#endif //DIGITALTWIN_JSONENTITIES_H
