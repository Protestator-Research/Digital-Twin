grammar KerML;

start:  namespace |
        classifier |
        feature |
        intenalCodeComment |
        dependency |
        comment |
        documentation |
        visibility |
        class |
        datatype |
        alias |
        importRule |
        type |
        abstract |
        start start;

intenalCodeComment: Commet commentText | CommentStart commentText CommentEnd;
comment: CommentWord NAME ('about' NAME)? CommentStart commentText CommentEnd;
feature: Feature NAME instanceNumbers? (specialization)? delimiter_rule;
classifier: Classifier NAME delimiter_rule;
namespace: Namespace NAME delimiter_rule;
dependency: Dependency (NAME | '\''NAME+'\'') From (NAME | '\''NAME+'\'') To (NAME | '\''NAME+'\'') DELIMITER;
documentation: DocumentationKeyword NAME? CommentStart commentText CommentEnd;
visibility: Private start | Public start | PROTECTED start;
class: Class NAME instanceNumbers? (specialization)? delimiter_rule;
datatype: Datatype NAME (specialization)? delimiter_rule;
specialization: ':' importAddress(',' importAddress)* | SPECIALIZES_KEYWORD importAddress (',' importAddress)*;
alias: ALIAS ('<' NAME '>')? NAME FOR NAME delimiter_rule;
package: PACKAGE NAME delimiter_rule;
importRule: IMPORT importAddress DELIMITER;
importAddress: NAME(NAMESPACE_DELIMITER(NAME|STAR))*(NAMESPACE_DELIMITER STAR STAR)?;
bracketed_content: '{' start '}';
delimiter_rule: (bracketed_content|DELIMITER);
abstract: 'abstract' start;
type: TYPE_KEYWORD NAME instanceNumbers? specialization delimiter_rule;
subtype: SUBTYPE_KEYWORD NAME specialization delimiter_rule;
instanceNumbers: '['NUMBER']';
commentText: (NAME|To|From|Classifier|DELIMITER|Feature|Namespace|CommentWord|Dependency|DocumentationKeyword|Private|Public|FOR|Class|Datatype|Datatype|ALIAS|PACKAGE|STAR|'.')*;



// Keywords
Classifier: 'classifier';
DELIMITER: ';';
Feature: 'feature';
Namespace: 'namespace';
Commet: '//';
CommentStart: '/*';
CommentEnd: '*/';
CommentWord: 'comment';
Dependency: 'dependency';
DocumentationKeyword: 'doc';
Private: 'private';
PROTECTED: 'protected';
Public: 'public';
From: 'from';
FOR: 'for';
To: 'to';
Class: 'class';
Datatype: 'datatype';
ALIAS: 'alias';
PACKAGE: 'package';
IMPORT: 'import';
STAR:'*';
NAMESPACE_DELIMITER: '::';
SPECIALIZES_KEYWORD: 'specializes';
TYPE_KEYWORD: 'type';
SUBTYPE_KEYWORD: 'subtype';

NUMBER: [0-9]+;
NAME: ([a-z]|[A-Z]|[0-9])+ ;
INT     : [0-9]+ ;
WS : [ \t\r\n]+ -> skip ;