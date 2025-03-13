
// Generated from KerML.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "KerMLParser.h"


/**
 * This interface defines an abstract listener for a parse tree produced by KerMLParser.
 */
class  KerMLListener : public antlr4::tree::ParseTreeListener {
public:

  virtual void enterStart(KerMLParser::StartContext *ctx) = 0;
  virtual void exitStart(KerMLParser::StartContext *ctx) = 0;

  virtual void enterStartRule(KerMLParser::StartRuleContext *ctx) = 0;
  virtual void exitStartRule(KerMLParser::StartRuleContext *ctx) = 0;

  virtual void enterElements(KerMLParser::ElementsContext *ctx) = 0;
  virtual void exitElements(KerMLParser::ElementsContext *ctx) = 0;

  virtual void enterElement(KerMLParser::ElementContext *ctx) = 0;
  virtual void exitElement(KerMLParser::ElementContext *ctx) = 0;

  virtual void enterComment(KerMLParser::CommentContext *ctx) = 0;
  virtual void exitComment(KerMLParser::CommentContext *ctx) = 0;

  virtual void enterFeature(KerMLParser::FeatureContext *ctx) = 0;
  virtual void exitFeature(KerMLParser::FeatureContext *ctx) = 0;

  virtual void enterClassifier(KerMLParser::ClassifierContext *ctx) = 0;
  virtual void exitClassifier(KerMLParser::ClassifierContext *ctx) = 0;

  virtual void enterNamespace(KerMLParser::NamespaceContext *ctx) = 0;
  virtual void exitNamespace(KerMLParser::NamespaceContext *ctx) = 0;

  virtual void enterDependency(KerMLParser::DependencyContext *ctx) = 0;
  virtual void exitDependency(KerMLParser::DependencyContext *ctx) = 0;

  virtual void enterDocumentation(KerMLParser::DocumentationContext *ctx) = 0;
  virtual void exitDocumentation(KerMLParser::DocumentationContext *ctx) = 0;

  virtual void enterVisibility(KerMLParser::VisibilityContext *ctx) = 0;
  virtual void exitVisibility(KerMLParser::VisibilityContext *ctx) = 0;

  virtual void enterClass(KerMLParser::ClassContext *ctx) = 0;
  virtual void exitClass(KerMLParser::ClassContext *ctx) = 0;

  virtual void enterDatatype(KerMLParser::DatatypeContext *ctx) = 0;
  virtual void exitDatatype(KerMLParser::DatatypeContext *ctx) = 0;

  virtual void enterSpecialization(KerMLParser::SpecializationContext *ctx) = 0;
  virtual void exitSpecialization(KerMLParser::SpecializationContext *ctx) = 0;

  virtual void enterAlias(KerMLParser::AliasContext *ctx) = 0;
  virtual void exitAlias(KerMLParser::AliasContext *ctx) = 0;

  virtual void enterPackage(KerMLParser::PackageContext *ctx) = 0;
  virtual void exitPackage(KerMLParser::PackageContext *ctx) = 0;

  virtual void enterImportRule(KerMLParser::ImportRuleContext *ctx) = 0;
  virtual void exitImportRule(KerMLParser::ImportRuleContext *ctx) = 0;

  virtual void enterImportAddress(KerMLParser::ImportAddressContext *ctx) = 0;
  virtual void exitImportAddress(KerMLParser::ImportAddressContext *ctx) = 0;

  virtual void enterBracketed_content(KerMLParser::Bracketed_contentContext *ctx) = 0;
  virtual void exitBracketed_content(KerMLParser::Bracketed_contentContext *ctx) = 0;

  virtual void enterDelimiter_rule(KerMLParser::Delimiter_ruleContext *ctx) = 0;
  virtual void exitDelimiter_rule(KerMLParser::Delimiter_ruleContext *ctx) = 0;

  virtual void enterAbstract(KerMLParser::AbstractContext *ctx) = 0;
  virtual void exitAbstract(KerMLParser::AbstractContext *ctx) = 0;

  virtual void enterType(KerMLParser::TypeContext *ctx) = 0;
  virtual void exitType(KerMLParser::TypeContext *ctx) = 0;

  virtual void enterSubtype(KerMLParser::SubtypeContext *ctx) = 0;
  virtual void exitSubtype(KerMLParser::SubtypeContext *ctx) = 0;

  virtual void enterInstanceNumbers(KerMLParser::InstanceNumbersContext *ctx) = 0;
  virtual void exitInstanceNumbers(KerMLParser::InstanceNumbersContext *ctx) = 0;

  virtual void enterCommentText(KerMLParser::CommentTextContext *ctx) = 0;
  virtual void exitCommentText(KerMLParser::CommentTextContext *ctx) = 0;


};

