
// Generated from KerML.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"




class  KerMLLexer : public antlr4::Lexer {
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

