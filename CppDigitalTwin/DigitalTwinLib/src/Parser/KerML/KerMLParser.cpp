
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
      "start", "intenalCodeComment", "comment", "feature", "classifier", 
      "namespace", "dependency", "documentation", "visibility", "class", 
      "datatype", "specialization", "alias", "package", "importRule", "importAddress", 
      "bracketed_content", "delimiter_rule", "abstract", "type", "subtype", 
      "instanceNumbers", "commentText"
    },
    std::vector<std::string>{
      "", "'about'", "'''", "':'", "','", "'<'", "'>'", "'{'", "'}'", "'abstract'", 
      "'['", "']'", "'.'", "'classifier'", "';'", "'feature'", "'namespace'", 
      "'//'", "'/*'", "'*/'", "'comment'", "'dependency'", "'doc'", "'private'", 
      "'protected'", "'public'", "'from'", "'for'", "'to'", "'class'", "'datatype'", 
      "'alias'", "'package'", "'import'", "'*'", "'::'", "'specializes'", 
      "'type'", "'subtype'"
    },
    std::vector<std::string>{
      "", "", "", "", "", "", "", "", "", "", "", "", "", "Classifier", 
      "DELIMITER", "Feature", "Namespace", "Commet", "CommentStart", "CommentEnd", 
      "CommentWord", "Dependency", "DocumentationKeyword", "Private", "PROTECTED", 
      "Public", "From", "FOR", "To", "Class", "Datatype", "ALIAS", "PACKAGE", 
      "IMPORT", "STAR", "NAMESPACE_DELIMITER", "SPECIALIZES_KEYWORD", "TYPE_KEYWORD", 
      "SUBTYPE_KEYWORD", "NUMBER", "NAME", "INT", "WS"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,42,261,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
  	14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,7,
  	21,2,22,7,22,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
  	1,0,3,0,62,8,0,1,0,1,0,5,0,66,8,0,10,0,12,0,69,9,0,1,1,1,1,1,1,1,1,1,
  	1,1,1,3,1,77,8,1,1,2,1,2,1,2,1,2,3,2,83,8,2,1,2,1,2,1,2,1,2,1,3,1,3,1,
  	3,3,3,92,8,3,1,3,3,3,95,8,3,1,3,1,3,1,4,1,4,1,4,1,4,1,5,1,5,1,5,1,5,1,
  	6,1,6,1,6,1,6,4,6,111,8,6,11,6,12,6,112,1,6,3,6,116,8,6,1,6,1,6,1,6,1,
  	6,4,6,122,8,6,11,6,12,6,123,1,6,3,6,127,8,6,1,6,1,6,1,6,1,6,4,6,133,8,
  	6,11,6,12,6,134,1,6,3,6,138,8,6,1,6,1,6,1,7,1,7,3,7,144,8,7,1,7,1,7,1,
  	7,1,7,1,8,1,8,1,8,1,8,1,8,1,8,3,8,156,8,8,1,9,1,9,1,9,3,9,161,8,9,1,9,
  	3,9,164,8,9,1,9,1,9,1,10,1,10,1,10,3,10,171,8,10,1,10,1,10,1,11,1,11,
  	1,11,1,11,5,11,179,8,11,10,11,12,11,182,9,11,1,11,1,11,1,11,1,11,5,11,
  	188,8,11,10,11,12,11,191,9,11,3,11,193,8,11,1,12,1,12,1,12,1,12,3,12,
  	199,8,12,1,12,1,12,1,12,1,12,1,12,1,13,1,13,1,13,1,13,1,14,1,14,1,14,
  	1,14,1,15,1,15,1,15,5,15,217,8,15,10,15,12,15,220,9,15,1,15,1,15,1,15,
  	3,15,225,8,15,1,16,1,16,1,16,1,16,1,17,1,17,3,17,233,8,17,1,18,1,18,1,
  	18,1,19,1,19,1,19,3,19,241,8,19,1,19,1,19,1,19,1,20,1,20,1,20,1,20,1,
  	20,1,21,1,21,1,21,1,21,1,22,5,22,256,8,22,10,22,12,22,259,9,22,1,22,0,
  	1,0,23,0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,
  	0,2,2,0,34,34,40,40,5,0,12,16,20,23,25,32,34,34,40,40,276,0,61,1,0,0,
  	0,2,76,1,0,0,0,4,78,1,0,0,0,6,88,1,0,0,0,8,98,1,0,0,0,10,102,1,0,0,0,
  	12,106,1,0,0,0,14,141,1,0,0,0,16,155,1,0,0,0,18,157,1,0,0,0,20,167,1,
  	0,0,0,22,192,1,0,0,0,24,194,1,0,0,0,26,205,1,0,0,0,28,209,1,0,0,0,30,
  	213,1,0,0,0,32,226,1,0,0,0,34,232,1,0,0,0,36,234,1,0,0,0,38,237,1,0,0,
  	0,40,245,1,0,0,0,42,250,1,0,0,0,44,257,1,0,0,0,46,47,6,0,-1,0,47,62,3,
  	10,5,0,48,62,3,8,4,0,49,62,3,6,3,0,50,62,3,2,1,0,51,62,3,12,6,0,52,62,
  	3,4,2,0,53,62,3,14,7,0,54,62,3,16,8,0,55,62,3,18,9,0,56,62,3,20,10,0,
  	57,62,3,24,12,0,58,62,3,28,14,0,59,62,3,38,19,0,60,62,3,36,18,0,61,46,
  	1,0,0,0,61,48,1,0,0,0,61,49,1,0,0,0,61,50,1,0,0,0,61,51,1,0,0,0,61,52,
  	1,0,0,0,61,53,1,0,0,0,61,54,1,0,0,0,61,55,1,0,0,0,61,56,1,0,0,0,61,57,
  	1,0,0,0,61,58,1,0,0,0,61,59,1,0,0,0,61,60,1,0,0,0,62,67,1,0,0,0,63,64,
  	10,1,0,0,64,66,3,0,0,2,65,63,1,0,0,0,66,69,1,0,0,0,67,65,1,0,0,0,67,68,
  	1,0,0,0,68,1,1,0,0,0,69,67,1,0,0,0,70,71,5,17,0,0,71,77,3,44,22,0,72,
  	73,5,18,0,0,73,74,3,44,22,0,74,75,5,19,0,0,75,77,1,0,0,0,76,70,1,0,0,
  	0,76,72,1,0,0,0,77,3,1,0,0,0,78,79,5,20,0,0,79,82,5,40,0,0,80,81,5,1,
  	0,0,81,83,5,40,0,0,82,80,1,0,0,0,82,83,1,0,0,0,83,84,1,0,0,0,84,85,5,
  	18,0,0,85,86,3,44,22,0,86,87,5,19,0,0,87,5,1,0,0,0,88,89,5,15,0,0,89,
  	91,5,40,0,0,90,92,3,42,21,0,91,90,1,0,0,0,91,92,1,0,0,0,92,94,1,0,0,0,
  	93,95,3,22,11,0,94,93,1,0,0,0,94,95,1,0,0,0,95,96,1,0,0,0,96,97,3,34,
  	17,0,97,7,1,0,0,0,98,99,5,13,0,0,99,100,5,40,0,0,100,101,3,34,17,0,101,
  	9,1,0,0,0,102,103,5,16,0,0,103,104,5,40,0,0,104,105,3,34,17,0,105,11,
  	1,0,0,0,106,115,5,21,0,0,107,116,5,40,0,0,108,110,5,2,0,0,109,111,5,40,
  	0,0,110,109,1,0,0,0,111,112,1,0,0,0,112,110,1,0,0,0,112,113,1,0,0,0,113,
  	114,1,0,0,0,114,116,5,2,0,0,115,107,1,0,0,0,115,108,1,0,0,0,116,117,1,
  	0,0,0,117,126,5,26,0,0,118,127,5,40,0,0,119,121,5,2,0,0,120,122,5,40,
  	0,0,121,120,1,0,0,0,122,123,1,0,0,0,123,121,1,0,0,0,123,124,1,0,0,0,124,
  	125,1,0,0,0,125,127,5,2,0,0,126,118,1,0,0,0,126,119,1,0,0,0,127,128,1,
  	0,0,0,128,137,5,28,0,0,129,138,5,40,0,0,130,132,5,2,0,0,131,133,5,40,
  	0,0,132,131,1,0,0,0,133,134,1,0,0,0,134,132,1,0,0,0,134,135,1,0,0,0,135,
  	136,1,0,0,0,136,138,5,2,0,0,137,129,1,0,0,0,137,130,1,0,0,0,138,139,1,
  	0,0,0,139,140,5,14,0,0,140,13,1,0,0,0,141,143,5,22,0,0,142,144,5,40,0,
  	0,143,142,1,0,0,0,143,144,1,0,0,0,144,145,1,0,0,0,145,146,5,18,0,0,146,
  	147,3,44,22,0,147,148,5,19,0,0,148,15,1,0,0,0,149,150,5,23,0,0,150,156,
  	3,0,0,0,151,152,5,25,0,0,152,156,3,0,0,0,153,154,5,24,0,0,154,156,3,0,
  	0,0,155,149,1,0,0,0,155,151,1,0,0,0,155,153,1,0,0,0,156,17,1,0,0,0,157,
  	158,5,29,0,0,158,160,5,40,0,0,159,161,3,42,21,0,160,159,1,0,0,0,160,161,
  	1,0,0,0,161,163,1,0,0,0,162,164,3,22,11,0,163,162,1,0,0,0,163,164,1,0,
  	0,0,164,165,1,0,0,0,165,166,3,34,17,0,166,19,1,0,0,0,167,168,5,30,0,0,
  	168,170,5,40,0,0,169,171,3,22,11,0,170,169,1,0,0,0,170,171,1,0,0,0,171,
  	172,1,0,0,0,172,173,3,34,17,0,173,21,1,0,0,0,174,175,5,3,0,0,175,180,
  	3,30,15,0,176,177,5,4,0,0,177,179,3,30,15,0,178,176,1,0,0,0,179,182,1,
  	0,0,0,180,178,1,0,0,0,180,181,1,0,0,0,181,193,1,0,0,0,182,180,1,0,0,0,
  	183,184,5,36,0,0,184,189,3,30,15,0,185,186,5,4,0,0,186,188,3,30,15,0,
  	187,185,1,0,0,0,188,191,1,0,0,0,189,187,1,0,0,0,189,190,1,0,0,0,190,193,
  	1,0,0,0,191,189,1,0,0,0,192,174,1,0,0,0,192,183,1,0,0,0,193,23,1,0,0,
  	0,194,198,5,31,0,0,195,196,5,5,0,0,196,197,5,40,0,0,197,199,5,6,0,0,198,
  	195,1,0,0,0,198,199,1,0,0,0,199,200,1,0,0,0,200,201,5,40,0,0,201,202,
  	5,27,0,0,202,203,5,40,0,0,203,204,3,34,17,0,204,25,1,0,0,0,205,206,5,
  	32,0,0,206,207,5,40,0,0,207,208,3,34,17,0,208,27,1,0,0,0,209,210,5,33,
  	0,0,210,211,3,30,15,0,211,212,5,14,0,0,212,29,1,0,0,0,213,218,5,40,0,
  	0,214,215,5,35,0,0,215,217,7,0,0,0,216,214,1,0,0,0,217,220,1,0,0,0,218,
  	216,1,0,0,0,218,219,1,0,0,0,219,224,1,0,0,0,220,218,1,0,0,0,221,222,5,
  	35,0,0,222,223,5,34,0,0,223,225,5,34,0,0,224,221,1,0,0,0,224,225,1,0,
  	0,0,225,31,1,0,0,0,226,227,5,7,0,0,227,228,3,0,0,0,228,229,5,8,0,0,229,
  	33,1,0,0,0,230,233,3,32,16,0,231,233,5,14,0,0,232,230,1,0,0,0,232,231,
  	1,0,0,0,233,35,1,0,0,0,234,235,5,9,0,0,235,236,3,0,0,0,236,37,1,0,0,0,
  	237,238,5,37,0,0,238,240,5,40,0,0,239,241,3,42,21,0,240,239,1,0,0,0,240,
  	241,1,0,0,0,241,242,1,0,0,0,242,243,3,22,11,0,243,244,3,34,17,0,244,39,
  	1,0,0,0,245,246,5,38,0,0,246,247,5,40,0,0,247,248,3,22,11,0,248,249,3,
  	34,17,0,249,41,1,0,0,0,250,251,5,10,0,0,251,252,5,39,0,0,252,253,5,11,
  	0,0,253,43,1,0,0,0,254,256,7,1,0,0,255,254,1,0,0,0,256,259,1,0,0,0,257,
  	255,1,0,0,0,257,258,1,0,0,0,258,45,1,0,0,0,259,257,1,0,0,0,26,61,67,76,
  	82,91,94,112,115,123,126,134,137,143,155,160,163,170,180,189,192,198,
  	218,224,232,240,257
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

KerMLParser::NamespaceContext* KerMLParser::StartContext::namespace_() {
  return getRuleContext<KerMLParser::NamespaceContext>(0);
}

KerMLParser::ClassifierContext* KerMLParser::StartContext::classifier() {
  return getRuleContext<KerMLParser::ClassifierContext>(0);
}

KerMLParser::FeatureContext* KerMLParser::StartContext::feature() {
  return getRuleContext<KerMLParser::FeatureContext>(0);
}

KerMLParser::IntenalCodeCommentContext* KerMLParser::StartContext::intenalCodeComment() {
  return getRuleContext<KerMLParser::IntenalCodeCommentContext>(0);
}

KerMLParser::DependencyContext* KerMLParser::StartContext::dependency() {
  return getRuleContext<KerMLParser::DependencyContext>(0);
}

KerMLParser::CommentContext* KerMLParser::StartContext::comment() {
  return getRuleContext<KerMLParser::CommentContext>(0);
}

KerMLParser::DocumentationContext* KerMLParser::StartContext::documentation() {
  return getRuleContext<KerMLParser::DocumentationContext>(0);
}

KerMLParser::VisibilityContext* KerMLParser::StartContext::visibility() {
  return getRuleContext<KerMLParser::VisibilityContext>(0);
}

KerMLParser::ClassContext* KerMLParser::StartContext::class_() {
  return getRuleContext<KerMLParser::ClassContext>(0);
}

KerMLParser::DatatypeContext* KerMLParser::StartContext::datatype() {
  return getRuleContext<KerMLParser::DatatypeContext>(0);
}

KerMLParser::AliasContext* KerMLParser::StartContext::alias() {
  return getRuleContext<KerMLParser::AliasContext>(0);
}

KerMLParser::ImportRuleContext* KerMLParser::StartContext::importRule() {
  return getRuleContext<KerMLParser::ImportRuleContext>(0);
}

KerMLParser::TypeContext* KerMLParser::StartContext::type() {
  return getRuleContext<KerMLParser::TypeContext>(0);
}

KerMLParser::AbstractContext* KerMLParser::StartContext::abstract() {
  return getRuleContext<KerMLParser::AbstractContext>(0);
}

std::vector<KerMLParser::StartContext *> KerMLParser::StartContext::start() {
  return getRuleContexts<KerMLParser::StartContext>();
}

KerMLParser::StartContext* KerMLParser::StartContext::start(size_t i) {
  return getRuleContext<KerMLParser::StartContext>(i);
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
   return start(0);
}

KerMLParser::StartContext* KerMLParser::start(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  KerMLParser::StartContext *_localctx = _tracker.createInstance<StartContext>(_ctx, parentState);
  KerMLParser::StartContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 0;
  enterRecursionRule(_localctx, 0, KerMLParser::RuleStart, precedence);

    

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    unrollRecursionContexts(parentContext);
  });
  try {
    size_t alt;
    enterOuterAlt(_localctx, 1);
    setState(61);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case KerMLParser::Namespace: {
        setState(47);
        namespace_();
        break;
      }

      case KerMLParser::Classifier: {
        setState(48);
        classifier();
        break;
      }

      case KerMLParser::Feature: {
        setState(49);
        feature();
        break;
      }

      case KerMLParser::Commet:
      case KerMLParser::CommentStart: {
        setState(50);
        intenalCodeComment();
        break;
      }

      case KerMLParser::Dependency: {
        setState(51);
        dependency();
        break;
      }

      case KerMLParser::CommentWord: {
        setState(52);
        comment();
        break;
      }

      case KerMLParser::DocumentationKeyword: {
        setState(53);
        documentation();
        break;
      }

      case KerMLParser::Private:
      case KerMLParser::PROTECTED:
      case KerMLParser::Public: {
        setState(54);
        visibility();
        break;
      }

      case KerMLParser::Class: {
        setState(55);
        class_();
        break;
      }

      case KerMLParser::Datatype: {
        setState(56);
        datatype();
        break;
      }

      case KerMLParser::ALIAS: {
        setState(57);
        alias();
        break;
      }

      case KerMLParser::IMPORT: {
        setState(58);
        importRule();
        break;
      }

      case KerMLParser::TYPE_KEYWORD: {
        setState(59);
        type();
        break;
      }

      case KerMLParser::T__8: {
        setState(60);
        abstract();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    _ctx->stop = _input->LT(-1);
    setState(67);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 1, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<StartContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RuleStart);
        setState(63);

        if (!(precpred(_ctx, 1))) throw FailedPredicateException(this, "precpred(_ctx, 1)");
        setState(64);
        start(2); 
      }
      setState(69);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 1, _ctx);
    }
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }
  return _localctx;
}

//----------------- IntenalCodeCommentContext ------------------------------------------------------------------

KerMLParser::IntenalCodeCommentContext::IntenalCodeCommentContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* KerMLParser::IntenalCodeCommentContext::Commet() {
  return getToken(KerMLParser::Commet, 0);
}

KerMLParser::CommentTextContext* KerMLParser::IntenalCodeCommentContext::commentText() {
  return getRuleContext<KerMLParser::CommentTextContext>(0);
}

tree::TerminalNode* KerMLParser::IntenalCodeCommentContext::CommentStart() {
  return getToken(KerMLParser::CommentStart, 0);
}

tree::TerminalNode* KerMLParser::IntenalCodeCommentContext::CommentEnd() {
  return getToken(KerMLParser::CommentEnd, 0);
}


size_t KerMLParser::IntenalCodeCommentContext::getRuleIndex() const {
  return KerMLParser::RuleIntenalCodeComment;
}

void KerMLParser::IntenalCodeCommentContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterIntenalCodeComment(this);
}

void KerMLParser::IntenalCodeCommentContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<KerMLListener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitIntenalCodeComment(this);
}

KerMLParser::IntenalCodeCommentContext* KerMLParser::intenalCodeComment() {
  IntenalCodeCommentContext *_localctx = _tracker.createInstance<IntenalCodeCommentContext>(_ctx, getState());
  enterRule(_localctx, 2, KerMLParser::RuleIntenalCodeComment);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(76);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case KerMLParser::Commet: {
        enterOuterAlt(_localctx, 1);
        setState(70);
        match(KerMLParser::Commet);
        setState(71);
        commentText();
        break;
      }

      case KerMLParser::CommentStart: {
        enterOuterAlt(_localctx, 2);
        setState(72);
        match(KerMLParser::CommentStart);
        setState(73);
        commentText();
        setState(74);
        match(KerMLParser::CommentEnd);
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

tree::TerminalNode* KerMLParser::CommentContext::CommentWord() {
  return getToken(KerMLParser::CommentWord, 0);
}

std::vector<tree::TerminalNode *> KerMLParser::CommentContext::NAME() {
  return getTokens(KerMLParser::NAME);
}

tree::TerminalNode* KerMLParser::CommentContext::NAME(size_t i) {
  return getToken(KerMLParser::NAME, i);
}

tree::TerminalNode* KerMLParser::CommentContext::CommentStart() {
  return getToken(KerMLParser::CommentStart, 0);
}

KerMLParser::CommentTextContext* KerMLParser::CommentContext::commentText() {
  return getRuleContext<KerMLParser::CommentTextContext>(0);
}

tree::TerminalNode* KerMLParser::CommentContext::CommentEnd() {
  return getToken(KerMLParser::CommentEnd, 0);
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
  enterRule(_localctx, 4, KerMLParser::RuleComment);
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
    setState(78);
    match(KerMLParser::CommentWord);
    setState(79);
    match(KerMLParser::NAME);
    setState(82);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == KerMLParser::T__0) {
      setState(80);
      match(KerMLParser::T__0);
      setState(81);
      match(KerMLParser::NAME);
    }
    setState(84);
    match(KerMLParser::CommentStart);
    setState(85);
    commentText();
    setState(86);
    match(KerMLParser::CommentEnd);
   
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
  enterRule(_localctx, 6, KerMLParser::RuleFeature);
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
    setState(88);
    match(KerMLParser::Feature);
    setState(89);
    match(KerMLParser::NAME);
    setState(91);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == KerMLParser::T__9) {
      setState(90);
      instanceNumbers();
    }
    setState(94);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == KerMLParser::T__2

    || _la == KerMLParser::SPECIALIZES_KEYWORD) {
      setState(93);
      specialization();
    }
    setState(96);
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
  enterRule(_localctx, 8, KerMLParser::RuleClassifier);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(98);
    match(KerMLParser::Classifier);
    setState(99);
    match(KerMLParser::NAME);
    setState(100);
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
  enterRule(_localctx, 10, KerMLParser::RuleNamespace);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(102);
    match(KerMLParser::Namespace);
    setState(103);
    match(KerMLParser::NAME);
    setState(104);
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
  enterRule(_localctx, 12, KerMLParser::RuleDependency);
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
    setState(106);
    match(KerMLParser::Dependency);
    setState(115);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case KerMLParser::NAME: {
        setState(107);
        match(KerMLParser::NAME);
        break;
      }

      case KerMLParser::T__1: {
        setState(108);
        match(KerMLParser::T__1);
        setState(110); 
        _errHandler->sync(this);
        _la = _input->LA(1);
        do {
          setState(109);
          match(KerMLParser::NAME);
          setState(112); 
          _errHandler->sync(this);
          _la = _input->LA(1);
        } while (_la == KerMLParser::NAME);
        setState(114);
        match(KerMLParser::T__1);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    setState(117);
    match(KerMLParser::From);
    setState(126);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case KerMLParser::NAME: {
        setState(118);
        match(KerMLParser::NAME);
        break;
      }

      case KerMLParser::T__1: {
        setState(119);
        match(KerMLParser::T__1);
        setState(121); 
        _errHandler->sync(this);
        _la = _input->LA(1);
        do {
          setState(120);
          match(KerMLParser::NAME);
          setState(123); 
          _errHandler->sync(this);
          _la = _input->LA(1);
        } while (_la == KerMLParser::NAME);
        setState(125);
        match(KerMLParser::T__1);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    setState(128);
    match(KerMLParser::To);
    setState(137);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case KerMLParser::NAME: {
        setState(129);
        match(KerMLParser::NAME);
        break;
      }

      case KerMLParser::T__1: {
        setState(130);
        match(KerMLParser::T__1);
        setState(132); 
        _errHandler->sync(this);
        _la = _input->LA(1);
        do {
          setState(131);
          match(KerMLParser::NAME);
          setState(134); 
          _errHandler->sync(this);
          _la = _input->LA(1);
        } while (_la == KerMLParser::NAME);
        setState(136);
        match(KerMLParser::T__1);
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    setState(139);
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

tree::TerminalNode* KerMLParser::DocumentationContext::CommentStart() {
  return getToken(KerMLParser::CommentStart, 0);
}

KerMLParser::CommentTextContext* KerMLParser::DocumentationContext::commentText() {
  return getRuleContext<KerMLParser::CommentTextContext>(0);
}

tree::TerminalNode* KerMLParser::DocumentationContext::CommentEnd() {
  return getToken(KerMLParser::CommentEnd, 0);
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
  enterRule(_localctx, 14, KerMLParser::RuleDocumentation);
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
    setState(141);
    match(KerMLParser::DocumentationKeyword);
    setState(143);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == KerMLParser::NAME) {
      setState(142);
      match(KerMLParser::NAME);
    }
    setState(145);
    match(KerMLParser::CommentStart);
    setState(146);
    commentText();
    setState(147);
    match(KerMLParser::CommentEnd);
   
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

KerMLParser::StartContext* KerMLParser::VisibilityContext::start() {
  return getRuleContext<KerMLParser::StartContext>(0);
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
  enterRule(_localctx, 16, KerMLParser::RuleVisibility);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(155);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case KerMLParser::Private: {
        enterOuterAlt(_localctx, 1);
        setState(149);
        match(KerMLParser::Private);
        setState(150);
        start(0);
        break;
      }

      case KerMLParser::Public: {
        enterOuterAlt(_localctx, 2);
        setState(151);
        match(KerMLParser::Public);
        setState(152);
        start(0);
        break;
      }

      case KerMLParser::PROTECTED: {
        enterOuterAlt(_localctx, 3);
        setState(153);
        match(KerMLParser::PROTECTED);
        setState(154);
        start(0);
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
  enterRule(_localctx, 18, KerMLParser::RuleClass);
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
    setState(157);
    match(KerMLParser::Class);
    setState(158);
    match(KerMLParser::NAME);
    setState(160);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == KerMLParser::T__9) {
      setState(159);
      instanceNumbers();
    }
    setState(163);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == KerMLParser::T__2

    || _la == KerMLParser::SPECIALIZES_KEYWORD) {
      setState(162);
      specialization();
    }
    setState(165);
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
  enterRule(_localctx, 20, KerMLParser::RuleDatatype);
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
    setState(167);
    match(KerMLParser::Datatype);
    setState(168);
    match(KerMLParser::NAME);
    setState(170);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == KerMLParser::T__2

    || _la == KerMLParser::SPECIALIZES_KEYWORD) {
      setState(169);
      specialization();
    }
    setState(172);
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
  enterRule(_localctx, 22, KerMLParser::RuleSpecialization);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(192);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case KerMLParser::T__2: {
        enterOuterAlt(_localctx, 1);
        setState(174);
        match(KerMLParser::T__2);
        setState(175);
        importAddress();
        setState(180);
        _errHandler->sync(this);
        _la = _input->LA(1);
        while (_la == KerMLParser::T__3) {
          setState(176);
          match(KerMLParser::T__3);
          setState(177);
          importAddress();
          setState(182);
          _errHandler->sync(this);
          _la = _input->LA(1);
        }
        break;
      }

      case KerMLParser::SPECIALIZES_KEYWORD: {
        enterOuterAlt(_localctx, 2);
        setState(183);
        match(KerMLParser::SPECIALIZES_KEYWORD);
        setState(184);
        importAddress();
        setState(189);
        _errHandler->sync(this);
        _la = _input->LA(1);
        while (_la == KerMLParser::T__3) {
          setState(185);
          match(KerMLParser::T__3);
          setState(186);
          importAddress();
          setState(191);
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
  enterRule(_localctx, 24, KerMLParser::RuleAlias);
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
    setState(194);
    match(KerMLParser::ALIAS);
    setState(198);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == KerMLParser::T__4) {
      setState(195);
      match(KerMLParser::T__4);
      setState(196);
      match(KerMLParser::NAME);
      setState(197);
      match(KerMLParser::T__5);
    }
    setState(200);
    match(KerMLParser::NAME);
    setState(201);
    match(KerMLParser::FOR);
    setState(202);
    match(KerMLParser::NAME);
    setState(203);
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
  enterRule(_localctx, 26, KerMLParser::RulePackage);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(205);
    match(KerMLParser::PACKAGE);
    setState(206);
    match(KerMLParser::NAME);
    setState(207);
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
  enterRule(_localctx, 28, KerMLParser::RuleImportRule);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(209);
    match(KerMLParser::IMPORT);
    setState(210);
    importAddress();
    setState(211);
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
  enterRule(_localctx, 30, KerMLParser::RuleImportAddress);
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
    setState(213);
    match(KerMLParser::NAME);
    setState(218);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 21, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(214);
        match(KerMLParser::NAMESPACE_DELIMITER);
        setState(215);
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
      setState(220);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 21, _ctx);
    }
    setState(224);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == KerMLParser::NAMESPACE_DELIMITER) {
      setState(221);
      match(KerMLParser::NAMESPACE_DELIMITER);
      setState(222);
      match(KerMLParser::STAR);
      setState(223);
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

KerMLParser::StartContext* KerMLParser::Bracketed_contentContext::start() {
  return getRuleContext<KerMLParser::StartContext>(0);
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
  enterRule(_localctx, 32, KerMLParser::RuleBracketed_content);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(226);
    match(KerMLParser::T__6);
    setState(227);
    start(0);
    setState(228);
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
  enterRule(_localctx, 34, KerMLParser::RuleDelimiter_rule);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(232);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case KerMLParser::T__6: {
        setState(230);
        bracketed_content();
        break;
      }

      case KerMLParser::DELIMITER: {
        setState(231);
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

KerMLParser::StartContext* KerMLParser::AbstractContext::start() {
  return getRuleContext<KerMLParser::StartContext>(0);
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
  enterRule(_localctx, 36, KerMLParser::RuleAbstract);

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
    match(KerMLParser::T__8);
    setState(235);
    start(0);
   
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
  enterRule(_localctx, 38, KerMLParser::RuleType);
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
    setState(237);
    match(KerMLParser::TYPE_KEYWORD);
    setState(238);
    match(KerMLParser::NAME);
    setState(240);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == KerMLParser::T__9) {
      setState(239);
      instanceNumbers();
    }
    setState(242);
    specialization();
    setState(243);
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
  enterRule(_localctx, 40, KerMLParser::RuleSubtype);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(245);
    match(KerMLParser::SUBTYPE_KEYWORD);
    setState(246);
    match(KerMLParser::NAME);
    setState(247);
    specialization();
    setState(248);
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
  enterRule(_localctx, 42, KerMLParser::RuleInstanceNumbers);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(250);
    match(KerMLParser::T__9);
    setState(251);
    match(KerMLParser::NUMBER);
    setState(252);
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

std::vector<tree::TerminalNode *> KerMLParser::CommentTextContext::CommentWord() {
  return getTokens(KerMLParser::CommentWord);
}

tree::TerminalNode* KerMLParser::CommentTextContext::CommentWord(size_t i) {
  return getToken(KerMLParser::CommentWord, i);
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
  enterRule(_localctx, 44, KerMLParser::RuleCommentText);
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
    setState(257);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 25, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(254);
        _la = _input->LA(1);
        if (!((((_la & ~ 0x3fULL) == 0) &&
          ((1ULL << _la) & 1125263732736) != 0))) {
        _errHandler->recoverInline(this);
        }
        else {
          _errHandler->reportMatch(this);
          consume();
        } 
      }
      setState(259);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 25, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

bool KerMLParser::sempred(RuleContext *context, size_t ruleIndex, size_t predicateIndex) {
  switch (ruleIndex) {
    case 0: return startSempred(antlrcpp::downCast<StartContext *>(context), predicateIndex);

  default:
    break;
  }
  return true;
}

bool KerMLParser::startSempred(StartContext *, size_t predicateIndex) {
  switch (predicateIndex) {
    case 0: return precpred(_ctx, 1);

  default:
    break;
  }
  return true;
}

void KerMLParser::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  kermlParserInitialize();
#else
  ::antlr4::internal::call_once(kermlParserOnceFlag, kermlParserInitialize);
#endif
}
