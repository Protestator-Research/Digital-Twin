
// Generated from KerML.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  KerMLParser : public antlr4::Parser {
public:
  enum {
    T__0 = 1, T__1 = 2, T__2 = 3, T__3 = 4, T__4 = 5, T__5 = 6, T__6 = 7, 
    T__7 = 8, T__8 = 9, T__9 = 10, T__10 = 11, T__11 = 12, Classifier = 13, 
    DELIMITER = 14, Feature = 15, Namespace = 16, Commet = 17, CommentStart = 18, 
    CommentEnd = 19, CommentWord = 20, Dependency = 21, DocumentationKeyword = 22, 
    Private = 23, PROTECTED = 24, Public = 25, From = 26, FOR = 27, To = 28, 
    Class = 29, Datatype = 30, ALIAS = 31, PACKAGE = 32, IMPORT = 33, STAR = 34, 
    NAMESPACE_DELIMITER = 35, SPECIALIZES_KEYWORD = 36, TYPE_KEYWORD = 37, 
    SUBTYPE_KEYWORD = 38, NUMBER = 39, NAME = 40, INT = 41, WS = 42
  };

  enum {
    RuleStart = 0, RuleIntenalCodeComment = 1, RuleComment = 2, RuleFeature = 3, 
    RuleClassifier = 4, RuleNamespace = 5, RuleDependency = 6, RuleDocumentation = 7, 
    RuleVisibility = 8, RuleClass = 9, RuleDatatype = 10, RuleSpecialization = 11, 
    RuleAlias = 12, RulePackage = 13, RuleImportRule = 14, RuleImportAddress = 15, 
    RuleBracketed_content = 16, RuleDelimiter_rule = 17, RuleAbstract = 18, 
    RuleType = 19, RuleSubtype = 20, RuleInstanceNumbers = 21, RuleCommentText = 22
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
  class IntenalCodeCommentContext;
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
    NamespaceContext *namespace_();
    ClassifierContext *classifier();
    FeatureContext *feature();
    IntenalCodeCommentContext *intenalCodeComment();
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
    std::vector<StartContext *> start();
    StartContext* start(size_t i);

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  StartContext* start();
  StartContext* start(int precedence);
  class  IntenalCodeCommentContext : public antlr4::ParserRuleContext {
  public:
    IntenalCodeCommentContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *Commet();
    CommentTextContext *commentText();
    antlr4::tree::TerminalNode *CommentStart();
    antlr4::tree::TerminalNode *CommentEnd();

    virtual void enterRule(antlr4::tree::ParseTreeListener *listener) override;
    virtual void exitRule(antlr4::tree::ParseTreeListener *listener) override;
   
  };

  IntenalCodeCommentContext* intenalCodeComment();

  class  CommentContext : public antlr4::ParserRuleContext {
  public:
    CommentContext(antlr4::ParserRuleContext *parent, size_t invokingState);
    virtual size_t getRuleIndex() const override;
    antlr4::tree::TerminalNode *CommentWord();
    std::vector<antlr4::tree::TerminalNode *> NAME();
    antlr4::tree::TerminalNode* NAME(size_t i);
    antlr4::tree::TerminalNode *CommentStart();
    CommentTextContext *commentText();
    antlr4::tree::TerminalNode *CommentEnd();

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
    antlr4::tree::TerminalNode *CommentStart();
    CommentTextContext *commentText();
    antlr4::tree::TerminalNode *CommentEnd();
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
    StartContext *start();
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
    StartContext *start();

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
    StartContext *start();

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
    std::vector<antlr4::tree::TerminalNode *> CommentWord();
    antlr4::tree::TerminalNode* CommentWord(size_t i);
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


  bool sempred(antlr4::RuleContext *_localctx, size_t ruleIndex, size_t predicateIndex) override;

  bool startSempred(StartContext *_localctx, size_t predicateIndex);

  // By default the static state used to implement the parser is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:
};

