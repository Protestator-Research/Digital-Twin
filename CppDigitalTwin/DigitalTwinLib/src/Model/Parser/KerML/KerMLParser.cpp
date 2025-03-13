
// Generated from KerML.g4 by ANTLR 4.13.2


#include "KerMLListener.h"

#include "KerMLParser.h"


using namespace antlrcpp;

using namespace antlr4;

namespace {

struct KerMLParserStaticData final {
  KerMLParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  KerMLParserStaticData(const KerMLParserStaticData&) = delete;
  KerMLParserStaticData(KerMLParserStaticData&&) = delete;
  KerMLParserStaticData& operator=(const KerMLParserStaticData&) = delete;
  KerMLParserStaticData& operator=(KerMLParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag kermlParserOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<KerMLParserStaticData> kermlParserStaticData = nullptr;

void kermlParserInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (kermlParserStaticData != nullptr) {
    return;
  }
#else
  assert(kermlParserStaticData == nullptr);
#endif
  auto staticData = std::make_unique<KerMLParserStaticData>(
    std::vector<std::string>{
      "start", "startRule", "elements", "element", "comment", "feature", 
      "classifier", "namespace", "dependency", "documentation", "visibility", 
      "class", "datatype", "specialization", "alias", "package", "importRule", 
      "importAddress", "bracketed_content", "delimiter_rule", "abstract", 
      "type", "subtype", "instanceNumbers", "commentText"
    },
    std::vector<std::string>{
      "", "'about'", "'''", "':'", "','", "'<'", "'>'", "'{'", "'}'", "'abstract'", 
      "'['", "']'", "'.'", "'classifier'", "';'", "'feature'", "'namespace'", 
      "'/*'", "'*/'", "'comment'", "'dependency'", "'doc'", "'private'", 
      "'protected'", "'public'", "'from'", "'for'", "'to'", "'class'", "'datatype'", 
      "'alias'", "'package'", "'import'", "'*'", "'::'", "'specializes'", 
      "'type'", "'subtype'"
    },
    std::vector<std::string>{
      "", "", "", "", "", "", "", "", "", "", "", "", "", "Classifier", 
      "DELIMITER", "Feature", "Namespace", "COMMENT_START", "COMMENT_END", 
      "COMMENT_WORD", "Dependency", "DocumentationKeyword", "Private", "PROTECTED", 
      "Public", "From", "FOR", "To", "Class", "Datatype", "ALIAS", "PACKAGE", 
      "IMPORT", "STAR", "NAMESPACE_DELIMITER", "SPECIALIZES_KEYWORD", "TYPE_KEYWORD", 
      "SUBTYPE_KEYWORD", "LINE_COMMENT", "NUMBER", "NAME", "INT", "WS"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,42,263,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
  	14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,7,
  	21,2,22,7,22,2,23,7,23,2,24,7,24,1,0,1,0,1,0,1,1,1,1,1,2,5,2,57,8,2,10,
  	2,12,2,60,9,2,1,3,1,3,1,3,1,3,1,3,1,3,1,3,1,3,1,3,1,3,1,3,1,3,1,3,3,3,
  	75,8,3,1,4,1,4,1,4,1,4,3,4,81,8,4,1,4,1,4,1,4,1,4,1,4,1,4,3,4,89,8,4,
  	1,5,1,5,1,5,3,5,94,8,5,1,5,3,5,97,8,5,1,5,1,5,1,6,1,6,1,6,1,6,1,7,1,7,
  	1,7,1,7,1,8,1,8,1,8,1,8,4,8,113,8,8,11,8,12,8,114,1,8,3,8,118,8,8,1,8,
  	1,8,1,8,1,8,4,8,124,8,8,11,8,12,8,125,1,8,3,8,129,8,8,1,8,1,8,1,8,1,8,
  	4,8,135,8,8,11,8,12,8,136,1,8,3,8,140,8,8,1,8,1,8,1,9,1,9,3,9,146,8,9,
  	1,9,1,9,1,9,1,9,1,10,1,10,1,10,1,10,1,10,1,10,3,10,158,8,10,1,11,1,11,
  	1,11,3,11,163,8,11,1,11,3,11,166,8,11,1,11,1,11,1,12,1,12,1,12,3,12,173,
  	8,12,1,12,1,12,1,13,1,13,1,13,1,13,5,13,181,8,13,10,13,12,13,184,9,13,
  	1,13,1,13,1,13,1,13,5,13,190,8,13,10,13,12,13,193,9,13,3,13,195,8,13,
  	1,14,1,14,1,14,1,14,3,14,201,8,14,1,14,1,14,1,14,1,14,1,14,1,15,1,15,
  	1,15,1,15,1,16,1,16,1,16,1,16,1,17,1,17,1,17,5,17,219,8,17,10,17,12,17,
  	222,9,17,1,17,1,17,1,17,3,17,227,8,17,1,18,1,18,1,18,1,18,1,19,1,19,3,
  	19,235,8,19,1,20,1,20,1,20,1,21,1,21,1,21,3,21,243,8,21,1,21,1,21,1,21,
  	1,22,1,22,1,22,1,22,1,22,1,23,1,23,1,23,1,23,1,24,5,24,258,8,24,10,24,
  	12,24,261,9,24,1,24,0,0,25,0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,
  	32,34,36,38,40,42,44,46,48,0,2,2,0,33,33,40,40,5,0,12,16,19,22,24,31,
  	33,33,40,40,275,0,50,1,0,0,0,2,53,1,0,0,0,4,58,1,0,0,0,6,74,1,0,0,0,8,
  	88,1,0,0,0,10,90,1,0,0,0,12,100,1,0,0,0,14,104,1,0,0,0,16,108,1,0,0,0,
  	18,143,1,0,0,0,20,157,1,0,0,0,22,159,1,0,0,0,24,169,1,0,0,0,26,194,1,
  	0,0,0,28,196,1,0,0,0,30,207,1,0,0,0,32,211,1,0,0,0,34,215,1,0,0,0,36,
  	228,1,0,0,0,38,234,1,0,0,0,40,236,1,0,0,0,42,239,1,0,0,0,44,247,1,0,0,
  	0,46,252,1,0,0,0,48,259,1,0,0,0,50,51,3,4,2,0,51,52,5,0,0,1,52,1,1,0,
  	0,0,53,54,3,0,0,0,54,3,1,0,0,0,55,57,3,6,3,0,56,55,1,0,0,0,57,60,1,0,
  	0,0,58,56,1,0,0,0,58,59,1,0,0,0,59,5,1,0,0,0,60,58,1,0,0,0,61,75,3,14,
  	7,0,62,75,3,12,6,0,63,75,3,10,5,0,64,75,3,16,8,0,65,75,3,8,4,0,66,75,
  	3,18,9,0,67,75,3,20,10,0,68,75,3,22,11,0,69,75,3,24,12,0,70,75,3,28,14,
  	0,71,75,3,32,16,0,72,75,3,42,21,0,73,75,3,40,20,0,74,61,1,0,0,0,74,62,
  	1,0,0,0,74,63,1,0,0,0,74,64,1,0,0,0,74,65,1,0,0,0,74,66,1,0,0,0,74,67,
  	1,0,0,0,74,68,1,0,0,0,74,69,1,0,0,0,74,70,1,0,0,0,74,71,1,0,0,0,74,72,
  	1,0,0,0,74,73,1,0,0,0,75,7,1,0,0,0,76,77,5,19,0,0,77,80,5,40,0,0,78,79,
  	5,1,0,0,79,81,5,40,0,0,80,78,1,0,0,0,80,81,1,0,0,0,81,82,1,0,0,0,82,83,
  	5,17,0,0,83,84,3,48,24,0,84,85,5,18,0,0,85,86,3,16,8,0,86,89,1,0,0,0,
  	87,89,5,38,0,0,88,76,1,0,0,0,88,87,1,0,0,0,89,9,1,0,0,0,90,91,5,15,0,
  	0,91,93,5,40,0,0,92,94,3,46,23,0,93,92,1,0,0,0,93,94,1,0,0,0,94,96,1,
  	0,0,0,95,97,3,26,13,0,96,95,1,0,0,0,96,97,1,0,0,0,97,98,1,0,0,0,98,99,
  	3,38,19,0,99,11,1,0,0,0,100,101,5,13,0,0,101,102,5,40,0,0,102,103,3,38,
  	19,0,103,13,1,0,0,0,104,105,5,16,0,0,105,106,5,40,0,0,106,107,3,38,19,
  	0,107,15,1,0,0,0,108,117,5,20,0,0,109,118,5,40,0,0,110,112,5,2,0,0,111,
  	113,5,40,0,0,112,111,1,0,0,0,113,114,1,0,0,0,114,112,1,0,0,0,114,115,
  	1,0,0,0,115,116,1,0,0,0,116,118,5,2,0,0,117,109,1,0,0,0,117,110,1,0,0,
  	0,118,119,1,0,0,0,119,128,5,25,0,0,120,129,5,40,0,0,121,123,5,2,0,0,122,
  	124,5,40,0,0,123,122,1,0,0,0,124,125,1,0,0,0,125,123,1,0,0,0,125,126,
  	1,0,0,0,126,127,1,0,0,0,127,129,5,2,0,0,128,120,1,0,0,0,128,121,1,0,0,
  	0,129,130,1,0,0,0,130,139,5,27,0,0,131,140,5,40,0,0,132,134,5,2,0,0,133,
  	135,5,40,0,0,134,133,1,0,0,0,135,136,1,0,0,0,136,134,1,0,0,0,136,137,
  	1,0,0,0,137,138,1,0,0,0,138,140,5,2,0,0,139,131,1,0,0,0,139,132,1,0,0,
  	0,140,141,1,0,0,0,141,142,5,14,0,0,142,17,1,0,0,0,143,145,5,21,0,0,144,
  	146,5,40,0,0,145,144,1,0,0,0,145,146,1,0,0,0,146,147,1,0,0,0,147,148,
  	5,17,0,0,148,149,3,48,24,0,149,150,5,18,0,0,150,19,1,0,0,0,151,152,5,
  	22,0,0,152,158,3,6,3,0,153,154,5,24,0,0,154,158,3,6,3,0,155,156,5,23,
  	0,0,156,158,3,6,3,0,157,151,1,0,0,0,157,153,1,0,0,0,157,155,1,0,0,0,158,
  	21,1,0,0,0,159,160,5,28,0,0,160,162,5,40,0,0,161,163,3,46,23,0,162,161,
  	1,0,0,0,162,163,1,0,0,0,163,165,1,0,0,0,164,166,3,26,13,0,165,164,1,0,
  	0,0,165,166,1,0,0,0,166,167,1,0,0,0,167,168,3,38,19,0,168,23,1,0,0,0,
  	169,170,5,29,0,0,170,172,5,40,0,0,171,173,3,26,13,0,172,171,1,0,0,0,172,
  	173,1,0,0,0,173,174,1,0,0,0,174,175,3,38,19,0,175,25,1,0,0,0,176,177,
  	5,3,0,0,177,182,3,34,17,0,178,179,5,4,0,0,179,181,3,34,17,0,180,178,1,
  	0,0,0,181,184,1,0,0,0,182,180,1,0,0,0,182,183,1,0,0,0,183,195,1,0,0,0,
  	184,182,1,0,0,0,185,186,5,35,0,0,186,191,3,34,17,0,187,188,5,4,0,0,188,
  	190,3,34,17,0,189,187,1,0,0,0,190,193,1,0,0,0,191,189,1,0,0,0,191,192,
  	1,0,0,0,192,195,1,0,0,0,193,191,1,0,0,0,194,176,1,0,0,0,194,185,1,0,0,
  	0,195,27,1,0,0,0,196,200,5,30,0,0,197,198,5,5,0,0,198,199,5,40,0,0,199,
  	201,5,6,0,0,200,197,1,0,0,0,200,201,1,0,0,0,201,202,1,0,0,0,202,203,5,
  	40,0,0,203,204,5,26,0,0,204,205,5,40,0,0,205,206,3,38,19,0,206,29,1,0,
  	0,0,207,208,5,31,0,0,208,209,5,40,0,0,209,210,3,38,19,0,210,31,1,0,0,
  	0,211,212,5,32,0,0,212,213,3,34,17,0,213,214,5,14,0,0,214,33,1,0,0,0,
  	215,220,5,40,0,0,216,217,5,34,0,0,217,219,7,0,0,0,218,216,1,0,0,0,219,
  	222,1,0,0,0,220,218,1,0,0,0,220,221,1,0,0,0,221,226,1,0,0,0,222,220,1,
  	0,0,0,223,224,5,34,0,0,224,225,5,33,0,0,225,227,5,33,0,0,226,223,1,0,
  	0,0,226,227,1,0,0,0,227,35,1,0,0,0,228,229,5,7,0,0,229,230,3,4,2,0,230,
  	231,5,8,0,0,231,37,1,0,0,0,232,235,3,36,18,0,233,235,5,14,0,0,234,232,
  	1,0,0,0,234,233,1,0,0,0,235,39,1,0,0,0,236,237,5,9,0,0,237,238,3,6,3,
  	0,238,41,1,0,0,0,239,240,5,36,0,0,240,242,5,40,0,0,241,243,3,46,23,0,
  	242,241,1,0,0,0,242,243,1,0,0,0,243,244,1,0,0,0,244,245,3,26,13,0,245,
  	246,3,38,19,0,246,43,1,0,0,0,247,248,5,37,0,0,248,249,5,40,0,0,249,250,
  	3,26,13,0,250,251,3,38,19,0,251,45,1,0,0,0,252,253,5,10,0,0,253,254,5,
  	39,0,0,254,255,5,11,0,0,255,47,1,0,0,0,256,258,7,1,0,0,257,256,1,0,0,
  	0,258,261,1,0,0,0,259,257,1,0,0,0,259,260,1,0,0,0,260,49,1,0,0,0,261,
  	259,1,0,0,0,26,58,74,80,88,93,96,114,117,125,128,136,139,145,157,162,
  	165,172,182,191,194,200,220,226,234,242,259
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  kermlParserStaticData = std::move(staticData);
}

}

KerMLParser::KerMLParser(TokenStream *input) : KerMLParser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

KerMLParser::KerMLParser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  KerMLParser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *kermlParserStaticData->atn, kermlParserStaticData->decisionToDFA, kermlParserStaticData->sharedContextCache, options);
}

KerMLParser::~KerMLParser() {
  delete _interpreter;
}

const atn::ATN& KerMLParser::getATN() const {
  return *kermlParserStaticData->atn;
}

std::string KerMLParser::getGrammarFileName() const {
  return "KerML.g4";
}

const std::vector<std::string>& KerMLParser::getRuleNames() const {
  return kermlParserStaticData->ruleNames;
}

const dfa::Vocabulary& KerMLParser::getVocabulary() const {
  return kermlParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView KerMLParser::getSerializedATN() const {
  return kermlParserStaticData->serializedATN;
}


//----------------- StartContext ------------------------------------------------------------------

KerMLParser::StartContext::StartContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

KerMLParser::ElementsContext* KerMLParser::StartContext::elements() {
  return getRuleContext<KerMLParser::ElementsContext>(0);
}

tree::TerminalNode* KerMLParser::StartContext::EOF() {
  return getToken(KerMLParser::EOF, 0);
}


size_t KerMLParser::StartContext::getRuleIndex() const {
  return KerMLParser::RuleStart;
}

void KerMLParser::StartContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterStart(this);
}

void KerMLParser::StartContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitStart(this);
}

KerMLParser::StartContext* KerMLParser::start() {
  StartContext *_localctx = _tracker.createInstance<StartContext>(_ctx, getState());
  enterRule(_localctx, 0, KerMLParser::RuleStart);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(50);
    elements();
    setState(51);
    match(KerMLParser::EOF);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- StartRuleContext ------------------------------------------------------------------

KerMLParser::StartRuleContext::StartRuleContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

KerMLParser::StartContext* KerMLParser::StartRuleContext::start() {
  return getRuleContext<KerMLParser::StartContext>(0);
}


size_t KerMLParser::StartRuleContext::getRuleIndex() const {
  return KerMLParser::RuleStartRule;
}

void KerMLParser::StartRuleContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterStartRule(this);
}

void KerMLParser::StartRuleContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitStartRule(this);
}

KerMLParser::StartRuleContext* KerMLParser::startRule() {
  StartRuleContext *_localctx = _tracker.createInstance<StartRuleContext>(_ctx, getState());
  enterRule(_localctx, 2, KerMLParser::RuleStartRule);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(53);
    start();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ElementsContext ------------------------------------------------------------------

KerMLParser::ElementsContext::ElementsContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<KerMLParser::ElementContext *> KerMLParser::ElementsContext::element() {
  return getRuleContexts<KerMLParser::ElementContext>();
}

KerMLParser::ElementContext* KerMLParser::ElementsContext::element(size_t i) {
  return getRuleContext<KerMLParser::ElementContext>(i);
}


size_t KerMLParser::ElementsContext::getRuleIndex() const {
  return KerMLParser::RuleElements;
}

void KerMLParser::ElementsContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterElements(this);
}

void KerMLParser::ElementsContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitElements(this);
}

KerMLParser::ElementsContext* KerMLParser::elements() {
  ElementsContext *_localctx = _tracker.createInstance<ElementsContext>(_ctx, getState());
  enterRule(_localctx, 4, KerMLParser::RuleElements);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(58);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 349804536320) != 0)) {
      setState(55);
      element();
      setState(60);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ElementContext ------------------------------------------------------------------

KerMLParser::ElementContext::ElementContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

KerMLParser::NamespaceContext* KerMLParser::ElementContext::namespace_() {
  return getRuleContext<KerMLParser::NamespaceContext>(0);
}

KerMLParser::ClassifierContext* KerMLParser::ElementContext::classifier() {
  return getRuleContext<KerMLParser::ClassifierContext>(0);
}

KerMLParser::FeatureContext* KerMLParser::ElementContext::feature() {
  return getRuleContext<KerMLParser::FeatureContext>(0);
}

KerMLParser::DependencyContext* KerMLParser::ElementContext::dependency() {
  return getRuleContext<KerMLParser::DependencyContext>(0);
}

KerMLParser::CommentContext* KerMLParser::ElementContext::comment() {
  return getRuleContext<KerMLParser::CommentContext>(0);
}

KerMLParser::DocumentationContext* KerMLParser::ElementContext::documentation() {
  return getRuleContext<KerMLParser::DocumentationContext>(0);
}

KerMLParser::VisibilityContext* KerMLParser::ElementContext::visibility() {
  return getRuleContext<KerMLParser::VisibilityContext>(0);
}

KerMLParser::ClassContext* KerMLParser::ElementContext::class_() {
  return getRuleContext<KerMLParser::ClassContext>(0);
}

KerMLParser::DatatypeContext* KerMLParser::ElementContext::datatype() {
  return getRuleContext<KerMLParser::DatatypeContext>(0);
}

KerMLParser::AliasContext* KerMLParser::ElementContext::alias() {
  return getRuleContext<KerMLParser::AliasContext>(0);
}

KerMLParser::ImportRuleContext* KerMLParser::ElementContext::importRule() {
  return getRuleContext<KerMLParser::ImportRuleContext>(0);
}

KerMLParser::TypeContext* KerMLParser::ElementContext::type() {
  return getRuleContext<KerMLParser::TypeContext>(0);
}

KerMLParser::AbstractContext* KerMLParser::ElementContext::abstract() {
  return getRuleContext<KerMLParser::AbstractContext>(0);
}


size_t KerMLParser::ElementContext::getRuleIndex() const {
  return KerMLParser::RuleElement;
}

void KerMLParser::ElementContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterElement(this);
}

void KerMLParser::ElementContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitElement(this);
}

KerMLParser::ElementContext* KerMLParser::element() {
  ElementContext *_localctx = _tracker.createInstance<ElementContext>(_ctx, getState());
  enterRule(_localctx, 6, KerMLParser::RuleElement);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(74);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case KerMLParser::Namespace: {
        enterOuterAlt(_localctx, 1);
        setState(61);
        namespace_();
        break;
      }

      case KerMLParser::Classifier: {
        enterOuterAlt(_localctx, 2);
        setState(62);
        classifier();
        break;
      }

      case KerMLParser::Feature: {
        enterOuterAlt(_localctx, 3);
        setState(63);
        feature();
        break;
      }

      case KerMLParser::Dependency: {
        enterOuterAlt(_localctx, 4);
        setState(64);
        dependency();
        break;
      }

      case KerMLParser::COMMENT_WORD:
      case KerMLParser::LINE_COMMENT: {
        enterOuterAlt(_localctx, 5);
        setState(65);
        comment();
        break;
      }

      case KerMLParser::DocumentationKeyword: {
        enterOuterAlt(_localctx, 6);
        setState(66);
        documentation();
        break;
      }

      case KerMLParser::Private:
      case KerMLParser::PROTECTED:
      case KerMLParser::Public: {
        enterOuterAlt(_localctx, 7);
        setState(67);
        visibility();
        break;
      }

      case KerMLParser::Class: {
        enterOuterAlt(_localctx, 8);
        setState(68);
        class_();
        break;
      }

      case KerMLParser::Datatype: {
        enterOuterAlt(_localctx, 9);
        setState(69);
        datatype();
        break;
      }

      case KerMLParser::ALIAS: {
        enterOuterAlt(_localctx, 10);
        setState(70);
        alias();
        break;
      }

      case KerMLParser::IMPORT: {
        enterOuterAlt(_localctx, 11);
        setState(71);
        importRule();
        break;
      }

      case KerMLParser::TYPE_KEYWORD: {
        enterOuterAlt(_localctx, 12);
        setState(72);
        type();
        break;
      }

      case KerMLParser::T__8: {
        enterOuterAlt(_localctx, 13);
        setState(73);
        abstract();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- CommentContext ------------------------------------------------------------------

KerMLParser::CommentContext::CommentContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* KerMLParser::CommentContext::COMMENT_WORD() {
  return getToken(KerMLParser::COMMENT_WORD, 0);
}

std::vector<tree::TerminalNode *> KerMLParser::CommentContext::NAME() {
  return getTokens(KerMLParser::NAME);
}

tree::TerminalNode* KerMLParser::CommentContext::NAME(size_t i) {
  return getToken(KerMLParser::NAME, i);
}

tree::TerminalNode* KerMLParser::CommentContext::COMMENT_START() {
  return getToken(KerMLParser::COMMENT_START, 0);
}

KerMLParser::CommentTextContext* KerMLParser::CommentContext::commentText() {
  return getRuleContext<KerMLParser::CommentTextContext>(0);
}

tree::TerminalNode* KerMLParser::CommentContext::COMMENT_END() {
  return getToken(KerMLParser::COMMENT_END, 0);
}

KerMLParser::DependencyContext* KerMLParser::CommentContext::dependency() {
  return getRuleContext<KerMLParser::DependencyContext>(0);
}

tree::TerminalNode* KerMLParser::CommentContext::LINE_COMMENT() {
  return getToken(KerMLParser::LINE_COMMENT, 0);
}


size_t KerMLParser::CommentContext::getRuleIndex() const {
  return KerMLParser::RuleComment;
}

void KerMLParser::CommentContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterComment(this);
}

void KerMLParser::CommentContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitComment(this);
}

KerMLParser::CommentContext* KerMLParser::comment() {
  CommentContext *_localctx = _tracker.createInstance<CommentContext>(_ctx, getState());
  enterRule(_localctx, 8, KerMLParser::RuleComment);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(88);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case KerMLParser::COMMENT_WORD: {
        enterOuterAlt(_localctx, 1);
        setState(76);
        match(KerMLParser::COMMENT_WORD);
        setState(77);
        match(KerMLParser::NAME);
        setState(80);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == KerMLParser::T__0) {
          setState(78);
          match(KerMLParser::T__0);
          setState(79);
          match(KerMLParser::NAME);
        }
        setState(82);
        match(KerMLParser::COMMENT_START);
        setState(83);
        commentText();
        setState(84);
        match(KerMLParser::COMMENT_END);
        setState(85);
        dependency();
        break;
      }

      case KerMLParser::LINE_COMMENT: {
        enterOuterAlt(_localctx, 2);
        setState(87);
        match(KerMLParser::LINE_COMMENT);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FeatureContext ------------------------------------------------------------------

KerMLParser::FeatureContext::FeatureContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* KerMLParser::FeatureContext::Feature() {
  return getToken(KerMLParser::Feature, 0);
}

tree::TerminalNode* KerMLParser::FeatureContext::NAME() {
  return getToken(KerMLParser::NAME, 0);
}

KerMLParser::Delimiter_ruleContext* KerMLParser::FeatureContext::delimiter_rule() {
  return getRuleContext<KerMLParser::Delimiter_ruleContext>(0);
}

KerMLParser::InstanceNumbersContext* KerMLParser::FeatureContext::instanceNumbers() {
  return getRuleContext<KerMLParser::InstanceNumbersContext>(0);
}

KerMLParser::SpecializationContext* KerMLParser::FeatureContext::specialization() {
  return getRuleContext<KerMLParser::SpecializationContext>(0);
}


size_t KerMLParser::FeatureContext::getRuleIndex() const {
  return KerMLParser::RuleFeature;
}

void KerMLParser::FeatureContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFeature(this);
}

void KerMLParser::FeatureContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFeature(this);
}

KerMLParser::FeatureContext* KerMLParser::feature() {
  FeatureContext *_localctx = _tracker.createInstance<FeatureContext>(_ctx, getState());
  enterRule(_localctx, 10, KerMLParser::RuleFeature);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(90);
    match(KerMLParser::Feature);
    setState(91);
    match(KerMLParser::NAME);
    setState(93);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == KerMLParser::T__9) {
      setState(92);
      instanceNumbers();
    }
    setState(96);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == KerMLParser::T__2

    || _la == KerMLParser::SPECIALIZES_KEYWORD) {
      setState(95);
      specialization();
    }
    setState(98);
    delimiter_rule();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ClassifierContext ------------------------------------------------------------------

KerMLParser::ClassifierContext::ClassifierContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* KerMLParser::ClassifierContext::Classifier() {
  return getToken(KerMLParser::Classifier, 0);
}

tree::TerminalNode* KerMLParser::ClassifierContext::NAME() {
  return getToken(KerMLParser::NAME, 0);
}

KerMLParser::Delimiter_ruleContext* KerMLParser::ClassifierContext::delimiter_rule() {
  return getRuleContext<KerMLParser::Delimiter_ruleContext>(0);
}


size_t KerMLParser::ClassifierContext::getRuleIndex() const {
  return KerMLParser::RuleClassifier;
}

void KerMLParser::ClassifierContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterClassifier(this);
}

void KerMLParser::ClassifierContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitClassifier(this);
}

KerMLParser::ClassifierContext* KerMLParser::classifier() {
  ClassifierContext *_localctx = _tracker.createInstance<ClassifierContext>(_ctx, getState());
  enterRule(_localctx, 12, KerMLParser::RuleClassifier);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(100);
    match(KerMLParser::Classifier);
    setState(101);
    match(KerMLParser::NAME);
    setState(102);
    delimiter_rule();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- NamespaceContext ------------------------------------------------------------------

KerMLParser::NamespaceContext::NamespaceContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* KerMLParser::NamespaceContext::Namespace() {
  return getToken(KerMLParser::Namespace, 0);
}

tree::TerminalNode* KerMLParser::NamespaceContext::NAME() {
  return getToken(KerMLParser::NAME, 0);
}

KerMLParser::Delimiter_ruleContext* KerMLParser::NamespaceContext::delimiter_rule() {
  return getRuleContext<KerMLParser::Delimiter_ruleContext>(0);
}


size_t KerMLParser::NamespaceContext::getRuleIndex() const {
  return KerMLParser::RuleNamespace;
}

void KerMLParser::NamespaceContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterNamespace(this);
}

void KerMLParser::NamespaceContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitNamespace(this);
}

KerMLParser::NamespaceContext* KerMLParser::namespace_() {
  NamespaceContext *_localctx = _tracker.createInstance<NamespaceContext>(_ctx, getState());
  enterRule(_localctx, 14, KerMLParser::RuleNamespace);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(104);
    match(KerMLParser::Namespace);
    setState(105);
    match(KerMLParser::NAME);
    setState(106);
    delimiter_rule();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DependencyContext ------------------------------------------------------------------

KerMLParser::DependencyContext::DependencyContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* KerMLParser::DependencyContext::Dependency() {
  return getToken(KerMLParser::Dependency, 0);
}

tree::TerminalNode* KerMLParser::DependencyContext::From() {
  return getToken(KerMLParser::From, 0);
}

tree::TerminalNode* KerMLParser::DependencyContext::To() {
  return getToken(KerMLParser::To, 0);
}

tree::TerminalNode* KerMLParser::DependencyContext::DELIMITER() {
  return getToken(KerMLParser::DELIMITER, 0);
}

std::vector<tree::TerminalNode *> KerMLParser::DependencyContext::NAME() {
  return getTokens(KerMLParser::NAME);
}

tree::TerminalNode* KerMLParser::DependencyContext::NAME(size_t i) {
  return getToken(KerMLParser::NAME, i);
}


size_t KerMLParser::DependencyContext::getRuleIndex() const {
  return KerMLParser::RuleDependency;
}

void KerMLParser::DependencyContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterDependency(this);
}

void KerMLParser::DependencyContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitDependency(this);
}

KerMLParser::DependencyContext* KerMLParser::dependency() {
  DependencyContext *_localctx = _tracker.createInstance<DependencyContext>(_ctx, getState());
  enterRule(_localctx, 16, KerMLParser::RuleDependency);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(108);
    match(KerMLParser::Dependency);
    setState(117);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case KerMLParser::NAME: {
        setState(109);
        match(KerMLParser::NAME);
        break;
      }

      case KerMLParser::T__1: {
        setState(110);
        match(KerMLParser::T__1);
        setState(112); 
        _errHandler->sync(this);
        _la = _input->LA(1);
        do {
          setState(111);
          match(KerMLParser::NAME);
          setState(114); 
          _errHandler->sync(this);
          _la = _input->LA(1);
        } while (_la == KerMLParser::NAME);
        setState(116);
        match(KerMLParser::T__1);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    setState(119);
    match(KerMLParser::From);
    setState(128);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case KerMLParser::NAME: {
        setState(120);
        match(KerMLParser::NAME);
        break;
      }

      case KerMLParser::T__1: {
        setState(121);
        match(KerMLParser::T__1);
        setState(123); 
        _errHandler->sync(this);
        _la = _input->LA(1);
        do {
          setState(122);
          match(KerMLParser::NAME);
          setState(125); 
          _errHandler->sync(this);
          _la = _input->LA(1);
        } while (_la == KerMLParser::NAME);
        setState(127);
        match(KerMLParser::T__1);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    setState(130);
    match(KerMLParser::To);
    setState(139);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case KerMLParser::NAME: {
        setState(131);
        match(KerMLParser::NAME);
        break;
      }

      case KerMLParser::T__1: {
        setState(132);
        match(KerMLParser::T__1);
        setState(134); 
        _errHandler->sync(this);
        _la = _input->LA(1);
        do {
          setState(133);
          match(KerMLParser::NAME);
          setState(136); 
          _errHandler->sync(this);
          _la = _input->LA(1);
        } while (_la == KerMLParser::NAME);
        setState(138);
        match(KerMLParser::T__1);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    setState(141);
    match(KerMLParser::DELIMITER);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DocumentationContext ------------------------------------------------------------------

KerMLParser::DocumentationContext::DocumentationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* KerMLParser::DocumentationContext::DocumentationKeyword() {
  return getToken(KerMLParser::DocumentationKeyword, 0);
}

tree::TerminalNode* KerMLParser::DocumentationContext::COMMENT_START() {
  return getToken(KerMLParser::COMMENT_START, 0);
}

KerMLParser::CommentTextContext* KerMLParser::DocumentationContext::commentText() {
  return getRuleContext<KerMLParser::CommentTextContext>(0);
}

tree::TerminalNode* KerMLParser::DocumentationContext::COMMENT_END() {
  return getToken(KerMLParser::COMMENT_END, 0);
}

tree::TerminalNode* KerMLParser::DocumentationContext::NAME() {
  return getToken(KerMLParser::NAME, 0);
}


size_t KerMLParser::DocumentationContext::getRuleIndex() const {
  return KerMLParser::RuleDocumentation;
}

void KerMLParser::DocumentationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterDocumentation(this);
}

void KerMLParser::DocumentationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitDocumentation(this);
}

KerMLParser::DocumentationContext* KerMLParser::documentation() {
  DocumentationContext *_localctx = _tracker.createInstance<DocumentationContext>(_ctx, getState());
  enterRule(_localctx, 18, KerMLParser::RuleDocumentation);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(143);
    match(KerMLParser::DocumentationKeyword);
    setState(145);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == KerMLParser::NAME) {
      setState(144);
      match(KerMLParser::NAME);
    }
    setState(147);
    match(KerMLParser::COMMENT_START);
    setState(148);
    commentText();
    setState(149);
    match(KerMLParser::COMMENT_END);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- VisibilityContext ------------------------------------------------------------------

KerMLParser::VisibilityContext::VisibilityContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* KerMLParser::VisibilityContext::Private() {
  return getToken(KerMLParser::Private, 0);
}

KerMLParser::ElementContext* KerMLParser::VisibilityContext::element() {
  return getRuleContext<KerMLParser::ElementContext>(0);
}

tree::TerminalNode* KerMLParser::VisibilityContext::Public() {
  return getToken(KerMLParser::Public, 0);
}

tree::TerminalNode* KerMLParser::VisibilityContext::PROTECTED() {
  return getToken(KerMLParser::PROTECTED, 0);
}


size_t KerMLParser::VisibilityContext::getRuleIndex() const {
  return KerMLParser::RuleVisibility;
}

void KerMLParser::VisibilityContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterVisibility(this);
}

void KerMLParser::VisibilityContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitVisibility(this);
}

KerMLParser::VisibilityContext* KerMLParser::visibility() {
  VisibilityContext *_localctx = _tracker.createInstance<VisibilityContext>(_ctx, getState());
  enterRule(_localctx, 20, KerMLParser::RuleVisibility);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(157);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case KerMLParser::Private: {
        enterOuterAlt(_localctx, 1);
        setState(151);
        match(KerMLParser::Private);
        setState(152);
        element();
        break;
      }

      case KerMLParser::Public: {
        enterOuterAlt(_localctx, 2);
        setState(153);
        match(KerMLParser::Public);
        setState(154);
        element();
        break;
      }

      case KerMLParser::PROTECTED: {
        enterOuterAlt(_localctx, 3);
        setState(155);
        match(KerMLParser::PROTECTED);
        setState(156);
        element();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ClassContext ------------------------------------------------------------------

KerMLParser::ClassContext::ClassContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* KerMLParser::ClassContext::Class() {
  return getToken(KerMLParser::Class, 0);
}

tree::TerminalNode* KerMLParser::ClassContext::NAME() {
  return getToken(KerMLParser::NAME, 0);
}

KerMLParser::Delimiter_ruleContext* KerMLParser::ClassContext::delimiter_rule() {
  return getRuleContext<KerMLParser::Delimiter_ruleContext>(0);
}

KerMLParser::InstanceNumbersContext* KerMLParser::ClassContext::instanceNumbers() {
  return getRuleContext<KerMLParser::InstanceNumbersContext>(0);
}

KerMLParser::SpecializationContext* KerMLParser::ClassContext::specialization() {
  return getRuleContext<KerMLParser::SpecializationContext>(0);
}


size_t KerMLParser::ClassContext::getRuleIndex() const {
  return KerMLParser::RuleClass;
}

void KerMLParser::ClassContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterClass(this);
}

void KerMLParser::ClassContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitClass(this);
}

KerMLParser::ClassContext* KerMLParser::class_() {
  ClassContext *_localctx = _tracker.createInstance<ClassContext>(_ctx, getState());
  enterRule(_localctx, 22, KerMLParser::RuleClass);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(159);
    match(KerMLParser::Class);
    setState(160);
    match(KerMLParser::NAME);
    setState(162);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == KerMLParser::T__9) {
      setState(161);
      instanceNumbers();
    }
    setState(165);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == KerMLParser::T__2

    || _la == KerMLParser::SPECIALIZES_KEYWORD) {
      setState(164);
      specialization();
    }
    setState(167);
    delimiter_rule();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DatatypeContext ------------------------------------------------------------------

KerMLParser::DatatypeContext::DatatypeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* KerMLParser::DatatypeContext::Datatype() {
  return getToken(KerMLParser::Datatype, 0);
}

tree::TerminalNode* KerMLParser::DatatypeContext::NAME() {
  return getToken(KerMLParser::NAME, 0);
}

KerMLParser::Delimiter_ruleContext* KerMLParser::DatatypeContext::delimiter_rule() {
  return getRuleContext<KerMLParser::Delimiter_ruleContext>(0);
}

KerMLParser::SpecializationContext* KerMLParser::DatatypeContext::specialization() {
  return getRuleContext<KerMLParser::SpecializationContext>(0);
}


size_t KerMLParser::DatatypeContext::getRuleIndex() const {
  return KerMLParser::RuleDatatype;
}

void KerMLParser::DatatypeContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterDatatype(this);
}

void KerMLParser::DatatypeContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitDatatype(this);
}

KerMLParser::DatatypeContext* KerMLParser::datatype() {
  DatatypeContext *_localctx = _tracker.createInstance<DatatypeContext>(_ctx, getState());
  enterRule(_localctx, 24, KerMLParser::RuleDatatype);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(169);
    match(KerMLParser::Datatype);
    setState(170);
    match(KerMLParser::NAME);
    setState(172);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == KerMLParser::T__2

    || _la == KerMLParser::SPECIALIZES_KEYWORD) {
      setState(171);
      specialization();
    }
    setState(174);
    delimiter_rule();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SpecializationContext ------------------------------------------------------------------

KerMLParser::SpecializationContext::SpecializationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<KerMLParser::ImportAddressContext *> KerMLParser::SpecializationContext::importAddress() {
  return getRuleContexts<KerMLParser::ImportAddressContext>();
}

KerMLParser::ImportAddressContext* KerMLParser::SpecializationContext::importAddress(size_t i) {
  return getRuleContext<KerMLParser::ImportAddressContext>(i);
}

tree::TerminalNode* KerMLParser::SpecializationContext::SPECIALIZES_KEYWORD() {
  return getToken(KerMLParser::SPECIALIZES_KEYWORD, 0);
}


size_t KerMLParser::SpecializationContext::getRuleIndex() const {
  return KerMLParser::RuleSpecialization;
}

void KerMLParser::SpecializationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterSpecialization(this);
}

void KerMLParser::SpecializationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitSpecialization(this);
}

KerMLParser::SpecializationContext* KerMLParser::specialization() {
  SpecializationContext *_localctx = _tracker.createInstance<SpecializationContext>(_ctx, getState());
  enterRule(_localctx, 26, KerMLParser::RuleSpecialization);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(194);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case KerMLParser::T__2: {
        enterOuterAlt(_localctx, 1);
        setState(176);
        match(KerMLParser::T__2);
        setState(177);
        importAddress();
        setState(182);
        _errHandler->sync(this);
        _la = _input->LA(1);
        while (_la == KerMLParser::T__3) {
          setState(178);
          match(KerMLParser::T__3);
          setState(179);
          importAddress();
          setState(184);
          _errHandler->sync(this);
          _la = _input->LA(1);
        }
        break;
      }

      case KerMLParser::SPECIALIZES_KEYWORD: {
        enterOuterAlt(_localctx, 2);
        setState(185);
        match(KerMLParser::SPECIALIZES_KEYWORD);
        setState(186);
        importAddress();
        setState(191);
        _errHandler->sync(this);
        _la = _input->LA(1);
        while (_la == KerMLParser::T__3) {
          setState(187);
          match(KerMLParser::T__3);
          setState(188);
          importAddress();
          setState(193);
          _errHandler->sync(this);
          _la = _input->LA(1);
        }
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AliasContext ------------------------------------------------------------------

KerMLParser::AliasContext::AliasContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* KerMLParser::AliasContext::ALIAS() {
  return getToken(KerMLParser::ALIAS, 0);
}

std::vector<tree::TerminalNode *> KerMLParser::AliasContext::NAME() {
  return getTokens(KerMLParser::NAME);
}

tree::TerminalNode* KerMLParser::AliasContext::NAME(size_t i) {
  return getToken(KerMLParser::NAME, i);
}

tree::TerminalNode* KerMLParser::AliasContext::FOR() {
  return getToken(KerMLParser::FOR, 0);
}

KerMLParser::Delimiter_ruleContext* KerMLParser::AliasContext::delimiter_rule() {
  return getRuleContext<KerMLParser::Delimiter_ruleContext>(0);
}


size_t KerMLParser::AliasContext::getRuleIndex() const {
  return KerMLParser::RuleAlias;
}

void KerMLParser::AliasContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterAlias(this);
}

void KerMLParser::AliasContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitAlias(this);
}

KerMLParser::AliasContext* KerMLParser::alias() {
  AliasContext *_localctx = _tracker.createInstance<AliasContext>(_ctx, getState());
  enterRule(_localctx, 28, KerMLParser::RuleAlias);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(196);
    match(KerMLParser::ALIAS);
    setState(200);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == KerMLParser::T__4) {
      setState(197);
      match(KerMLParser::T__4);
      setState(198);
      match(KerMLParser::NAME);
      setState(199);
      match(KerMLParser::T__5);
    }
    setState(202);
    match(KerMLParser::NAME);
    setState(203);
    match(KerMLParser::FOR);
    setState(204);
    match(KerMLParser::NAME);
    setState(205);
    delimiter_rule();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PackageContext ------------------------------------------------------------------

KerMLParser::PackageContext::PackageContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* KerMLParser::PackageContext::PACKAGE() {
  return getToken(KerMLParser::PACKAGE, 0);
}

tree::TerminalNode* KerMLParser::PackageContext::NAME() {
  return getToken(KerMLParser::NAME, 0);
}

KerMLParser::Delimiter_ruleContext* KerMLParser::PackageContext::delimiter_rule() {
  return getRuleContext<KerMLParser::Delimiter_ruleContext>(0);
}


size_t KerMLParser::PackageContext::getRuleIndex() const {
  return KerMLParser::RulePackage;
}

void KerMLParser::PackageContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPackage(this);
}

void KerMLParser::PackageContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPackage(this);
}

KerMLParser::PackageContext* KerMLParser::package() {
  PackageContext *_localctx = _tracker.createInstance<PackageContext>(_ctx, getState());
  enterRule(_localctx, 30, KerMLParser::RulePackage);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(207);
    match(KerMLParser::PACKAGE);
    setState(208);
    match(KerMLParser::NAME);
    setState(209);
    delimiter_rule();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ImportRuleContext ------------------------------------------------------------------

KerMLParser::ImportRuleContext::ImportRuleContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* KerMLParser::ImportRuleContext::IMPORT() {
  return getToken(KerMLParser::IMPORT, 0);
}

KerMLParser::ImportAddressContext* KerMLParser::ImportRuleContext::importAddress() {
  return getRuleContext<KerMLParser::ImportAddressContext>(0);
}

tree::TerminalNode* KerMLParser::ImportRuleContext::DELIMITER() {
  return getToken(KerMLParser::DELIMITER, 0);
}


size_t KerMLParser::ImportRuleContext::getRuleIndex() const {
  return KerMLParser::RuleImportRule;
}

void KerMLParser::ImportRuleContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterImportRule(this);
}

void KerMLParser::ImportRuleContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitImportRule(this);
}

KerMLParser::ImportRuleContext* KerMLParser::importRule() {
  ImportRuleContext *_localctx = _tracker.createInstance<ImportRuleContext>(_ctx, getState());
  enterRule(_localctx, 32, KerMLParser::RuleImportRule);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(211);
    match(KerMLParser::IMPORT);
    setState(212);
    importAddress();
    setState(213);
    match(KerMLParser::DELIMITER);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ImportAddressContext ------------------------------------------------------------------

KerMLParser::ImportAddressContext::ImportAddressContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> KerMLParser::ImportAddressContext::NAME() {
  return getTokens(KerMLParser::NAME);
}

tree::TerminalNode* KerMLParser::ImportAddressContext::NAME(size_t i) {
  return getToken(KerMLParser::NAME, i);
}

std::vector<tree::TerminalNode *> KerMLParser::ImportAddressContext::NAMESPACE_DELIMITER() {
  return getTokens(KerMLParser::NAMESPACE_DELIMITER);
}

tree::TerminalNode* KerMLParser::ImportAddressContext::NAMESPACE_DELIMITER(size_t i) {
  return getToken(KerMLParser::NAMESPACE_DELIMITER, i);
}

std::vector<tree::TerminalNode *> KerMLParser::ImportAddressContext::STAR() {
  return getTokens(KerMLParser::STAR);
}

tree::TerminalNode* KerMLParser::ImportAddressContext::STAR(size_t i) {
  return getToken(KerMLParser::STAR, i);
}


size_t KerMLParser::ImportAddressContext::getRuleIndex() const {
  return KerMLParser::RuleImportAddress;
}

void KerMLParser::ImportAddressContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterImportAddress(this);
}

void KerMLParser::ImportAddressContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitImportAddress(this);
}

KerMLParser::ImportAddressContext* KerMLParser::importAddress() {
  ImportAddressContext *_localctx = _tracker.createInstance<ImportAddressContext>(_ctx, getState());
  enterRule(_localctx, 34, KerMLParser::RuleImportAddress);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(215);
    match(KerMLParser::NAME);
    setState(220);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 21, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(216);
        match(KerMLParser::NAMESPACE_DELIMITER);
        setState(217);
        _la = _input->LA(1);
        if (!(_la == KerMLParser::STAR

        || _la == KerMLParser::NAME)) {
        _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
        } 
      }
      setState(222);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 21, _ctx);
    }
    setState(226);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == KerMLParser::NAMESPACE_DELIMITER) {
      setState(223);
      match(KerMLParser::NAMESPACE_DELIMITER);
      setState(224);
      match(KerMLParser::STAR);
      setState(225);
      match(KerMLParser::STAR);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Bracketed_contentContext ------------------------------------------------------------------

KerMLParser::Bracketed_contentContext::Bracketed_contentContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

KerMLParser::ElementsContext* KerMLParser::Bracketed_contentContext::elements() {
  return getRuleContext<KerMLParser::ElementsContext>(0);
}


size_t KerMLParser::Bracketed_contentContext::getRuleIndex() const {
  return KerMLParser::RuleBracketed_content;
}

void KerMLParser::Bracketed_contentContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBracketed_content(this);
}

void KerMLParser::Bracketed_contentContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBracketed_content(this);
}

KerMLParser::Bracketed_contentContext* KerMLParser::bracketed_content() {
  Bracketed_contentContext *_localctx = _tracker.createInstance<Bracketed_contentContext>(_ctx, getState());
  enterRule(_localctx, 36, KerMLParser::RuleBracketed_content);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(228);
    match(KerMLParser::T__6);
    setState(229);
    elements();
    setState(230);
    match(KerMLParser::T__7);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Delimiter_ruleContext ------------------------------------------------------------------

KerMLParser::Delimiter_ruleContext::Delimiter_ruleContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

KerMLParser::Bracketed_contentContext* KerMLParser::Delimiter_ruleContext::bracketed_content() {
  return getRuleContext<KerMLParser::Bracketed_contentContext>(0);
}

tree::TerminalNode* KerMLParser::Delimiter_ruleContext::DELIMITER() {
  return getToken(KerMLParser::DELIMITER, 0);
}


size_t KerMLParser::Delimiter_ruleContext::getRuleIndex() const {
  return KerMLParser::RuleDelimiter_rule;
}

void KerMLParser::Delimiter_ruleContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterDelimiter_rule(this);
}

void KerMLParser::Delimiter_ruleContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitDelimiter_rule(this);
}

KerMLParser::Delimiter_ruleContext* KerMLParser::delimiter_rule() {
  Delimiter_ruleContext *_localctx = _tracker.createInstance<Delimiter_ruleContext>(_ctx, getState());
  enterRule(_localctx, 38, KerMLParser::RuleDelimiter_rule);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(234);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case KerMLParser::T__6: {
        setState(232);
        bracketed_content();
        break;
      }

      case KerMLParser::DELIMITER: {
        setState(233);
        match(KerMLParser::DELIMITER);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AbstractContext ------------------------------------------------------------------

KerMLParser::AbstractContext::AbstractContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

KerMLParser::ElementContext* KerMLParser::AbstractContext::element() {
  return getRuleContext<KerMLParser::ElementContext>(0);
}


size_t KerMLParser::AbstractContext::getRuleIndex() const {
  return KerMLParser::RuleAbstract;
}

void KerMLParser::AbstractContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterAbstract(this);
}

void KerMLParser::AbstractContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitAbstract(this);
}

KerMLParser::AbstractContext* KerMLParser::abstract() {
  AbstractContext *_localctx = _tracker.createInstance<AbstractContext>(_ctx, getState());
  enterRule(_localctx, 40, KerMLParser::RuleAbstract);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(236);
    match(KerMLParser::T__8);
    setState(237);
    element();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- TypeContext ------------------------------------------------------------------

KerMLParser::TypeContext::TypeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* KerMLParser::TypeContext::TYPE_KEYWORD() {
  return getToken(KerMLParser::TYPE_KEYWORD, 0);
}

tree::TerminalNode* KerMLParser::TypeContext::NAME() {
  return getToken(KerMLParser::NAME, 0);
}

KerMLParser::SpecializationContext* KerMLParser::TypeContext::specialization() {
  return getRuleContext<KerMLParser::SpecializationContext>(0);
}

KerMLParser::Delimiter_ruleContext* KerMLParser::TypeContext::delimiter_rule() {
  return getRuleContext<KerMLParser::Delimiter_ruleContext>(0);
}

KerMLParser::InstanceNumbersContext* KerMLParser::TypeContext::instanceNumbers() {
  return getRuleContext<KerMLParser::InstanceNumbersContext>(0);
}


size_t KerMLParser::TypeContext::getRuleIndex() const {
  return KerMLParser::RuleType;
}

void KerMLParser::TypeContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterType(this);
}

void KerMLParser::TypeContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitType(this);
}

KerMLParser::TypeContext* KerMLParser::type() {
  TypeContext *_localctx = _tracker.createInstance<TypeContext>(_ctx, getState());
  enterRule(_localctx, 42, KerMLParser::RuleType);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(239);
    match(KerMLParser::TYPE_KEYWORD);
    setState(240);
    match(KerMLParser::NAME);
    setState(242);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == KerMLParser::T__9) {
      setState(241);
      instanceNumbers();
    }
    setState(244);
    specialization();
    setState(245);
    delimiter_rule();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- SubtypeContext ------------------------------------------------------------------

KerMLParser::SubtypeContext::SubtypeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* KerMLParser::SubtypeContext::SUBTYPE_KEYWORD() {
  return getToken(KerMLParser::SUBTYPE_KEYWORD, 0);
}

tree::TerminalNode* KerMLParser::SubtypeContext::NAME() {
  return getToken(KerMLParser::NAME, 0);
}

KerMLParser::SpecializationContext* KerMLParser::SubtypeContext::specialization() {
  return getRuleContext<KerMLParser::SpecializationContext>(0);
}

KerMLParser::Delimiter_ruleContext* KerMLParser::SubtypeContext::delimiter_rule() {
  return getRuleContext<KerMLParser::Delimiter_ruleContext>(0);
}


size_t KerMLParser::SubtypeContext::getRuleIndex() const {
  return KerMLParser::RuleSubtype;
}

void KerMLParser::SubtypeContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterSubtype(this);
}

void KerMLParser::SubtypeContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitSubtype(this);
}

KerMLParser::SubtypeContext* KerMLParser::subtype() {
  SubtypeContext *_localctx = _tracker.createInstance<SubtypeContext>(_ctx, getState());
  enterRule(_localctx, 44, KerMLParser::RuleSubtype);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(247);
    match(KerMLParser::SUBTYPE_KEYWORD);
    setState(248);
    match(KerMLParser::NAME);
    setState(249);
    specialization();
    setState(250);
    delimiter_rule();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- InstanceNumbersContext ------------------------------------------------------------------

KerMLParser::InstanceNumbersContext::InstanceNumbersContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* KerMLParser::InstanceNumbersContext::NUMBER() {
  return getToken(KerMLParser::NUMBER, 0);
}


size_t KerMLParser::InstanceNumbersContext::getRuleIndex() const {
  return KerMLParser::RuleInstanceNumbers;
}

void KerMLParser::InstanceNumbersContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterInstanceNumbers(this);
}

void KerMLParser::InstanceNumbersContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitInstanceNumbers(this);
}

KerMLParser::InstanceNumbersContext* KerMLParser::instanceNumbers() {
  InstanceNumbersContext *_localctx = _tracker.createInstance<InstanceNumbersContext>(_ctx, getState());
  enterRule(_localctx, 46, KerMLParser::RuleInstanceNumbers);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(252);
    match(KerMLParser::T__9);
    setState(253);
    match(KerMLParser::NUMBER);
    setState(254);
    match(KerMLParser::T__10);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- CommentTextContext ------------------------------------------------------------------

KerMLParser::CommentTextContext::CommentTextContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> KerMLParser::CommentTextContext::NAME() {
  return getTokens(KerMLParser::NAME);
}

tree::TerminalNode* KerMLParser::CommentTextContext::NAME(size_t i) {
  return getToken(KerMLParser::NAME, i);
}

std::vector<tree::TerminalNode *> KerMLParser::CommentTextContext::To() {
  return getTokens(KerMLParser::To);
}

tree::TerminalNode* KerMLParser::CommentTextContext::To(size_t i) {
  return getToken(KerMLParser::To, i);
}

std::vector<tree::TerminalNode *> KerMLParser::CommentTextContext::From() {
  return getTokens(KerMLParser::From);
}

tree::TerminalNode* KerMLParser::CommentTextContext::From(size_t i) {
  return getToken(KerMLParser::From, i);
}

std::vector<tree::TerminalNode *> KerMLParser::CommentTextContext::Classifier() {
  return getTokens(KerMLParser::Classifier);
}

tree::TerminalNode* KerMLParser::CommentTextContext::Classifier(size_t i) {
  return getToken(KerMLParser::Classifier, i);
}

std::vector<tree::TerminalNode *> KerMLParser::CommentTextContext::DELIMITER() {
  return getTokens(KerMLParser::DELIMITER);
}

tree::TerminalNode* KerMLParser::CommentTextContext::DELIMITER(size_t i) {
  return getToken(KerMLParser::DELIMITER, i);
}

std::vector<tree::TerminalNode *> KerMLParser::CommentTextContext::Feature() {
  return getTokens(KerMLParser::Feature);
}

tree::TerminalNode* KerMLParser::CommentTextContext::Feature(size_t i) {
  return getToken(KerMLParser::Feature, i);
}

std::vector<tree::TerminalNode *> KerMLParser::CommentTextContext::Namespace() {
  return getTokens(KerMLParser::Namespace);
}

tree::TerminalNode* KerMLParser::CommentTextContext::Namespace(size_t i) {
  return getToken(KerMLParser::Namespace, i);
}

std::vector<tree::TerminalNode *> KerMLParser::CommentTextContext::COMMENT_WORD() {
  return getTokens(KerMLParser::COMMENT_WORD);
}

tree::TerminalNode* KerMLParser::CommentTextContext::COMMENT_WORD(size_t i) {
  return getToken(KerMLParser::COMMENT_WORD, i);
}

std::vector<tree::TerminalNode *> KerMLParser::CommentTextContext::Dependency() {
  return getTokens(KerMLParser::Dependency);
}

tree::TerminalNode* KerMLParser::CommentTextContext::Dependency(size_t i) {
  return getToken(KerMLParser::Dependency, i);
}

std::vector<tree::TerminalNode *> KerMLParser::CommentTextContext::DocumentationKeyword() {
  return getTokens(KerMLParser::DocumentationKeyword);
}

tree::TerminalNode* KerMLParser::CommentTextContext::DocumentationKeyword(size_t i) {
  return getToken(KerMLParser::DocumentationKeyword, i);
}

std::vector<tree::TerminalNode *> KerMLParser::CommentTextContext::Private() {
  return getTokens(KerMLParser::Private);
}

tree::TerminalNode* KerMLParser::CommentTextContext::Private(size_t i) {
  return getToken(KerMLParser::Private, i);
}

std::vector<tree::TerminalNode *> KerMLParser::CommentTextContext::Public() {
  return getTokens(KerMLParser::Public);
}

tree::TerminalNode* KerMLParser::CommentTextContext::Public(size_t i) {
  return getToken(KerMLParser::Public, i);
}

std::vector<tree::TerminalNode *> KerMLParser::CommentTextContext::FOR() {
  return getTokens(KerMLParser::FOR);
}

tree::TerminalNode* KerMLParser::CommentTextContext::FOR(size_t i) {
  return getToken(KerMLParser::FOR, i);
}

std::vector<tree::TerminalNode *> KerMLParser::CommentTextContext::Class() {
  return getTokens(KerMLParser::Class);
}

tree::TerminalNode* KerMLParser::CommentTextContext::Class(size_t i) {
  return getToken(KerMLParser::Class, i);
}

std::vector<tree::TerminalNode *> KerMLParser::CommentTextContext::Datatype() {
  return getTokens(KerMLParser::Datatype);
}

tree::TerminalNode* KerMLParser::CommentTextContext::Datatype(size_t i) {
  return getToken(KerMLParser::Datatype, i);
}

std::vector<tree::TerminalNode *> KerMLParser::CommentTextContext::ALIAS() {
  return getTokens(KerMLParser::ALIAS);
}

tree::TerminalNode* KerMLParser::CommentTextContext::ALIAS(size_t i) {
  return getToken(KerMLParser::ALIAS, i);
}

std::vector<tree::TerminalNode *> KerMLParser::CommentTextContext::PACKAGE() {
  return getTokens(KerMLParser::PACKAGE);
}

tree::TerminalNode* KerMLParser::CommentTextContext::PACKAGE(size_t i) {
  return getToken(KerMLParser::PACKAGE, i);
}

std::vector<tree::TerminalNode *> KerMLParser::CommentTextContext::STAR() {
  return getTokens(KerMLParser::STAR);
}

tree::TerminalNode* KerMLParser::CommentTextContext::STAR(size_t i) {
  return getToken(KerMLParser::STAR, i);
}


size_t KerMLParser::CommentTextContext::getRuleIndex() const {
  return KerMLParser::RuleCommentText;
}

void KerMLParser::CommentTextContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterCommentText(this);
}

void KerMLParser::CommentTextContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitCommentText(this);
}

KerMLParser::CommentTextContext* KerMLParser::commentText() {
  CommentTextContext *_localctx = _tracker.createInstance<CommentTextContext>(_ctx, getState());
  enterRule(_localctx, 48, KerMLParser::RuleCommentText);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(259);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 1112387743744) != 0)) {
      setState(256);
      _la = _input->LA(1);
      if (!((((_la & ~ 0x3fULL) == 0) &&
        ((1ULL << _la) & 1112387743744) != 0))) {
      _errHandler->recoverInline(this);
      }
      else {
        _errHandler->reportMatch(this);
        consume();
      }
      setState(261);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

void KerMLParser::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  kermlParserInitialize();
#else
  ::antlr4::internal::call_once(kermlParserOnceFlag, kermlParserInitialize);
#endif
}
