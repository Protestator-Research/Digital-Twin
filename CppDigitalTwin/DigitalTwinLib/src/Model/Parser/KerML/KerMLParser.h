
// Generated from KerML.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  KerMLParser : public antlr4::Parser {
public:
  enum {
    T__0 = 1, T__1 = 2, T__2 = 3, T__3 = 4, T__4 = 5, T__5 = 6, T__6 = 7, 
    T__7 = 8, T__8 = 9, T__9 = 10, T__10 = 11, T__11 = 12, Classifier = 13, 
    DELIMITER = 14, Feature = 15, Namespace = 16, COMMENT_START = 17, COMMENT_END = 18, 
    COMMENT_WORD = 19, Dependency = 20, DocumentationKeyword = 21, Private = 22, 
    PROTECTED = 23, Public = 24, From = 25, FOR = 26, To = 27, Class = 28, 
    Datatype = 29, ALIAS = 30, PACKAGE = 31, IMPORT = 32, STAR = 33, NAMESPACE_DELIMITER = 34, 
    SPECIALIZES_KEYWORD = 35, TYPE_KEYWORD = 36, SUBTYPE_KEYWORD = 37, LINE_COMMENT = 38, 
    NUMBER = 39, NAME = 40, INT = 41, WS = 42
  };

  enum {
    RuleStart = 0, RuleStartRule = 1, RuleElements = 2, RuleElement = 3, 
    RuleComment = 4, RuleFeature = 5, RuleClassifier = 6, RuleNamespace = 7, 
    RuleDependency = 8, RuleDocumentation = 9, RuleVisibility = 10, RuleClass = 11, 
    RuleDatatype = 12, RuleSpecialization = 13, RuleAlias = 14, RulePackage = 15, 
    RuleImportRule = 16, RuleImportAddress = 17, RuleBracketed_content = 18, 
    RuleDelimiter_rule = 19, RuleAbstract = 20, RuleType = 21, RuleSubtype = 22, 
    RuleInstanceNumbers = 23, RuleCommentText = 24
  };

  explicit KerMLParser(antlr4::TokenStream *input);

  KerMLParser(antlr4::TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options);

  ~KerMLParser() override;

  std::string getGrammarFileName() const override;

  const antlr4::atn::ATN& getATN() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;


  class StartContext;
  class StartRuleContext;
  class ElementsContext;
  class ElementContext;
  class CommentContext;
  class FeatureContext;
  class ClassifierContext;
  class NamespaceContext;
  class DependencyContext;
  class DocumentationContext;
  class VisibilityContext;
  class ClassContext;
  class DatatypeContext;
  class SpecializationContext;
  class AliasContext;
  class PackageContext;
  class ImportRuleContext;
  class ImportAddressContext;
  class Bracketed_contentContext;
  class Delimiter_ruleContext;
  class AbstractContext;
  class TypeContext;
  class SubtypeContext;
  class InstanceNumbersContext;
  class CommentTextContext; 

  class  StartContext : public antlr4::ParserRuleContext {
  public:
    StartContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ElementsContext *elements();
    antlr4::tree::TerminalNode *EOF();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  StartContext* start();

  class  StartRuleContext : public antlr4::ParserRuleContext {
  public:
    StartRuleContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    StartContext *start();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  StartRuleContext* startRule();

  class  ElementsContext : public antlr4::ParserRuleContext {
  public:
    ElementsContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ElementContext *> element();
    ElementContext* element(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  ElementsContext* elements();

  class  ElementContext : public antlr4::ParserRuleContext {
  public:
    ElementContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    NamespaceContext *namespace_();
    ClassifierContext *classifier();
    FeatureContext *feature();
    DependencyContext *dependency();
    CommentContext *comment();
    DocumentationContext *documentation();
    VisibilityContext *visibility();
    ClassContext *class_();
    DatatypeContext *datatype();
    AliasContext *alias();
    ImportRuleContext *importRule();
    TypeContext *type();
    AbstractContext *abstract();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  ElementContext* element();

  class  CommentContext : public antlr4::ParserRuleContext {
  public:
    CommentContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *COMMENT_WORD();
    std::vector<antlr4::tree::TerminalNode *> NAME();
    antlr4::tree::TerminalNode* NAME(size_t i);
    antlr4::tree::TerminalNode *COMMENT_START();
    CommentTextContext *commentText();
    antlr4::tree::TerminalNode *COMMENT_END();
    DependencyContext *dependency();
    antlr4::tree::TerminalNode *LINE_COMMENT();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  CommentContext* comment();

  class  FeatureContext : public antlr4::ParserRuleContext {
  public:
    FeatureContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Feature();
    antlr4::tree::TerminalNode *NAME();
    Delimiter_ruleContext *delimiter_rule();
    InstanceNumbersContext *instanceNumbers();
    SpecializationContext *specialization();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  FeatureContext* feature();

  class  ClassifierContext : public antlr4::ParserRuleContext {
  public:
    ClassifierContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Classifier();
    antlr4::tree::TerminalNode *NAME();
    Delimiter_ruleContext *delimiter_rule();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  ClassifierContext* classifier();

  class  NamespaceContext : public antlr4::ParserRuleContext {
  public:
    NamespaceContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Namespace();
    antlr4::tree::TerminalNode *NAME();
    Delimiter_ruleContext *delimiter_rule();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  NamespaceContext* namespace_();

  class  DependencyContext : public antlr4::ParserRuleContext {
  public:
    DependencyContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Dependency();
    antlr4::tree::TerminalNode *From();
    antlr4::tree::TerminalNode *To();
    antlr4::tree::TerminalNode *DELIMITER();
    std::vector<antlr4::tree::TerminalNode *> NAME();
    antlr4::tree::TerminalNode* NAME(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  DependencyContext* dependency();

  class  DocumentationContext : public antlr4::ParserRuleContext {
  public:
    DocumentationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *DocumentationKeyword();
    antlr4::tree::TerminalNode *COMMENT_START();
    CommentTextContext *commentText();
    antlr4::tree::TerminalNode *COMMENT_END();
    antlr4::tree::TerminalNode *NAME();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  DocumentationContext* documentation();

  class  VisibilityContext : public antlr4::ParserRuleContext {
  public:
    VisibilityContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Private();
    ElementContext *element();
    antlr4::tree::TerminalNode *Public();
    antlr4::tree::TerminalNode *PROTECTED();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  VisibilityContext* visibility();

  class  ClassContext : public antlr4::ParserRuleContext {
  public:
    ClassContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Class();
    antlr4::tree::TerminalNode *NAME();
    Delimiter_ruleContext *delimiter_rule();
    InstanceNumbersContext *instanceNumbers();
    SpecializationContext *specialization();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  ClassContext* class_();

  class  DatatypeContext : public antlr4::ParserRuleContext {
  public:
    DatatypeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Datatype();
    antlr4::tree::TerminalNode *NAME();
    Delimiter_ruleContext *delimiter_rule();
    SpecializationContext *specialization();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  DatatypeContext* datatype();

  class  SpecializationContext : public antlr4::ParserRuleContext {
  public:
    SpecializationContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<ImportAddressContext *> importAddress();
    ImportAddressContext* importAddress(size_t i);
    antlr4::tree::TerminalNode *SPECIALIZES_KEYWORD();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  SpecializationContext* specialization();

  class  AliasContext : public antlr4::ParserRuleContext {
  public:
    AliasContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *ALIAS();
    std::vector<antlr4::tree::TerminalNode *> NAME();
    antlr4::tree::TerminalNode* NAME(size_t i);
    antlr4::tree::TerminalNode *FOR();
    Delimiter_ruleContext *delimiter_rule();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  AliasContext* alias();

  class  PackageContext : public antlr4::ParserRuleContext {
  public:
    PackageContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *PACKAGE();
    antlr4::tree::TerminalNode *NAME();
    Delimiter_ruleContext *delimiter_rule();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  PackageContext* package();

  class  ImportRuleContext : public antlr4::ParserRuleContext {
  public:
    ImportRuleContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *IMPORT();
    ImportAddressContext *importAddress();
    antlr4::tree::TerminalNode *DELIMITER();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  ImportRuleContext* importRule();

  class  ImportAddressContext : public antlr4::ParserRuleContext {
  public:
    ImportAddressContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> NAME();
    antlr4::tree::TerminalNode* NAME(size_t i);
    std::vector<antlr4::tree::TerminalNode *> NAMESPACE_DELIMITER();
    antlr4::tree::TerminalNode* NAMESPACE_DELIMITER(size_t i);
    std::vector<antlr4::tree::TerminalNode *> STAR();
    antlr4::tree::TerminalNode* STAR(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  ImportAddressContext* importAddress();

  class  Bracketed_contentContext : public antlr4::ParserRuleContext {
  public:
    Bracketed_contentContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ElementsContext *elements();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Bracketed_contentContext* bracketed_content();

  class  Delimiter_ruleContext : public antlr4::ParserRuleContext {
  public:
    Delimiter_ruleContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    Bracketed_contentContext *bracketed_content();
    antlr4::tree::TerminalNode *DELIMITER();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  Delimiter_ruleContext* delimiter_rule();

  class  AbstractContext : public antlr4::ParserRuleContext {
  public:
    AbstractContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    ElementContext *element();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  AbstractContext* abstract();

  class  TypeContext : public antlr4::ParserRuleContext {
  public:
    TypeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *TYPE_KEYWORD();
    antlr4::tree::TerminalNode *NAME();
    SpecializationContext *specialization();
    Delimiter_ruleContext *delimiter_rule();
    InstanceNumbersContext *instanceNumbers();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  TypeContext* type();

  class  SubtypeContext : public antlr4::ParserRuleContext {
  public:
    SubtypeContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *SUBTYPE_KEYWORD();
    antlr4::tree::TerminalNode *NAME();
    SpecializationContext *specialization();
    Delimiter_ruleContext *delimiter_rule();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  SubtypeContext* subtype();

  class  InstanceNumbersContext : public antlr4::ParserRuleContext {
  public:
    InstanceNumbersContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *NUMBER();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  InstanceNumbersContext* instanceNumbers();

  class  CommentTextContext : public antlr4::ParserRuleContext {
  public:
    CommentTextContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    std::vector<antlr4::tree::TerminalNode *> NAME();
    antlr4::tree::TerminalNode* NAME(size_t i);
    std::vector<antlr4::tree::TerminalNode *> To();
    antlr4::tree::TerminalNode* To(size_t i);
    std::vector<antlr4::tree::TerminalNode *> From();
    antlr4::tree::TerminalNode* From(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Classifier();
    antlr4::tree::TerminalNode* Classifier(size_t i);
    std::vector<antlr4::tree::TerminalNode *> DELIMITER();
    antlr4::tree::TerminalNode* DELIMITER(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Feature();
    antlr4::tree::TerminalNode* Feature(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Namespace();
    antlr4::tree::TerminalNode* Namespace(size_t i);
    std::vector<antlr4::tree::TerminalNode *> COMMENT_WORD();
    antlr4::tree::TerminalNode* COMMENT_WORD(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Dependency();
    antlr4::tree::TerminalNode* Dependency(size_t i);
    std::vector<antlr4::tree::TerminalNode *> DocumentationKeyword();
    antlr4::tree::TerminalNode* DocumentationKeyword(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Private();
    antlr4::tree::TerminalNode* Private(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Public();
    antlr4::tree::TerminalNode* Public(size_t i);
    std::vector<antlr4::tree::TerminalNode *> FOR();
    antlr4::tree::TerminalNode* FOR(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Class();
    antlr4::tree::TerminalNode* Class(size_t i);
    std::vector<antlr4::tree::TerminalNode *> Datatype();
    antlr4::tree::TerminalNode* Datatype(size_t i);
    std::vector<antlr4::tree::TerminalNode *> ALIAS();
    antlr4::tree::TerminalNode* ALIAS(size_t i);
    std::vector<antlr4::tree::TerminalNode *> PACKAGE();
    antlr4::tree::TerminalNode* PACKAGE(size_t i);
    std::vector<antlr4::tree::TerminalNode *> STAR();
    antlr4::tree::TerminalNode* STAR(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  CommentTextContext* commentText();


  // By default the static state used to implement the parser is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:
};

