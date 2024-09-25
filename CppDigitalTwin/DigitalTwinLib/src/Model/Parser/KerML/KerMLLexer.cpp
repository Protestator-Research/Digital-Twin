
// Generated from KerML.g4 by ANTLR 4.13.2


#include "KerMLLexer.h"


using namespace antlr4;



using namespace antlr4;

namespace {

struct KerMLLexerStaticData final {
  KerMLLexerStaticData(std::vector<std::string> ruleNames,
                          std::vector<std::string> channelNames,
                          std::vector<std::string> modeNames,
                          std::vector<std::string> literalNames,
                          std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), channelNames(std::move(channelNames)),
        modeNames(std::move(modeNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  KerMLLexerStaticData(const KerMLLexerStaticData&) = delete;
  KerMLLexerStaticData(KerMLLexerStaticData&&) = delete;
  KerMLLexerStaticData& operator=(const KerMLLexerStaticData&) = delete;
  KerMLLexerStaticData& operator=(KerMLLexerStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> channelNames;
  const std::vector<std::string> modeNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag kermllexerLexerOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<KerMLLexerStaticData> kermllexerLexerStaticData = nullptr;

void kermllexerLexerInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (kermllexerLexerStaticData != nullptr) {
    return;
  }
#else
  assert(kermllexerLexerStaticData == nullptr);
#endif
  auto staticData = std::make_unique<KerMLLexerStaticData>(
    std::vector<std::string>{
      "T__0", "T__1", "T__2", "T__3", "T__4", "T__5", "T__6", "T__7", "T__8", 
      "T__9", "T__10", "T__11", "Classifier", "DELIMITER", "Feature", "Namespace", 
      "Commet", "CommentStart", "CommentEnd", "CommentWord", "Dependency", 
      "DocumentationKeyword", "Private", "PROTECTED", "Public", "From", 
      "FOR", "To", "Class", "Datatype", "ALIAS", "PACKAGE", "IMPORT", "STAR", 
      "NAMESPACE_DELIMITER", "SPECIALIZES_KEYWORD", "TYPE_KEYWORD", "SUBTYPE_KEYWORD", 
      "NUMBER", "NAME", "INT", "WS"
    },
    std::vector<std::string>{
      "DEFAULT_TOKEN_CHANNEL", "HIDDEN"
    },
    std::vector<std::string>{
      "DEFAULT_MODE"
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
  	4,0,42,308,6,-1,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,
  	6,2,7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,
  	7,14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,
  	7,21,2,22,7,22,2,23,7,23,2,24,7,24,2,25,7,25,2,26,7,26,2,27,7,27,2,28,
  	7,28,2,29,7,29,2,30,7,30,2,31,7,31,2,32,7,32,2,33,7,33,2,34,7,34,2,35,
  	7,35,2,36,7,36,2,37,7,37,2,38,7,38,2,39,7,39,2,40,7,40,2,41,7,41,1,0,
  	1,0,1,0,1,0,1,0,1,0,1,1,1,1,1,2,1,2,1,3,1,3,1,4,1,4,1,5,1,5,1,6,1,6,1,
  	7,1,7,1,8,1,8,1,8,1,8,1,8,1,8,1,8,1,8,1,8,1,9,1,9,1,10,1,10,1,11,1,11,
  	1,12,1,12,1,12,1,12,1,12,1,12,1,12,1,12,1,12,1,12,1,12,1,13,1,13,1,14,
  	1,14,1,14,1,14,1,14,1,14,1,14,1,14,1,15,1,15,1,15,1,15,1,15,1,15,1,15,
  	1,15,1,15,1,15,1,16,1,16,1,16,1,17,1,17,1,17,1,18,1,18,1,18,1,19,1,19,
  	1,19,1,19,1,19,1,19,1,19,1,19,1,20,1,20,1,20,1,20,1,20,1,20,1,20,1,20,
  	1,20,1,20,1,20,1,21,1,21,1,21,1,21,1,22,1,22,1,22,1,22,1,22,1,22,1,22,
  	1,22,1,23,1,23,1,23,1,23,1,23,1,23,1,23,1,23,1,23,1,23,1,24,1,24,1,24,
  	1,24,1,24,1,24,1,24,1,25,1,25,1,25,1,25,1,25,1,26,1,26,1,26,1,26,1,27,
  	1,27,1,27,1,28,1,28,1,28,1,28,1,28,1,28,1,29,1,29,1,29,1,29,1,29,1,29,
  	1,29,1,29,1,29,1,30,1,30,1,30,1,30,1,30,1,30,1,31,1,31,1,31,1,31,1,31,
  	1,31,1,31,1,31,1,32,1,32,1,32,1,32,1,32,1,32,1,32,1,33,1,33,1,34,1,34,
  	1,34,1,35,1,35,1,35,1,35,1,35,1,35,1,35,1,35,1,35,1,35,1,35,1,35,1,36,
  	1,36,1,36,1,36,1,36,1,37,1,37,1,37,1,37,1,37,1,37,1,37,1,37,1,38,4,38,
  	288,8,38,11,38,12,38,289,1,39,4,39,293,8,39,11,39,12,39,294,1,40,4,40,
  	298,8,40,11,40,12,40,299,1,41,4,41,303,8,41,11,41,12,41,304,1,41,1,41,
  	0,0,42,1,1,3,2,5,3,7,4,9,5,11,6,13,7,15,8,17,9,19,10,21,11,23,12,25,13,
  	27,14,29,15,31,16,33,17,35,18,37,19,39,20,41,21,43,22,45,23,47,24,49,
  	25,51,26,53,27,55,28,57,29,59,30,61,31,63,32,65,33,67,34,69,35,71,36,
  	73,37,75,38,77,39,79,40,81,41,83,42,1,0,3,1,0,48,57,3,0,48,57,65,90,97,
  	122,3,0,9,10,13,13,32,32,311,0,1,1,0,0,0,0,3,1,0,0,0,0,5,1,0,0,0,0,7,
  	1,0,0,0,0,9,1,0,0,0,0,11,1,0,0,0,0,13,1,0,0,0,0,15,1,0,0,0,0,17,1,0,0,
  	0,0,19,1,0,0,0,0,21,1,0,0,0,0,23,1,0,0,0,0,25,1,0,0,0,0,27,1,0,0,0,0,
  	29,1,0,0,0,0,31,1,0,0,0,0,33,1,0,0,0,0,35,1,0,0,0,0,37,1,0,0,0,0,39,1,
  	0,0,0,0,41,1,0,0,0,0,43,1,0,0,0,0,45,1,0,0,0,0,47,1,0,0,0,0,49,1,0,0,
  	0,0,51,1,0,0,0,0,53,1,0,0,0,0,55,1,0,0,0,0,57,1,0,0,0,0,59,1,0,0,0,0,
  	61,1,0,0,0,0,63,1,0,0,0,0,65,1,0,0,0,0,67,1,0,0,0,0,69,1,0,0,0,0,71,1,
  	0,0,0,0,73,1,0,0,0,0,75,1,0,0,0,0,77,1,0,0,0,0,79,1,0,0,0,0,81,1,0,0,
  	0,0,83,1,0,0,0,1,85,1,0,0,0,3,91,1,0,0,0,5,93,1,0,0,0,7,95,1,0,0,0,9,
  	97,1,0,0,0,11,99,1,0,0,0,13,101,1,0,0,0,15,103,1,0,0,0,17,105,1,0,0,0,
  	19,114,1,0,0,0,21,116,1,0,0,0,23,118,1,0,0,0,25,120,1,0,0,0,27,131,1,
  	0,0,0,29,133,1,0,0,0,31,141,1,0,0,0,33,151,1,0,0,0,35,154,1,0,0,0,37,
  	157,1,0,0,0,39,160,1,0,0,0,41,168,1,0,0,0,43,179,1,0,0,0,45,183,1,0,0,
  	0,47,191,1,0,0,0,49,201,1,0,0,0,51,208,1,0,0,0,53,213,1,0,0,0,55,217,
  	1,0,0,0,57,220,1,0,0,0,59,226,1,0,0,0,61,235,1,0,0,0,63,241,1,0,0,0,65,
  	249,1,0,0,0,67,256,1,0,0,0,69,258,1,0,0,0,71,261,1,0,0,0,73,273,1,0,0,
  	0,75,278,1,0,0,0,77,287,1,0,0,0,79,292,1,0,0,0,81,297,1,0,0,0,83,302,
  	1,0,0,0,85,86,5,97,0,0,86,87,5,98,0,0,87,88,5,111,0,0,88,89,5,117,0,0,
  	89,90,5,116,0,0,90,2,1,0,0,0,91,92,5,39,0,0,92,4,1,0,0,0,93,94,5,58,0,
  	0,94,6,1,0,0,0,95,96,5,44,0,0,96,8,1,0,0,0,97,98,5,60,0,0,98,10,1,0,0,
  	0,99,100,5,62,0,0,100,12,1,0,0,0,101,102,5,123,0,0,102,14,1,0,0,0,103,
  	104,5,125,0,0,104,16,1,0,0,0,105,106,5,97,0,0,106,107,5,98,0,0,107,108,
  	5,115,0,0,108,109,5,116,0,0,109,110,5,114,0,0,110,111,5,97,0,0,111,112,
  	5,99,0,0,112,113,5,116,0,0,113,18,1,0,0,0,114,115,5,91,0,0,115,20,1,0,
  	0,0,116,117,5,93,0,0,117,22,1,0,0,0,118,119,5,46,0,0,119,24,1,0,0,0,120,
  	121,5,99,0,0,121,122,5,108,0,0,122,123,5,97,0,0,123,124,5,115,0,0,124,
  	125,5,115,0,0,125,126,5,105,0,0,126,127,5,102,0,0,127,128,5,105,0,0,128,
  	129,5,101,0,0,129,130,5,114,0,0,130,26,1,0,0,0,131,132,5,59,0,0,132,28,
  	1,0,0,0,133,134,5,102,0,0,134,135,5,101,0,0,135,136,5,97,0,0,136,137,
  	5,116,0,0,137,138,5,117,0,0,138,139,5,114,0,0,139,140,5,101,0,0,140,30,
  	1,0,0,0,141,142,5,110,0,0,142,143,5,97,0,0,143,144,5,109,0,0,144,145,
  	5,101,0,0,145,146,5,115,0,0,146,147,5,112,0,0,147,148,5,97,0,0,148,149,
  	5,99,0,0,149,150,5,101,0,0,150,32,1,0,0,0,151,152,5,47,0,0,152,153,5,
  	47,0,0,153,34,1,0,0,0,154,155,5,47,0,0,155,156,5,42,0,0,156,36,1,0,0,
  	0,157,158,5,42,0,0,158,159,5,47,0,0,159,38,1,0,0,0,160,161,5,99,0,0,161,
  	162,5,111,0,0,162,163,5,109,0,0,163,164,5,109,0,0,164,165,5,101,0,0,165,
  	166,5,110,0,0,166,167,5,116,0,0,167,40,1,0,0,0,168,169,5,100,0,0,169,
  	170,5,101,0,0,170,171,5,112,0,0,171,172,5,101,0,0,172,173,5,110,0,0,173,
  	174,5,100,0,0,174,175,5,101,0,0,175,176,5,110,0,0,176,177,5,99,0,0,177,
  	178,5,121,0,0,178,42,1,0,0,0,179,180,5,100,0,0,180,181,5,111,0,0,181,
  	182,5,99,0,0,182,44,1,0,0,0,183,184,5,112,0,0,184,185,5,114,0,0,185,186,
  	5,105,0,0,186,187,5,118,0,0,187,188,5,97,0,0,188,189,5,116,0,0,189,190,
  	5,101,0,0,190,46,1,0,0,0,191,192,5,112,0,0,192,193,5,114,0,0,193,194,
  	5,111,0,0,194,195,5,116,0,0,195,196,5,101,0,0,196,197,5,99,0,0,197,198,
  	5,116,0,0,198,199,5,101,0,0,199,200,5,100,0,0,200,48,1,0,0,0,201,202,
  	5,112,0,0,202,203,5,117,0,0,203,204,5,98,0,0,204,205,5,108,0,0,205,206,
  	5,105,0,0,206,207,5,99,0,0,207,50,1,0,0,0,208,209,5,102,0,0,209,210,5,
  	114,0,0,210,211,5,111,0,0,211,212,5,109,0,0,212,52,1,0,0,0,213,214,5,
  	102,0,0,214,215,5,111,0,0,215,216,5,114,0,0,216,54,1,0,0,0,217,218,5,
  	116,0,0,218,219,5,111,0,0,219,56,1,0,0,0,220,221,5,99,0,0,221,222,5,108,
  	0,0,222,223,5,97,0,0,223,224,5,115,0,0,224,225,5,115,0,0,225,58,1,0,0,
  	0,226,227,5,100,0,0,227,228,5,97,0,0,228,229,5,116,0,0,229,230,5,97,0,
  	0,230,231,5,116,0,0,231,232,5,121,0,0,232,233,5,112,0,0,233,234,5,101,
  	0,0,234,60,1,0,0,0,235,236,5,97,0,0,236,237,5,108,0,0,237,238,5,105,0,
  	0,238,239,5,97,0,0,239,240,5,115,0,0,240,62,1,0,0,0,241,242,5,112,0,0,
  	242,243,5,97,0,0,243,244,5,99,0,0,244,245,5,107,0,0,245,246,5,97,0,0,
  	246,247,5,103,0,0,247,248,5,101,0,0,248,64,1,0,0,0,249,250,5,105,0,0,
  	250,251,5,109,0,0,251,252,5,112,0,0,252,253,5,111,0,0,253,254,5,114,0,
  	0,254,255,5,116,0,0,255,66,1,0,0,0,256,257,5,42,0,0,257,68,1,0,0,0,258,
  	259,5,58,0,0,259,260,5,58,0,0,260,70,1,0,0,0,261,262,5,115,0,0,262,263,
  	5,112,0,0,263,264,5,101,0,0,264,265,5,99,0,0,265,266,5,105,0,0,266,267,
  	5,97,0,0,267,268,5,108,0,0,268,269,5,105,0,0,269,270,5,122,0,0,270,271,
  	5,101,0,0,271,272,5,115,0,0,272,72,1,0,0,0,273,274,5,116,0,0,274,275,
  	5,121,0,0,275,276,5,112,0,0,276,277,5,101,0,0,277,74,1,0,0,0,278,279,
  	5,115,0,0,279,280,5,117,0,0,280,281,5,98,0,0,281,282,5,116,0,0,282,283,
  	5,121,0,0,283,284,5,112,0,0,284,285,5,101,0,0,285,76,1,0,0,0,286,288,
  	7,0,0,0,287,286,1,0,0,0,288,289,1,0,0,0,289,287,1,0,0,0,289,290,1,0,0,
  	0,290,78,1,0,0,0,291,293,7,1,0,0,292,291,1,0,0,0,293,294,1,0,0,0,294,
  	292,1,0,0,0,294,295,1,0,0,0,295,80,1,0,0,0,296,298,7,0,0,0,297,296,1,
  	0,0,0,298,299,1,0,0,0,299,297,1,0,0,0,299,300,1,0,0,0,300,82,1,0,0,0,
  	301,303,7,2,0,0,302,301,1,0,0,0,303,304,1,0,0,0,304,302,1,0,0,0,304,305,
  	1,0,0,0,305,306,1,0,0,0,306,307,6,41,0,0,307,84,1,0,0,0,6,0,289,292,294,
  	299,304,1,6,0,0
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  kermllexerLexerStaticData = std::move(staticData);
}

}

KerMLLexer::KerMLLexer(CharStream *input) : Lexer(input) {
  KerMLLexer::initialize();
  _interpreter = new atn::LexerATNSimulator(this, *kermllexerLexerStaticData->atn, kermllexerLexerStaticData->decisionToDFA, kermllexerLexerStaticData->sharedContextCache);
}

KerMLLexer::~KerMLLexer() {
  delete _interpreter;
}

std::string KerMLLexer::getGrammarFileName() const {
  return "KerML.g4";
}

const std::vector<std::string>& KerMLLexer::getRuleNames() const {
  return kermllexerLexerStaticData->ruleNames;
}

const std::vector<std::string>& KerMLLexer::getChannelNames() const {
  return kermllexerLexerStaticData->channelNames;
}

const std::vector<std::string>& KerMLLexer::getModeNames() const {
  return kermllexerLexerStaticData->modeNames;
}

const dfa::Vocabulary& KerMLLexer::getVocabulary() const {
  return kermllexerLexerStaticData->vocabulary;
}

antlr4::atn::SerializedATNView KerMLLexer::getSerializedATN() const {
  return kermllexerLexerStaticData->serializedATN;
}

const atn::ATN& KerMLLexer::getATN() const {
  return *kermllexerLexerStaticData->atn;
}




void KerMLLexer::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  kermllexerLexerInitialize();
#else
  ::antlr4::internal::call_once(kermllexerLexerOnceFlag, kermllexerLexerInitialize);
#endif
}
