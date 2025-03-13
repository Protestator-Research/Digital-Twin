/**
 * Implementation of the KerML Beta 2 Standard for ANTLR 4 Parsing
 * @author Moritz Herzog
 **/

grammar KerML;

start: elements EOF;

startRule: start;

elements: element*;

//element:  //namespace |
        //classifier |
        //feature |
        //intenalCodeComment |
        //dependency |
//        comment;// |
        //documentation |
        //visibility |
        //class |
        //datatype |
        //alias |
        //importRule |
        //type |
        //abstract |
        //package |
        //composite;

//comment: REGULAR_COMMENT | SINGLE_LINE_NOTE;
identification: ('<' NAME '>')? | (NAME)?;
relationshipBody: SYMBOL_STATEMENT_DELIMITER | SYMBOL_CURLY_BRACKET_OPEN relationshipOwnedElement* SYMBOL_CURLY_BRACKET_CLOSE;
relationshipOwnedElement: ownedRelatedElem += ownedRelatedElement | ownedRelationship += ownedAnnotation;
ownedRelatedElement: non_feature_element | feature_element;
dependency: (ownedRelationShip = prefix_metadata_annotation)* KEYWORD_DEPENDENCY (identification KEYWORD_FROM)? client += qualifiedName (SYMBOL_COMMA client += qualifiedName)* KEYWORD_TO supplier += qualifiedName (SYMBOL_COMMA supplier += qualifiedName)* relationshipBody;
annotatedElement: qualifiedName;
ownedAnnotation: annotatedElement = annotatingElement;
annotatingElement: comment | documentation | textualRepresentation | metadataFeature;
comment: (KEYWORD_COMMENT identification (KEYWORD_ABOUT annotation += annotatedElement {ownedRelationship += annotation} (SYMBOL_COMMA annotation += annotatedElement {ownedRelationship += annotation})*)?)? ('locale' locale = STRING_VALUE)? body = REGULAR_COMMENT;
documentation: KEYWWORD_DOC identification ('locale' locale = STRING_VALUE)? body = REGULAR_COMMENT;
textualRepresentation: (KEYWORD_REP identification)? KEYWORD_LANGUAGE language = STRING_VALUE body = REGULAR_COMMENT;

root_namespace: namespace_body_element*;
namespace: (ownedRelationship += prefix_metadata_member)* namespace_declaration namespace_body;
namespace_declaration: KEYWORD_NAMESPACE identification;
namespace_body: SYMBOL_STATEMENT_DELIMITER | SYMBOL_CURLY_BRACKET_OPEN namespace_body_element* SYMBOL_CURLY_BRACKET_CLOSE;
namespace_body_element: ownedRelationship += namespace_member | alias_member | namespace_import;
member_prefix: (visibility = visibility_indicator)?;
visibility_indicator: KEYWORD_PUBLIC | KEYWORD_PRIVATE | KEYWORD_PROTECTED;
namespace_member: non_feature_member | namespace_feature_member;
non_feature_member: member_prefix ownedRelatedElement += non_feature_element;
namespace_feature_member: member_prefix ownedRelatedElement += feature_element;
alias_member: member_prefix KEYWORD_ALIAS (SYMBOL_SMALLER memberShortName = NAME SYMBOL_GREATER)? (memberName = NAME)? KEYWORD_FOR memberElement = qualifiedName relationshipBody;
qualifiedName: (NAME SYMBOL_NAMESPACE_SUBSET)* NAME;

namespace_import: (visibility = visibility_indicator)? KEYWORD_IMPORT (isImportAll = KEYWORD_ALL)? import_declaration relationshipBody;
import_declaration: membership_import | namespace_import;
membership_import: imported_membership = qualifedName (SYMBOL_NAMESPACE_SUBSET isRecursive = SYMBOL_DOUBLE_STAR)?;
filter_package: ownedRelationship += import_declaration (ownedRelationship += filter_package_member);
filter_package_member: SYMBOL_SQUARE_BRACKET_OPEN ownedRelatedElement += ownedExpression SYMBOL_SQUARE_BRACKET_CLOSE {visibility = 'private'};

element: annotatingElement | non_feature_element | feature_element;
non_feature_element: dependency |
                    type |
                    classifier |
                    data_type |
                    namespace |
                    class |
                    structure |
                    metaclass |
                    association |
                    association_structure |
                    interaction |
                    behavior |
                    function |
                    predicate |
                    multiplicity |
                    package |
                    library_package |
                    specialization |
                    conjunction |
                    subsclassification |
                    disjoining |
                    feature_inverting |
                    feature_typing |
                    subsetting |
                    redefinition |
                    type_featuring;

feature_element: feature |
                 step |
                 expression |
                 boolean_expression |
                 invariant |
                 connector |
                 binding_connector |
                 succession |
                 item_flow |
                 succession_item_flow;

type: type_prefix KEYWORD_TYPE type_declaration type_body;
type_prefix: (isAbstract = KEYWORD_ABSTRACT)? (ownedRelationship += prefix_metadata_member)*;
type_declaration: (isSufficient = KEYWORD_ALL)? identification (ownedRelationship += owned_multiplicity)? (specialization_part | conjugation_part)+ type_relation_part*;
specialization_part: SPECIALIZES ownedRelationship += owned_specialization (SYMBOL_COMMA ownedRelationship+=owned_specialization);
conjungation_part: CONJUNGATES ownedRelationship += owned_conjungation;
type_relationship_part: disjoining_part | unioning_part | intersecting_part | differencing_part;
disjoining_part: KEYWORD_DISJOINT KEYWORD_FROM ownedRelationship += owned_disjoining (SYMBOL_COMMA ownedRelationship += owned_disjoining);
unioning_part: KEYWORD_UNIONS ownedRelationship += unioning (SYMBOL_COMMA ownedRelationship += unioning)*;
intersecting_part: KEYWORD_INTERSECTS ownedRelationship += intersecting (SYMBOL_COMMA ownedRelationship += intersecting)*;
differencing_part: KEYWORD_DIFFERENCES ownedRelationship += differencing (SYMBOL_COMMA ownedRelationship += differencing)*;
type_body: SYMBOL_STATEMENT_DELIMITER | SYMBOL_CURLY_BRACKET_OPEN type_body_element* SYMBOL_CURLY_BRACKET_CLOSE;
type_body_element: ownedRelationship += non_feature_element | ownedRelationship += feature_element | ownedRelationship += alias_member | ownedRelationship += namespace_import;

specialization: (KEYWORD_SPECIALIZATION identification)? KEYWORD_SUBTYPE specific_type SPECIALIZES general_type relationshipBody;
owned_specialization: general_type;
specific_type: specific = qualifiedName | specific += owned_feature_chain {ownedRelatedElement += specific};
general_type: general = qualifiedName | general += owned_feature_chain {ownedRelatedElement += general};

conjunction: (KEYWORD_CONJUGATION identification)? KEYWORD_CONJUGATE (conjugatedType = qualifiedName | conjugatedType = feature_chain {ownedRelatedElment += conjugatedType}) CONJUNGATES (originalType = qualifiedName | originalType = feature_chain {ownedRelatedElement += orginalType}) relationshipBody;
owned_conjungation: originalType = qualifiedName | originalType = feature_chain {ownedRelatedElement += orginalType};

disjoining: (KEYWORD_DISJOINING identification)? KEYWORD_DISJOINT (typeDisjoint = qualifiedName | typeDisjoint = feature_chain {ownedRelatedElement += typeDisjoint}) KEYWORD_FROM (disjoiningType = qualifiedName | disjoiningType = feature_chain {onwedRelatedElment += disjoiningType}) relationshipBody;
owned_disjoining: disjoining_type = qualifiedName | disjoiningType = feature_chain {onwedRelatedElment += disjoiningType};

unioning: uninionType = qualifiedName | ownedRelatedElement += owned_feature_chain;
intersecting: intersectionType = qualifiedName | ownedRelatedElement += owned_feature_chain;
differencing: differencingType = qualifiedName | ownedRelatedElement += owned_feature_chain;

feature_member: type_feature_member | owned_feature_member;
type_feature_member: member_prefix KEYWORD_MEMBER ownedRelationship += feature_element;
owned_feature_member: member_prefix ownedRelatedElement += feature_element;

classifier: type_prefix KEYWORD_CLASSIFIER classifier_declaration type_body;
classifier_declaration: (isSufficient = KEYWORD_ALL)? identification (ownedRelationship += owned_multiplicity)? (superclassing_part | conjunction_part)? type_relationship_part*;

TYPED_BY: SYMBOL_TYPED_BY | KEYWORD_TYPED KEYWORD_BY;
SPECIALIZES: SYMBOL_SPECIALIZES | KEYWORD_SPECIALIZES;
SUBSETS:  SYMBOL_SPECIALIZES | KEYWORD_SUBSETS;
REFERENCES: SYMBOL_REFERENCES | KEYWORD_REFERENCES;
REDEFINES: SYMBOL_REDEFINES | KEYWORD_REDEFINES;
CONJUNGATES: SYMBOL_CONJUNGATES | KEYWORD_CONJUGATES;

value: NAME | INT | NUMBER+('.'NUMBER+)?;

// Keywords
KEYWORD_ABOUT: 'about';
KEYWORD_ABSTRACT: 'abstract';
KEYWORD_ALIAS: 'alias';
KEYWORD_ALL: 'all';
KEYWORD_AND: 'and';
KEYWORD_AS: 'as';
KEYWORD_ASSOC: 'assoc';
KEYWORD_BEHAVIOR: 'behavior';
KEYWORD_BINDING: 'binding';
KEYWORD_BOOL: 'bool';
KEYWORD_BY: 'by';
KEYWORD_CHAINS: 'chains';
KEYWORD_CLASS: 'class';
KEYWORD_CLASSIFIER: 'classifier';
KEYWORD_COMMENT: 'comment';
KEYWORD_COMPOSITE: 'composite';
KEYWORD_CONJUGATE: 'conjugate';
KEYWORD_CONJUGATES: 'conjugates';
KEYWORD_CONJUGATION: 'conjugation';
KEYWORD_CONNECTOR: 'connector';
KEYWORD_DATATYPE: 'datatype';
KEYWORD_DEFAULT: 'default';
KEYWORD_DEPENDENCY: 'dependency';
KEYWORD_DERIVED: 'derived';
KEYWORD_DIFFERENCES: 'differences';
KEYWORD_DISJOINING: 'disjoining';
KEYWORD_DISJOINT: 'disjoint';
KEYWORD_DOC: 'doc';
KEYWORD_ELSE: 'else';
KEYWORD_END: 'end';
KEYWORD_EXPR: 'expr';
KEYWORD_FALSE: 'false';
KEYWORD_FEATURE: 'feature';
KEYWORD_FEATURED: 'featured';
KEYWORD_FEATURING: 'featuring';
KEYWORD_FILTER: 'filter';
KEYWORD_FIRST: 'first';
KEYWORD_FLOW: 'flow';
KEYWORD_FOR: 'for';
KEYWORD_FROM: 'from';
KEYWORD_FUNCTION: 'function';
KEYWORD_HASTYPE: 'hastype';
KEYWORD_IF: 'if';
KEYWORD_INTERSECTS:'intersects';
KEYWORD_IMPLIES: 'implies';
KEYWORD_IMPORT: 'import';
KEYWORD_IN: 'in';
KEYWORD_INOUT: 'inout';
KEYWORD_INTERACTION: 'interaction';
KEYWORD_INV: 'inv';
KEYWORD_INVERSE: 'inverse';
KEYWORD_INVERTING: 'inverting';
KEYWORD_ISTYPE: 'istype';
KEYWORD_LANGUAGE: 'language';
KEYWORD_MEMBER: 'member';
KEYWORD_METACLASS: 'metaclass';
KEYWORD_METADATA: 'metadata';
KEYWORD_MULTIPLICITY: 'multiplicity';
KEYWORD_NAMESPACE: 'namespace';
KEYWORD_NONUNIQUE: 'nonunique';
KEYWORD_NOT: 'not';
KEYWORD_NULL: 'null';
KEYWORD_OF: 'of';
KEYWORD_OR: 'or';
KEYWORD_ORDERED: 'ordered';
KEYWORD_OUT: 'out';
KEYWORD_PACKAGE: 'package';
KEYWORD_PORTION: 'potion';
KEYWORD_PREDICATE: 'predicate';
KEYWORD_PRIVATE: 'private';
KEYWORD_PROTECTED: 'protected';
KEYWORD_PUBLIC: 'public';
KEYWORD_READONLY: 'readonly';
KEYWORD_REDEFINES: 'redefines';
KEYWORD_REDEFINITION: 'redefinition';
KEYWORD_REFERENCES: 'references';
KEYWORD_REP: 'rep';
KEYWORD_RETURN: 'return';
KEYWORD_SPECILIZATION: 'specialization';
KEYWORD_SPECIALIZES: 'specializes';
KEYWORD_STEP: 'step';
KEYWORD_STRUCT: 'struct';
KEYWORD_SUBCLASSIFIER: 'subsclassifier';
KEYWORD_SUBSET: 'subset';
KEYWORD_SUBSETS: 'subsets';
KEYWORD_SUBTYPE: 'subtype';
KEYWORD_SUCCSESSION: 'succession';
KEYWORD_THEN: 'then';
KEYWORD_TO: 'to';
KEYWORD_TRUE: 'true';
KEYWORD_TYPE: 'type';
KEYWORD_TYPED: 'typed';
KEYWORD_TYPING: 'typing';
KEYWORD_UNIONS: 'unions';
KEYWORD_XOR: 'xor';



//Notes and Comments
SINGLE_LINE_NOTE: '//'  ~( '\r' | '\n' )*;
MULTI_LINE_NOTE: SYMBOL_NOTE_BLOCK_START .*? SYMBOL_COMMENT_BLOCK_END;
REGULAR_COMMENT: SYMBOL_COMMENT_BLOCK_START .*? SYMBOL_COMMENT_BLOCK_END;

//Symbols
SYMBOL_COMMENT_BLOCK_START: '/*';
SYMBOL_NOTE_BLOCK_START: '//*';
SYMBOL_COMMENT_BLOCK_END: '*/';
SYMBOL_STATEMENT_DELIMITER: ';';
SYMBOL_STAR:'*';
SYMBOL_NAMESPACE_SUBSET: '::';
SYMBOL_TYPED_BY: ':';
SYMBOL_SPECIALIZES: ':>';
SYMBOL_REFERENCES: '::>';
SYMBOL_REDEFINES: ':>>';
SYMBOL_CONJUNGATES: '~';
SYMBOL_ROUND_BRACKET_OPEN: '(';
SYMBOL_ROUND_BRACKET_CLOSE: ')';
SYMBOL_CURLY_BRACKET_OPEN: '{';
SYMBOL_CURLY_BRACKET_CLOSE: '}';
SYMBOL_SQUARE_BRACKET_OPEN: '[';
SYMBOL_SQUARE_BRACKET_CLOSE: ']';
SYMBOL_COMMA: ',';
SYMBOL_AT: '@';
SYMBOL_HASHTAG: '#';
SYMBOL_MOD: '%';
SYMBOL_AND: '&';
SYMBOL_UPPER: '^';
SYMBOL_VERTICAL_LINE: '|';
SYMBOL_DOUBLE_STAR: '**';
SYMBOL_PLUS: '+';
SYMBOL_MINUS: '-';
SYMBOL_SLASH: '/';
SYMBOL_ARROOW: '->';
SYMBOL_DOT: '.';
SYMBOL_DDOT: '..';
SYMBOL_SMALLER: '<';
SYMBOL_SMALLER_EQUAL: '<=';
SYMBOL_ASSIGN: '=';
SYMBOL_DEF_ASSIGN: ':=';
SYMBOL_EQUALS: '==';
SYMBOL_IFF_EQUALS: '===';
SYMBOL_NOT_EQUALS: '!=';
SYMBOL_IFF_NOT_EQUALS: '!==';
SYMBOL_GREATER: '>';
SYMBOL_GREATER_EQUALS: '>=';
SYMBOL_QUESTION: '?';
SYMBOL_DQUESTION: '??';
SYMBOL_DOT_QUESTION: '.?';

NAME: BASIC_NAME | UNRESTRICTED_NAME;
BASIC_NAME: ('_'|[a-z]|[A-Z]|[0-9])+ ;
UNRESTRICTED_NAME: '\'' .*? '\'';

//Values:
DECIMAL_VALUE: [0-9]+ ;
EXPONENTIAL_VALUE: DECIMAL_VALUE ('e' | 'E') ('+'|'-') DECIMAL_VALUE;
STRING_VALUE: '"' .*? '"';

WS: [ \t\r\n]+ -> skip ;