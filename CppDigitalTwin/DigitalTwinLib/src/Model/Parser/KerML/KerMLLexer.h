
// Generated from KerML.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  KerMLLexer : public antlr4::Lexer {
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

  explicit KerMLLexer(antlr4::CharStream *input);

  ~KerMLLexer() override;


  std::string getGrammarFileName() const override;

  const std::vector<std::string>& getRuleNames() const override;

  const std::vector<std::string>& getChannelNames() const override;

  const std::vector<std::string>& getModeNames() const override;

  const antlr4::dfa::Vocabulary& getVocabulary() const override;

  antlr4::atn::SerializedATNView getSerializedATN() const override;

  const antlr4::atn::ATN& getATN() const override;

  // By default the static state used to implement the lexer is lazily initialized during the first
  // call to the constructor. You can call this function if you wish to initialize the static state
  // ahead of time.
  static void initialize();

private:

  // Individual action functions triggered by action() above.

  // Individual semantic predicate functions triggered by sempred() above.

};

