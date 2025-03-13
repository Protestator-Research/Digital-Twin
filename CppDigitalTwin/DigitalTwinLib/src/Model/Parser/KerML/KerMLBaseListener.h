
// Generated from KerML.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "KerMLListener.h"


/**
 * This class provides an empty implementation of KerMLListener,
 * which can be extended to create a listener which only needs to handle a subset
 * of the available methods.
 */
class  KerMLBaseListener : public KerMLListener {
public:

  virtual void enterStart(KerMLParser::StartContext * /*ctx*/) override { }
  virtual void exitStart(KerMLParser::StartContext * /*ctx*/) override { }

  virtual void enterStartRule(KerMLParser::StartRuleContext * /*ctx*/) override { }
  virtual void exitStartRule(KerMLParser::StartRuleContext * /*ctx*/) override { }

  virtual void enterElements(KerMLParser::ElementsContext * /*ctx*/) override { }
  virtual void exitElements(KerMLParser::ElementsContext * /*ctx*/) override { }

  virtual void enterElement(KerMLParser::ElementContext * /*ctx*/) override { }
  virtual void exitElement(KerMLParser::ElementContext * /*ctx*/) override { }

  virtual void enterComment(KerMLParser::CommentContext * /*ctx*/) override { }
  virtual void exitComment(KerMLParser::CommentContext * /*ctx*/) override { }

  virtual void enterFeature(KerMLParser::FeatureContext * /*ctx*/) override { }
  virtual void exitFeature(KerMLParser::FeatureContext * /*ctx*/) override { }

  virtual void enterClassifier(KerMLParser::ClassifierContext * /*ctx*/) override { }
  virtual void exitClassifier(KerMLParser::ClassifierContext * /*ctx*/) override { }

  virtual void enterNamespace(KerMLParser::NamespaceContext * /*ctx*/) override { }
  virtual void exitNamespace(KerMLParser::NamespaceContext * /*ctx*/) override { }

  virtual void enterDependency(KerMLParser::DependencyContext * /*ctx*/) override { }
  virtual void exitDependency(KerMLParser::DependencyContext * /*ctx*/) override { }

  virtual void enterDocumentation(KerMLParser::DocumentationContext * /*ctx*/) override { }
  virtual void exitDocumentation(KerMLParser::DocumentationContext * /*ctx*/) override { }

  virtual void enterVisibility(KerMLParser::VisibilityContext * /*ctx*/) override { }
  virtual void exitVisibility(KerMLParser::VisibilityContext * /*ctx*/) override { }

  virtual void enterClass(KerMLParser::ClassContext * /*ctx*/) override { }
  virtual void exitClass(KerMLParser::ClassContext * /*ctx*/) override { }

  virtual void enterDatatype(KerMLParser::DatatypeContext * /*ctx*/) override { }
  virtual void exitDatatype(KerMLParser::DatatypeContext * /*ctx*/) override { }

  virtual void enterSpecialization(KerMLParser::SpecializationContext * /*ctx*/) override { }
  virtual void exitSpecialization(KerMLParser::SpecializationContext * /*ctx*/) override { }

  virtual void enterAlias(KerMLParser::AliasContext * /*ctx*/) override { }
  virtual void exitAlias(KerMLParser::AliasContext * /*ctx*/) override { }

  virtual void enterPackage(KerMLParser::PackageContext * /*ctx*/) override { }
  virtual void exitPackage(KerMLParser::PackageContext * /*ctx*/) override { }

  virtual void enterImportRule(KerMLParser::ImportRuleContext * /*ctx*/) override { }
  virtual void exitImportRule(KerMLParser::ImportRuleContext * /*ctx*/) override { }

  virtual void enterImportAddress(KerMLParser::ImportAddressContext * /*ctx*/) override { }
  virtual void exitImportAddress(KerMLParser::ImportAddressContext * /*ctx*/) override { }

  virtual void enterBracketed_content(KerMLParser::Bracketed_contentContext * /*ctx*/) override { }
  virtual void exitBracketed_content(KerMLParser::Bracketed_contentContext * /*ctx*/) override { }

  virtual void enterDelimiter_rule(KerMLParser::Delimiter_ruleContext * /*ctx*/) override { }
  virtual void exitDelimiter_rule(KerMLParser::Delimiter_ruleContext * /*ctx*/) override { }

  virtual void enterAbstract(KerMLParser::AbstractContext * /*ctx*/) override { }
  virtual void exitAbstract(KerMLParser::AbstractContext * /*ctx*/) override { }

  virtual void enterType(KerMLParser::TypeContext * /*ctx*/) override { }
  virtual void exitType(KerMLParser::TypeContext * /*ctx*/) override { }

  virtual void enterSubtype(KerMLParser::SubtypeContext * /*ctx*/) override { }
  virtual void exitSubtype(KerMLParser::SubtypeContext * /*ctx*/) override { }

  virtual void enterInstanceNumbers(KerMLParser::InstanceNumbersContext * /*ctx*/) override { }
  virtual void exitInstanceNumbers(KerMLParser::InstanceNumbersContext * /*ctx*/) override { }

  virtual void enterCommentText(KerMLParser::CommentTextContext * /*ctx*/) override { }
  virtual void exitCommentText(KerMLParser::CommentTextContext * /*ctx*/) override { }


  virtual void enterEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void exitEveryRule(antlr4::ParserRuleContext * /*ctx*/) override { }
  virtual void visitTerminal(antlr4::tree::TerminalNode * /*node*/) override { }
  virtual void visitErrorNode(antlr4::tree::ErrorNode * /*node*/) override { }

};

