
// Generated from SysMLv2.g4 by ANTLR 4.13.2


#include "SysMLv2Listener.h"

#include "SysMLv2Parser.h"


using namespace antlrcpp;

using namespace antlr4;

namespace {

struct SysMLv2ParserStaticData final {
  SysMLv2ParserStaticData(std::vector<std::string> ruleNames,
                        std::vector<std::string> literalNames,
                        std::vector<std::string> symbolicNames)
      : ruleNames(std::move(ruleNames)), literalNames(std::move(literalNames)),
        symbolicNames(std::move(symbolicNames)),
        vocabulary(this->literalNames, this->symbolicNames) {}

  SysMLv2ParserStaticData(const SysMLv2ParserStaticData&) = delete;
  SysMLv2ParserStaticData(SysMLv2ParserStaticData&&) = delete;
  SysMLv2ParserStaticData& operator=(const SysMLv2ParserStaticData&) = delete;
  SysMLv2ParserStaticData& operator=(SysMLv2ParserStaticData&&) = delete;

  std::vector<antlr4::dfa::DFA> decisionToDFA;
  antlr4::atn::PredictionContextCache sharedContextCache;
  const std::vector<std::string> ruleNames;
  const std::vector<std::string> literalNames;
  const std::vector<std::string> symbolicNames;
  const antlr4::dfa::Vocabulary vocabulary;
  antlr4::atn::SerializedATNView serializedATN;
  std::unique_ptr<antlr4::atn::ATN> atn;
};

::antlr4::internal::OnceFlag sysmlv2ParserOnceFlag;
#if ANTLR4_USE_THREAD_LOCAL_CACHE
static thread_local
#endif
std::unique_ptr<SysMLv2ParserStaticData> sysmlv2ParserStaticData = nullptr;

void sysmlv2ParserInitialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  if (sysmlv2ParserStaticData != nullptr) {
    return;
  }
#else
  assert(sysmlv2ParserStaticData == nullptr);
#endif
  auto staticData = std::make_unique<SysMLv2ParserStaticData>(
    std::vector<std::string>{
      "start", "elemements", "dependency", "textual_representaion", "comment", 
      "part", "port", "attribute", "item", "package", "assertion", "actions", 
      "alias", "visibility", "import_rule", "abstraction", "variation", 
      "variant", "function", "input", "output", "return", "command_definition", 
      "connectTo", "type_definition", "about", "specilization", "decriptor", 
      "namelist", "name", "address", "bracketed_content", "fuction_arguments", 
      "argument", "delimiter_rule", "multiplicity", "unit", "definition_rule"
    },
    std::vector<std::string>{
      "", "':'", "','", "'\"'", "'''", "'::'", "'{'", "'}'", "'('", "')'", 
      "'['", "']'", "'dependency'", "'rep'", "'language'", "'doc'", "'comment'", 
      "'about'", "'part'", "", "", "'to'", "'from'", "'for'", "';'", "'port'", 
      "'item'", "'package'", "'def'", "'attribute'", "'measurable'", "'controllable'", 
      "'assert'", "'action'", "'alias'", "'private'", "'protected'", "'public'", 
      "'import'", "'abstract'", "'variation'", "'variant'", "'*'", "'calc'", 
      "'in'", "'out'", "'return'", "'#'", "'command'", "'variable'", "'connectTo'"
    },
    std::vector<std::string>{
      "", "", "", "", "", "", "", "", "", "", "", "", "DEPENDENCY", "REPRESENTATION", 
      "LANGUAGE_DESCRIPTOR", "DOCUMENTATION", "COMMENT", "ABOUT", "PART", 
      "BLOCK_COMMENT", "LINE_COMMENT", "TO", "FROM", "FOR", "DELIMITER", 
      "PORT", "ITEM", "PACKAGE", "DEFINITION", "ATTRIBUTE", "MEASURABLE", 
      "CONTROLLABLE", "ASSERT", "ACTION", "ALIAS", "PRIVATE", "PROTECTED", 
      "PUBLIC", "IMPORT", "ABSTRACT", "VARIATION", "VARIANT", "STAR", "CALC", 
      "IN", "OUT", "RETURN", "HASHTAG", "COMMAND", "VARIABLE", "CONNECT_TO", 
      "NUMBER", "NAME", "INT", "WS"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,54,349,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
  	14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,7,
  	21,2,22,7,22,2,23,7,23,2,24,7,24,2,25,7,25,2,26,7,26,2,27,7,27,2,28,7,
  	28,2,29,7,29,2,30,7,30,2,31,7,31,2,32,7,32,2,33,7,33,2,34,7,34,2,35,7,
  	35,2,36,7,36,2,37,7,37,1,0,5,0,78,8,0,10,0,12,0,81,9,0,1,1,1,1,1,1,1,
  	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  	1,1,3,1,105,8,1,1,2,1,2,1,2,1,2,3,2,111,8,2,1,2,1,2,1,2,1,2,1,3,3,3,118,
  	8,3,1,3,3,3,121,8,3,1,3,1,3,1,3,1,4,1,4,1,4,3,4,129,8,4,1,4,3,4,132,8,
  	4,1,4,1,4,1,4,3,4,137,8,4,1,4,1,4,3,4,141,8,4,1,5,1,5,3,5,145,8,5,1,5,
  	1,5,3,5,149,8,5,1,5,1,5,1,6,1,6,3,6,155,8,6,1,6,1,6,3,6,159,8,6,1,6,1,
  	6,1,7,1,7,3,7,165,8,7,1,7,1,7,3,7,169,8,7,1,7,3,7,172,8,7,1,7,3,7,175,
  	8,7,1,7,1,7,1,8,1,8,1,8,1,8,1,9,1,9,1,9,1,9,1,10,1,10,1,10,1,10,1,11,
  	1,11,3,11,193,8,11,1,11,3,11,196,8,11,1,11,1,11,1,11,1,12,1,12,1,12,1,
  	12,1,12,1,12,1,13,1,13,1,13,1,13,1,13,1,13,3,13,213,8,13,1,14,1,14,1,
  	14,1,14,1,15,1,15,1,15,1,16,1,16,1,16,1,17,1,17,1,17,1,18,1,18,1,18,1,
  	18,1,19,1,19,1,19,1,19,1,19,1,20,1,20,1,20,1,20,1,20,1,21,1,21,1,21,1,
  	21,1,22,1,22,1,22,1,22,1,22,1,22,1,23,1,23,1,23,1,23,1,23,1,24,1,24,1,
  	24,1,25,1,25,1,25,1,25,5,25,264,8,25,10,25,12,25,267,9,25,1,26,1,26,1,
  	26,1,27,1,27,1,27,1,27,1,27,1,28,1,28,1,28,5,28,280,8,28,10,28,12,28,
  	283,9,28,1,29,1,29,1,29,4,29,288,8,29,11,29,12,29,289,1,29,3,29,293,8,
  	29,1,30,1,30,1,30,1,30,1,30,3,30,300,8,30,3,30,302,8,30,5,30,304,8,30,
  	10,30,12,30,307,9,30,1,31,1,31,5,31,311,8,31,10,31,12,31,314,9,31,1,31,
  	1,31,1,32,1,32,3,32,320,8,32,1,32,1,32,5,32,324,8,32,10,32,12,32,327,
  	9,32,1,32,1,32,1,33,1,33,1,33,1,34,1,34,3,34,336,8,34,1,35,1,35,1,35,
  	1,35,1,36,1,36,1,36,1,36,1,37,1,37,1,37,1,37,0,0,38,0,2,4,6,8,10,12,14,
  	16,18,20,22,24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,
  	62,64,66,68,70,72,74,0,2,2,0,29,31,49,49,2,0,42,42,51,51,364,0,79,1,0,
  	0,0,2,104,1,0,0,0,4,106,1,0,0,0,6,117,1,0,0,0,8,140,1,0,0,0,10,142,1,
  	0,0,0,12,152,1,0,0,0,14,162,1,0,0,0,16,178,1,0,0,0,18,182,1,0,0,0,20,
  	186,1,0,0,0,22,190,1,0,0,0,24,200,1,0,0,0,26,212,1,0,0,0,28,214,1,0,0,
  	0,30,218,1,0,0,0,32,221,1,0,0,0,34,224,1,0,0,0,36,227,1,0,0,0,38,231,
  	1,0,0,0,40,236,1,0,0,0,42,241,1,0,0,0,44,245,1,0,0,0,46,251,1,0,0,0,48,
  	256,1,0,0,0,50,259,1,0,0,0,52,268,1,0,0,0,54,271,1,0,0,0,56,276,1,0,0,
  	0,58,292,1,0,0,0,60,294,1,0,0,0,62,308,1,0,0,0,64,317,1,0,0,0,66,330,
  	1,0,0,0,68,335,1,0,0,0,70,337,1,0,0,0,72,341,1,0,0,0,74,345,1,0,0,0,76,
  	78,3,2,1,0,77,76,1,0,0,0,78,81,1,0,0,0,79,77,1,0,0,0,79,80,1,0,0,0,80,
  	1,1,0,0,0,81,79,1,0,0,0,82,105,3,4,2,0,83,105,3,8,4,0,84,105,3,10,5,0,
  	85,105,3,12,6,0,86,105,3,16,8,0,87,105,3,18,9,0,88,105,3,14,7,0,89,105,
  	3,20,10,0,90,105,3,22,11,0,91,105,3,24,12,0,92,105,3,26,13,0,93,105,3,
  	28,14,0,94,105,3,30,15,0,95,105,3,32,16,0,96,105,3,34,17,0,97,105,3,36,
  	18,0,98,105,3,38,19,0,99,105,3,40,20,0,100,105,3,42,21,0,101,105,3,44,
  	22,0,102,105,3,6,3,0,103,105,3,46,23,0,104,82,1,0,0,0,104,83,1,0,0,0,
  	104,84,1,0,0,0,104,85,1,0,0,0,104,86,1,0,0,0,104,87,1,0,0,0,104,88,1,
  	0,0,0,104,89,1,0,0,0,104,90,1,0,0,0,104,91,1,0,0,0,104,92,1,0,0,0,104,
  	93,1,0,0,0,104,94,1,0,0,0,104,95,1,0,0,0,104,96,1,0,0,0,104,97,1,0,0,
  	0,104,98,1,0,0,0,104,99,1,0,0,0,104,100,1,0,0,0,104,101,1,0,0,0,104,102,
  	1,0,0,0,104,103,1,0,0,0,105,3,1,0,0,0,106,107,5,12,0,0,107,110,3,56,28,
  	0,108,109,5,22,0,0,109,111,3,56,28,0,110,108,1,0,0,0,110,111,1,0,0,0,
  	111,112,1,0,0,0,112,113,5,21,0,0,113,114,3,56,28,0,114,115,5,24,0,0,115,
  	5,1,0,0,0,116,118,5,13,0,0,117,116,1,0,0,0,117,118,1,0,0,0,118,120,1,
  	0,0,0,119,121,5,52,0,0,120,119,1,0,0,0,120,121,1,0,0,0,121,122,1,0,0,
  	0,122,123,3,54,27,0,123,124,5,19,0,0,124,7,1,0,0,0,125,141,5,19,0,0,126,
  	128,5,16,0,0,127,129,5,52,0,0,128,127,1,0,0,0,128,129,1,0,0,0,129,131,
  	1,0,0,0,130,132,3,50,25,0,131,130,1,0,0,0,131,132,1,0,0,0,132,133,1,0,
  	0,0,133,141,5,19,0,0,134,136,5,15,0,0,135,137,5,52,0,0,136,135,1,0,0,
  	0,136,137,1,0,0,0,137,138,1,0,0,0,138,141,5,19,0,0,139,141,5,20,0,0,140,
  	125,1,0,0,0,140,126,1,0,0,0,140,134,1,0,0,0,140,139,1,0,0,0,141,9,1,0,
  	0,0,142,144,5,18,0,0,143,145,5,28,0,0,144,143,1,0,0,0,144,145,1,0,0,0,
  	145,146,1,0,0,0,146,148,5,52,0,0,147,149,3,52,26,0,148,147,1,0,0,0,148,
  	149,1,0,0,0,149,150,1,0,0,0,150,151,3,68,34,0,151,11,1,0,0,0,152,154,
  	5,25,0,0,153,155,5,28,0,0,154,153,1,0,0,0,154,155,1,0,0,0,155,156,1,0,
  	0,0,156,158,5,52,0,0,157,159,3,52,26,0,158,157,1,0,0,0,158,159,1,0,0,
  	0,159,160,1,0,0,0,160,161,3,68,34,0,161,13,1,0,0,0,162,164,7,0,0,0,163,
  	165,5,28,0,0,164,163,1,0,0,0,164,165,1,0,0,0,165,166,1,0,0,0,166,168,
  	5,52,0,0,167,169,3,70,35,0,168,167,1,0,0,0,168,169,1,0,0,0,169,171,1,
  	0,0,0,170,172,3,48,24,0,171,170,1,0,0,0,171,172,1,0,0,0,172,174,1,0,0,
  	0,173,175,3,72,36,0,174,173,1,0,0,0,174,175,1,0,0,0,175,176,1,0,0,0,176,
  	177,3,68,34,0,177,15,1,0,0,0,178,179,5,26,0,0,179,180,5,52,0,0,180,181,
  	3,68,34,0,181,17,1,0,0,0,182,183,5,27,0,0,183,184,5,52,0,0,184,185,3,
  	68,34,0,185,19,1,0,0,0,186,187,5,32,0,0,187,188,5,52,0,0,188,189,3,68,
  	34,0,189,21,1,0,0,0,190,192,5,33,0,0,191,193,5,28,0,0,192,191,1,0,0,0,
  	192,193,1,0,0,0,193,195,1,0,0,0,194,196,5,52,0,0,195,194,1,0,0,0,195,
  	196,1,0,0,0,196,197,1,0,0,0,197,198,3,64,32,0,198,199,3,62,31,0,199,23,
  	1,0,0,0,200,201,5,34,0,0,201,202,3,60,30,0,202,203,5,23,0,0,203,204,3,
  	60,30,0,204,205,5,24,0,0,205,25,1,0,0,0,206,207,5,35,0,0,207,213,3,0,
  	0,0,208,209,5,37,0,0,209,213,3,0,0,0,210,211,5,36,0,0,211,213,3,0,0,0,
  	212,206,1,0,0,0,212,208,1,0,0,0,212,210,1,0,0,0,213,27,1,0,0,0,214,215,
  	5,38,0,0,215,216,3,60,30,0,216,217,5,24,0,0,217,29,1,0,0,0,218,219,5,
  	39,0,0,219,220,3,0,0,0,220,31,1,0,0,0,221,222,5,40,0,0,222,223,3,0,0,
  	0,223,33,1,0,0,0,224,225,5,41,0,0,225,226,3,10,5,0,226,35,1,0,0,0,227,
  	228,5,43,0,0,228,229,3,74,37,0,229,230,3,62,31,0,230,37,1,0,0,0,231,232,
  	5,44,0,0,232,233,5,52,0,0,233,234,3,48,24,0,234,235,5,24,0,0,235,39,1,
  	0,0,0,236,237,5,45,0,0,237,238,5,52,0,0,238,239,3,48,24,0,239,240,5,24,
  	0,0,240,41,1,0,0,0,241,242,5,46,0,0,242,243,3,48,24,0,243,244,5,24,0,
  	0,244,43,1,0,0,0,245,246,5,47,0,0,246,247,5,48,0,0,247,248,5,52,0,0,248,
  	249,5,29,0,0,249,250,5,24,0,0,250,45,1,0,0,0,251,252,3,60,30,0,252,253,
  	5,50,0,0,253,254,3,60,30,0,254,255,5,24,0,0,255,47,1,0,0,0,256,257,5,
  	1,0,0,257,258,3,60,30,0,258,49,1,0,0,0,259,260,5,17,0,0,260,265,3,60,
  	30,0,261,262,5,2,0,0,262,264,3,60,30,0,263,261,1,0,0,0,264,267,1,0,0,
  	0,265,263,1,0,0,0,265,266,1,0,0,0,266,51,1,0,0,0,267,265,1,0,0,0,268,
  	269,5,1,0,0,269,270,3,60,30,0,270,53,1,0,0,0,271,272,5,14,0,0,272,273,
  	5,3,0,0,273,274,5,52,0,0,274,275,5,3,0,0,275,55,1,0,0,0,276,281,3,58,
  	29,0,277,278,5,2,0,0,278,280,3,58,29,0,279,277,1,0,0,0,280,283,1,0,0,
  	0,281,279,1,0,0,0,281,282,1,0,0,0,282,57,1,0,0,0,283,281,1,0,0,0,284,
  	293,5,52,0,0,285,287,5,4,0,0,286,288,5,52,0,0,287,286,1,0,0,0,288,289,
  	1,0,0,0,289,287,1,0,0,0,289,290,1,0,0,0,290,291,1,0,0,0,291,293,5,4,0,
  	0,292,284,1,0,0,0,292,285,1,0,0,0,293,59,1,0,0,0,294,305,5,52,0,0,295,
  	301,5,5,0,0,296,302,5,52,0,0,297,299,5,42,0,0,298,300,5,42,0,0,299,298,
  	1,0,0,0,299,300,1,0,0,0,300,302,1,0,0,0,301,296,1,0,0,0,301,297,1,0,0,
  	0,302,304,1,0,0,0,303,295,1,0,0,0,304,307,1,0,0,0,305,303,1,0,0,0,305,
  	306,1,0,0,0,306,61,1,0,0,0,307,305,1,0,0,0,308,312,5,6,0,0,309,311,3,
  	2,1,0,310,309,1,0,0,0,311,314,1,0,0,0,312,310,1,0,0,0,312,313,1,0,0,0,
  	313,315,1,0,0,0,314,312,1,0,0,0,315,316,5,7,0,0,316,63,1,0,0,0,317,319,
  	5,8,0,0,318,320,3,66,33,0,319,318,1,0,0,0,319,320,1,0,0,0,320,321,1,0,
  	0,0,321,325,5,2,0,0,322,324,3,66,33,0,323,322,1,0,0,0,324,327,1,0,0,0,
  	325,323,1,0,0,0,325,326,1,0,0,0,326,328,1,0,0,0,327,325,1,0,0,0,328,329,
  	5,9,0,0,329,65,1,0,0,0,330,331,5,52,0,0,331,332,3,48,24,0,332,67,1,0,
  	0,0,333,336,3,62,31,0,334,336,5,24,0,0,335,333,1,0,0,0,335,334,1,0,0,
  	0,336,69,1,0,0,0,337,338,5,10,0,0,338,339,7,1,0,0,339,340,5,11,0,0,340,
  	71,1,0,0,0,341,342,5,10,0,0,342,343,5,52,0,0,343,344,5,11,0,0,344,73,
  	1,0,0,0,345,346,5,28,0,0,346,347,5,52,0,0,347,75,1,0,0,0,31,79,104,110,
  	117,120,128,131,136,140,144,148,154,158,164,168,171,174,192,195,212,265,
  	281,289,292,299,301,305,312,319,325,335
  };
  staticData->serializedATN = antlr4::atn::SerializedATNView(serializedATNSegment, sizeof(serializedATNSegment) / sizeof(serializedATNSegment[0]));

  antlr4::atn::ATNDeserializer deserializer;
  staticData->atn = deserializer.deserialize(staticData->serializedATN);

  const size_t count = staticData->atn->getNumberOfDecisions();
  staticData->decisionToDFA.reserve(count);
  for (size_t i = 0; i < count; i++) { 
    staticData->decisionToDFA.emplace_back(staticData->atn->getDecisionState(i), i);
  }
  sysmlv2ParserStaticData = std::move(staticData);
}

}

SysMLv2Parser::SysMLv2Parser(TokenStream *input) : SysMLv2Parser(input, antlr4::atn::ParserATNSimulatorOptions()) {}

SysMLv2Parser::SysMLv2Parser(TokenStream *input, const antlr4::atn::ParserATNSimulatorOptions &options) : Parser(input) {
  SysMLv2Parser::initialize();
  _interpreter = new atn::ParserATNSimulator(this, *sysmlv2ParserStaticData->atn, sysmlv2ParserStaticData->decisionToDFA, sysmlv2ParserStaticData->sharedContextCache, options);
}

SysMLv2Parser::~SysMLv2Parser() {
  delete _interpreter;
}

const atn::ATN& SysMLv2Parser::getATN() const {
  return *sysmlv2ParserStaticData->atn;
}

std::string SysMLv2Parser::getGrammarFileName() const {
  return "SysMLv2.g4";
}

const std::vector<std::string>& SysMLv2Parser::getRuleNames() const {
  return sysmlv2ParserStaticData->ruleNames;
}

const dfa::Vocabulary& SysMLv2Parser::getVocabulary() const {
  return sysmlv2ParserStaticData->vocabulary;
}

antlr4::atn::SerializedATNView SysMLv2Parser::getSerializedATN() const {
  return sysmlv2ParserStaticData->serializedATN;
}


//----------------- StartContext ------------------------------------------------------------------

SysMLv2Parser::StartContext::StartContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SysMLv2Parser::ElemementsContext *> SysMLv2Parser::StartContext::elemements() {
  return getRuleContexts<SysMLv2Parser::ElemementsContext>();
}

SysMLv2Parser::ElemementsContext* SysMLv2Parser::StartContext::elemements(size_t i) {
  return getRuleContext<SysMLv2Parser::ElemementsContext>(i);
}


size_t SysMLv2Parser::StartContext::getRuleIndex() const {
  return SysMLv2Parser::RuleStart;
}

void SysMLv2Parser::StartContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterStart(this);
}

void SysMLv2Parser::StartContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitStart(this);
}

SysMLv2Parser::StartContext* SysMLv2Parser::start() {
  StartContext *_localctx = _tracker.createInstance<StartContext>(_ctx, getState());
  enterRule(_localctx, 0, SysMLv2Parser::RuleStart);

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
    setState(79);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 0, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        setState(76);
        elemements(); 
      }
      setState(81);
      _errHandler->sync(this);
      alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 0, _ctx);
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ElemementsContext ------------------------------------------------------------------

SysMLv2Parser::ElemementsContext::ElemementsContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysMLv2Parser::DependencyContext* SysMLv2Parser::ElemementsContext::dependency() {
  return getRuleContext<SysMLv2Parser::DependencyContext>(0);
}

SysMLv2Parser::CommentContext* SysMLv2Parser::ElemementsContext::comment() {
  return getRuleContext<SysMLv2Parser::CommentContext>(0);
}

SysMLv2Parser::PartContext* SysMLv2Parser::ElemementsContext::part() {
  return getRuleContext<SysMLv2Parser::PartContext>(0);
}

SysMLv2Parser::PortContext* SysMLv2Parser::ElemementsContext::port() {
  return getRuleContext<SysMLv2Parser::PortContext>(0);
}

SysMLv2Parser::ItemContext* SysMLv2Parser::ElemementsContext::item() {
  return getRuleContext<SysMLv2Parser::ItemContext>(0);
}

SysMLv2Parser::PackageContext* SysMLv2Parser::ElemementsContext::package() {
  return getRuleContext<SysMLv2Parser::PackageContext>(0);
}

SysMLv2Parser::AttributeContext* SysMLv2Parser::ElemementsContext::attribute() {
  return getRuleContext<SysMLv2Parser::AttributeContext>(0);
}

SysMLv2Parser::AssertionContext* SysMLv2Parser::ElemementsContext::assertion() {
  return getRuleContext<SysMLv2Parser::AssertionContext>(0);
}

SysMLv2Parser::ActionsContext* SysMLv2Parser::ElemementsContext::actions() {
  return getRuleContext<SysMLv2Parser::ActionsContext>(0);
}

SysMLv2Parser::AliasContext* SysMLv2Parser::ElemementsContext::alias() {
  return getRuleContext<SysMLv2Parser::AliasContext>(0);
}

SysMLv2Parser::VisibilityContext* SysMLv2Parser::ElemementsContext::visibility() {
  return getRuleContext<SysMLv2Parser::VisibilityContext>(0);
}

SysMLv2Parser::Import_ruleContext* SysMLv2Parser::ElemementsContext::import_rule() {
  return getRuleContext<SysMLv2Parser::Import_ruleContext>(0);
}

SysMLv2Parser::AbstractionContext* SysMLv2Parser::ElemementsContext::abstraction() {
  return getRuleContext<SysMLv2Parser::AbstractionContext>(0);
}

SysMLv2Parser::VariationContext* SysMLv2Parser::ElemementsContext::variation() {
  return getRuleContext<SysMLv2Parser::VariationContext>(0);
}

SysMLv2Parser::VariantContext* SysMLv2Parser::ElemementsContext::variant() {
  return getRuleContext<SysMLv2Parser::VariantContext>(0);
}

SysMLv2Parser::FunctionContext* SysMLv2Parser::ElemementsContext::function() {
  return getRuleContext<SysMLv2Parser::FunctionContext>(0);
}

SysMLv2Parser::InputContext* SysMLv2Parser::ElemementsContext::input() {
  return getRuleContext<SysMLv2Parser::InputContext>(0);
}

SysMLv2Parser::OutputContext* SysMLv2Parser::ElemementsContext::output() {
  return getRuleContext<SysMLv2Parser::OutputContext>(0);
}

SysMLv2Parser::ReturnContext* SysMLv2Parser::ElemementsContext::return_() {
  return getRuleContext<SysMLv2Parser::ReturnContext>(0);
}

SysMLv2Parser::Command_definitionContext* SysMLv2Parser::ElemementsContext::command_definition() {
  return getRuleContext<SysMLv2Parser::Command_definitionContext>(0);
}

SysMLv2Parser::Textual_representaionContext* SysMLv2Parser::ElemementsContext::textual_representaion() {
  return getRuleContext<SysMLv2Parser::Textual_representaionContext>(0);
}

SysMLv2Parser::ConnectToContext* SysMLv2Parser::ElemementsContext::connectTo() {
  return getRuleContext<SysMLv2Parser::ConnectToContext>(0);
}


size_t SysMLv2Parser::ElemementsContext::getRuleIndex() const {
  return SysMLv2Parser::RuleElemements;
}

void SysMLv2Parser::ElemementsContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterElemements(this);
}

void SysMLv2Parser::ElemementsContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitElemements(this);
}

SysMLv2Parser::ElemementsContext* SysMLv2Parser::elemements() {
  ElemementsContext *_localctx = _tracker.createInstance<ElemementsContext>(_ctx, getState());
  enterRule(_localctx, 2, SysMLv2Parser::RuleElemements);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(104);
    _errHandler->sync(this);
    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 1, _ctx)) {
    case 1: {
      enterOuterAlt(_localctx, 1);
      setState(82);
      dependency();
      break;
    }

    case 2: {
      enterOuterAlt(_localctx, 2);
      setState(83);
      comment();
      break;
    }

    case 3: {
      enterOuterAlt(_localctx, 3);
      setState(84);
      part();
      break;
    }

    case 4: {
      enterOuterAlt(_localctx, 4);
      setState(85);
      port();
      break;
    }

    case 5: {
      enterOuterAlt(_localctx, 5);
      setState(86);
      item();
      break;
    }

    case 6: {
      enterOuterAlt(_localctx, 6);
      setState(87);
      package();
      break;
    }

    case 7: {
      enterOuterAlt(_localctx, 7);
      setState(88);
      attribute();
      break;
    }

    case 8: {
      enterOuterAlt(_localctx, 8);
      setState(89);
      assertion();
      break;
    }

    case 9: {
      enterOuterAlt(_localctx, 9);
      setState(90);
      actions();
      break;
    }

    case 10: {
      enterOuterAlt(_localctx, 10);
      setState(91);
      alias();
      break;
    }

    case 11: {
      enterOuterAlt(_localctx, 11);
      setState(92);
      visibility();
      break;
    }

    case 12: {
      enterOuterAlt(_localctx, 12);
      setState(93);
      import_rule();
      break;
    }

    case 13: {
      enterOuterAlt(_localctx, 13);
      setState(94);
      abstraction();
      break;
    }

    case 14: {
      enterOuterAlt(_localctx, 14);
      setState(95);
      variation();
      break;
    }

    case 15: {
      enterOuterAlt(_localctx, 15);
      setState(96);
      variant();
      break;
    }

    case 16: {
      enterOuterAlt(_localctx, 16);
      setState(97);
      function();
      break;
    }

    case 17: {
      enterOuterAlt(_localctx, 17);
      setState(98);
      input();
      break;
    }

    case 18: {
      enterOuterAlt(_localctx, 18);
      setState(99);
      output();
      break;
    }

    case 19: {
      enterOuterAlt(_localctx, 19);
      setState(100);
      return_();
      break;
    }

    case 20: {
      enterOuterAlt(_localctx, 20);
      setState(101);
      command_definition();
      break;
    }

    case 21: {
      enterOuterAlt(_localctx, 21);
      setState(102);
      textual_representaion();
      break;
    }

    case 22: {
      enterOuterAlt(_localctx, 22);
      setState(103);
      connectTo();
      break;
    }

    default:
      break;
    }
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DependencyContext ------------------------------------------------------------------

SysMLv2Parser::DependencyContext::DependencyContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysMLv2Parser::DependencyContext::DEPENDENCY() {
  return getToken(SysMLv2Parser::DEPENDENCY, 0);
}

std::vector<SysMLv2Parser::NamelistContext *> SysMLv2Parser::DependencyContext::namelist() {
  return getRuleContexts<SysMLv2Parser::NamelistContext>();
}

SysMLv2Parser::NamelistContext* SysMLv2Parser::DependencyContext::namelist(size_t i) {
  return getRuleContext<SysMLv2Parser::NamelistContext>(i);
}

tree::TerminalNode* SysMLv2Parser::DependencyContext::TO() {
  return getToken(SysMLv2Parser::TO, 0);
}

tree::TerminalNode* SysMLv2Parser::DependencyContext::DELIMITER() {
  return getToken(SysMLv2Parser::DELIMITER, 0);
}

tree::TerminalNode* SysMLv2Parser::DependencyContext::FROM() {
  return getToken(SysMLv2Parser::FROM, 0);
}


size_t SysMLv2Parser::DependencyContext::getRuleIndex() const {
  return SysMLv2Parser::RuleDependency;
}

void SysMLv2Parser::DependencyContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterDependency(this);
}

void SysMLv2Parser::DependencyContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitDependency(this);
}

SysMLv2Parser::DependencyContext* SysMLv2Parser::dependency() {
  DependencyContext *_localctx = _tracker.createInstance<DependencyContext>(_ctx, getState());
  enterRule(_localctx, 4, SysMLv2Parser::RuleDependency);
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
    match(SysMLv2Parser::DEPENDENCY);
    setState(107);
    namelist();
    setState(110);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SysMLv2Parser::FROM) {
      setState(108);
      match(SysMLv2Parser::FROM);
      setState(109);
      namelist();
    }
    setState(112);
    match(SysMLv2Parser::TO);
    setState(113);
    namelist();
    setState(114);
    match(SysMLv2Parser::DELIMITER);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Textual_representaionContext ------------------------------------------------------------------

SysMLv2Parser::Textual_representaionContext::Textual_representaionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysMLv2Parser::DecriptorContext* SysMLv2Parser::Textual_representaionContext::decriptor() {
  return getRuleContext<SysMLv2Parser::DecriptorContext>(0);
}

tree::TerminalNode* SysMLv2Parser::Textual_representaionContext::BLOCK_COMMENT() {
  return getToken(SysMLv2Parser::BLOCK_COMMENT, 0);
}

tree::TerminalNode* SysMLv2Parser::Textual_representaionContext::REPRESENTATION() {
  return getToken(SysMLv2Parser::REPRESENTATION, 0);
}

tree::TerminalNode* SysMLv2Parser::Textual_representaionContext::NAME() {
  return getToken(SysMLv2Parser::NAME, 0);
}


size_t SysMLv2Parser::Textual_representaionContext::getRuleIndex() const {
  return SysMLv2Parser::RuleTextual_representaion;
}

void SysMLv2Parser::Textual_representaionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterTextual_representaion(this);
}

void SysMLv2Parser::Textual_representaionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitTextual_representaion(this);
}

SysMLv2Parser::Textual_representaionContext* SysMLv2Parser::textual_representaion() {
  Textual_representaionContext *_localctx = _tracker.createInstance<Textual_representaionContext>(_ctx, getState());
  enterRule(_localctx, 6, SysMLv2Parser::RuleTextual_representaion);
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
    setState(117);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SysMLv2Parser::REPRESENTATION) {
      setState(116);
      match(SysMLv2Parser::REPRESENTATION);
    }
    setState(120);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SysMLv2Parser::NAME) {
      setState(119);
      match(SysMLv2Parser::NAME);
    }
    setState(122);
    decriptor();
    setState(123);
    match(SysMLv2Parser::BLOCK_COMMENT);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- CommentContext ------------------------------------------------------------------

SysMLv2Parser::CommentContext::CommentContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysMLv2Parser::CommentContext::BLOCK_COMMENT() {
  return getToken(SysMLv2Parser::BLOCK_COMMENT, 0);
}

tree::TerminalNode* SysMLv2Parser::CommentContext::COMMENT() {
  return getToken(SysMLv2Parser::COMMENT, 0);
}

tree::TerminalNode* SysMLv2Parser::CommentContext::NAME() {
  return getToken(SysMLv2Parser::NAME, 0);
}

SysMLv2Parser::AboutContext* SysMLv2Parser::CommentContext::about() {
  return getRuleContext<SysMLv2Parser::AboutContext>(0);
}

tree::TerminalNode* SysMLv2Parser::CommentContext::DOCUMENTATION() {
  return getToken(SysMLv2Parser::DOCUMENTATION, 0);
}

tree::TerminalNode* SysMLv2Parser::CommentContext::LINE_COMMENT() {
  return getToken(SysMLv2Parser::LINE_COMMENT, 0);
}


size_t SysMLv2Parser::CommentContext::getRuleIndex() const {
  return SysMLv2Parser::RuleComment;
}

void SysMLv2Parser::CommentContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterComment(this);
}

void SysMLv2Parser::CommentContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitComment(this);
}

SysMLv2Parser::CommentContext* SysMLv2Parser::comment() {
  CommentContext *_localctx = _tracker.createInstance<CommentContext>(_ctx, getState());
  enterRule(_localctx, 8, SysMLv2Parser::RuleComment);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(140);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysMLv2Parser::BLOCK_COMMENT: {
        enterOuterAlt(_localctx, 1);
        setState(125);
        match(SysMLv2Parser::BLOCK_COMMENT);
        break;
      }

      case SysMLv2Parser::COMMENT: {
        enterOuterAlt(_localctx, 2);
        setState(126);
        match(SysMLv2Parser::COMMENT);
        setState(128);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == SysMLv2Parser::NAME) {
          setState(127);
          match(SysMLv2Parser::NAME);
        }
        setState(131);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == SysMLv2Parser::ABOUT) {
          setState(130);
          about();
        }
        setState(133);
        match(SysMLv2Parser::BLOCK_COMMENT);
        break;
      }

      case SysMLv2Parser::DOCUMENTATION: {
        enterOuterAlt(_localctx, 3);
        setState(134);
        match(SysMLv2Parser::DOCUMENTATION);
        setState(136);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == SysMLv2Parser::NAME) {
          setState(135);
          match(SysMLv2Parser::NAME);
        }
        setState(138);
        match(SysMLv2Parser::BLOCK_COMMENT);
        break;
      }

      case SysMLv2Parser::LINE_COMMENT: {
        enterOuterAlt(_localctx, 4);
        setState(139);
        match(SysMLv2Parser::LINE_COMMENT);
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

//----------------- PartContext ------------------------------------------------------------------

SysMLv2Parser::PartContext::PartContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysMLv2Parser::PartContext::PART() {
  return getToken(SysMLv2Parser::PART, 0);
}

tree::TerminalNode* SysMLv2Parser::PartContext::NAME() {
  return getToken(SysMLv2Parser::NAME, 0);
}

SysMLv2Parser::Delimiter_ruleContext* SysMLv2Parser::PartContext::delimiter_rule() {
  return getRuleContext<SysMLv2Parser::Delimiter_ruleContext>(0);
}

tree::TerminalNode* SysMLv2Parser::PartContext::DEFINITION() {
  return getToken(SysMLv2Parser::DEFINITION, 0);
}

SysMLv2Parser::SpecilizationContext* SysMLv2Parser::PartContext::specilization() {
  return getRuleContext<SysMLv2Parser::SpecilizationContext>(0);
}


size_t SysMLv2Parser::PartContext::getRuleIndex() const {
  return SysMLv2Parser::RulePart;
}

void SysMLv2Parser::PartContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPart(this);
}

void SysMLv2Parser::PartContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPart(this);
}

SysMLv2Parser::PartContext* SysMLv2Parser::part() {
  PartContext *_localctx = _tracker.createInstance<PartContext>(_ctx, getState());
  enterRule(_localctx, 10, SysMLv2Parser::RulePart);
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
    setState(142);
    match(SysMLv2Parser::PART);
    setState(144);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SysMLv2Parser::DEFINITION) {
      setState(143);
      match(SysMLv2Parser::DEFINITION);
    }
    setState(146);
    match(SysMLv2Parser::NAME);
    setState(148);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SysMLv2Parser::T__0) {
      setState(147);
      specilization();
    }
    setState(150);
    delimiter_rule();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- PortContext ------------------------------------------------------------------

SysMLv2Parser::PortContext::PortContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysMLv2Parser::PortContext::PORT() {
  return getToken(SysMLv2Parser::PORT, 0);
}

tree::TerminalNode* SysMLv2Parser::PortContext::NAME() {
  return getToken(SysMLv2Parser::NAME, 0);
}

SysMLv2Parser::Delimiter_ruleContext* SysMLv2Parser::PortContext::delimiter_rule() {
  return getRuleContext<SysMLv2Parser::Delimiter_ruleContext>(0);
}

tree::TerminalNode* SysMLv2Parser::PortContext::DEFINITION() {
  return getToken(SysMLv2Parser::DEFINITION, 0);
}

SysMLv2Parser::SpecilizationContext* SysMLv2Parser::PortContext::specilization() {
  return getRuleContext<SysMLv2Parser::SpecilizationContext>(0);
}


size_t SysMLv2Parser::PortContext::getRuleIndex() const {
  return SysMLv2Parser::RulePort;
}

void SysMLv2Parser::PortContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPort(this);
}

void SysMLv2Parser::PortContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPort(this);
}

SysMLv2Parser::PortContext* SysMLv2Parser::port() {
  PortContext *_localctx = _tracker.createInstance<PortContext>(_ctx, getState());
  enterRule(_localctx, 12, SysMLv2Parser::RulePort);
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
    setState(152);
    match(SysMLv2Parser::PORT);
    setState(154);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SysMLv2Parser::DEFINITION) {
      setState(153);
      match(SysMLv2Parser::DEFINITION);
    }
    setState(156);
    match(SysMLv2Parser::NAME);
    setState(158);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SysMLv2Parser::T__0) {
      setState(157);
      specilization();
    }
    setState(160);
    delimiter_rule();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AttributeContext ------------------------------------------------------------------

SysMLv2Parser::AttributeContext::AttributeContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysMLv2Parser::AttributeContext::NAME() {
  return getToken(SysMLv2Parser::NAME, 0);
}

SysMLv2Parser::Delimiter_ruleContext* SysMLv2Parser::AttributeContext::delimiter_rule() {
  return getRuleContext<SysMLv2Parser::Delimiter_ruleContext>(0);
}

tree::TerminalNode* SysMLv2Parser::AttributeContext::ATTRIBUTE() {
  return getToken(SysMLv2Parser::ATTRIBUTE, 0);
}

tree::TerminalNode* SysMLv2Parser::AttributeContext::MEASURABLE() {
  return getToken(SysMLv2Parser::MEASURABLE, 0);
}

tree::TerminalNode* SysMLv2Parser::AttributeContext::CONTROLLABLE() {
  return getToken(SysMLv2Parser::CONTROLLABLE, 0);
}

tree::TerminalNode* SysMLv2Parser::AttributeContext::VARIABLE() {
  return getToken(SysMLv2Parser::VARIABLE, 0);
}

tree::TerminalNode* SysMLv2Parser::AttributeContext::DEFINITION() {
  return getToken(SysMLv2Parser::DEFINITION, 0);
}

SysMLv2Parser::MultiplicityContext* SysMLv2Parser::AttributeContext::multiplicity() {
  return getRuleContext<SysMLv2Parser::MultiplicityContext>(0);
}

SysMLv2Parser::Type_definitionContext* SysMLv2Parser::AttributeContext::type_definition() {
  return getRuleContext<SysMLv2Parser::Type_definitionContext>(0);
}

SysMLv2Parser::UnitContext* SysMLv2Parser::AttributeContext::unit() {
  return getRuleContext<SysMLv2Parser::UnitContext>(0);
}


size_t SysMLv2Parser::AttributeContext::getRuleIndex() const {
  return SysMLv2Parser::RuleAttribute;
}

void SysMLv2Parser::AttributeContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterAttribute(this);
}

void SysMLv2Parser::AttributeContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitAttribute(this);
}

SysMLv2Parser::AttributeContext* SysMLv2Parser::attribute() {
  AttributeContext *_localctx = _tracker.createInstance<AttributeContext>(_ctx, getState());
  enterRule(_localctx, 14, SysMLv2Parser::RuleAttribute);
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
    setState(162);
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 562953711517696) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
    setState(164);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SysMLv2Parser::DEFINITION) {
      setState(163);
      match(SysMLv2Parser::DEFINITION);
    }
    setState(166);
    match(SysMLv2Parser::NAME);
    setState(168);
    _errHandler->sync(this);

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 14, _ctx)) {
    case 1: {
      setState(167);
      multiplicity();
      break;
    }

    default:
      break;
    }
    setState(171);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SysMLv2Parser::T__0) {
      setState(170);
      type_definition();
    }
    setState(174);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SysMLv2Parser::T__9) {
      setState(173);
      unit();
    }
    setState(176);
    delimiter_rule();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ItemContext ------------------------------------------------------------------

SysMLv2Parser::ItemContext::ItemContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysMLv2Parser::ItemContext::ITEM() {
  return getToken(SysMLv2Parser::ITEM, 0);
}

tree::TerminalNode* SysMLv2Parser::ItemContext::NAME() {
  return getToken(SysMLv2Parser::NAME, 0);
}

SysMLv2Parser::Delimiter_ruleContext* SysMLv2Parser::ItemContext::delimiter_rule() {
  return getRuleContext<SysMLv2Parser::Delimiter_ruleContext>(0);
}


size_t SysMLv2Parser::ItemContext::getRuleIndex() const {
  return SysMLv2Parser::RuleItem;
}

void SysMLv2Parser::ItemContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterItem(this);
}

void SysMLv2Parser::ItemContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitItem(this);
}

SysMLv2Parser::ItemContext* SysMLv2Parser::item() {
  ItemContext *_localctx = _tracker.createInstance<ItemContext>(_ctx, getState());
  enterRule(_localctx, 16, SysMLv2Parser::RuleItem);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(178);
    match(SysMLv2Parser::ITEM);
    setState(179);
    match(SysMLv2Parser::NAME);
    setState(180);
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

SysMLv2Parser::PackageContext::PackageContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysMLv2Parser::PackageContext::PACKAGE() {
  return getToken(SysMLv2Parser::PACKAGE, 0);
}

tree::TerminalNode* SysMLv2Parser::PackageContext::NAME() {
  return getToken(SysMLv2Parser::NAME, 0);
}

SysMLv2Parser::Delimiter_ruleContext* SysMLv2Parser::PackageContext::delimiter_rule() {
  return getRuleContext<SysMLv2Parser::Delimiter_ruleContext>(0);
}


size_t SysMLv2Parser::PackageContext::getRuleIndex() const {
  return SysMLv2Parser::RulePackage;
}

void SysMLv2Parser::PackageContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterPackage(this);
}

void SysMLv2Parser::PackageContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitPackage(this);
}

SysMLv2Parser::PackageContext* SysMLv2Parser::package() {
  PackageContext *_localctx = _tracker.createInstance<PackageContext>(_ctx, getState());
  enterRule(_localctx, 18, SysMLv2Parser::RulePackage);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(182);
    match(SysMLv2Parser::PACKAGE);
    setState(183);
    match(SysMLv2Parser::NAME);
    setState(184);
    delimiter_rule();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AssertionContext ------------------------------------------------------------------

SysMLv2Parser::AssertionContext::AssertionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysMLv2Parser::AssertionContext::ASSERT() {
  return getToken(SysMLv2Parser::ASSERT, 0);
}

tree::TerminalNode* SysMLv2Parser::AssertionContext::NAME() {
  return getToken(SysMLv2Parser::NAME, 0);
}

SysMLv2Parser::Delimiter_ruleContext* SysMLv2Parser::AssertionContext::delimiter_rule() {
  return getRuleContext<SysMLv2Parser::Delimiter_ruleContext>(0);
}


size_t SysMLv2Parser::AssertionContext::getRuleIndex() const {
  return SysMLv2Parser::RuleAssertion;
}

void SysMLv2Parser::AssertionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterAssertion(this);
}

void SysMLv2Parser::AssertionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitAssertion(this);
}

SysMLv2Parser::AssertionContext* SysMLv2Parser::assertion() {
  AssertionContext *_localctx = _tracker.createInstance<AssertionContext>(_ctx, getState());
  enterRule(_localctx, 20, SysMLv2Parser::RuleAssertion);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(186);
    match(SysMLv2Parser::ASSERT);
    setState(187);
    match(SysMLv2Parser::NAME);
    setState(188);
    delimiter_rule();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ActionsContext ------------------------------------------------------------------

SysMLv2Parser::ActionsContext::ActionsContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysMLv2Parser::ActionsContext::ACTION() {
  return getToken(SysMLv2Parser::ACTION, 0);
}

SysMLv2Parser::Fuction_argumentsContext* SysMLv2Parser::ActionsContext::fuction_arguments() {
  return getRuleContext<SysMLv2Parser::Fuction_argumentsContext>(0);
}

SysMLv2Parser::Bracketed_contentContext* SysMLv2Parser::ActionsContext::bracketed_content() {
  return getRuleContext<SysMLv2Parser::Bracketed_contentContext>(0);
}

tree::TerminalNode* SysMLv2Parser::ActionsContext::DEFINITION() {
  return getToken(SysMLv2Parser::DEFINITION, 0);
}

tree::TerminalNode* SysMLv2Parser::ActionsContext::NAME() {
  return getToken(SysMLv2Parser::NAME, 0);
}


size_t SysMLv2Parser::ActionsContext::getRuleIndex() const {
  return SysMLv2Parser::RuleActions;
}

void SysMLv2Parser::ActionsContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterActions(this);
}

void SysMLv2Parser::ActionsContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitActions(this);
}

SysMLv2Parser::ActionsContext* SysMLv2Parser::actions() {
  ActionsContext *_localctx = _tracker.createInstance<ActionsContext>(_ctx, getState());
  enterRule(_localctx, 22, SysMLv2Parser::RuleActions);
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
    setState(190);
    match(SysMLv2Parser::ACTION);
    setState(192);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SysMLv2Parser::DEFINITION) {
      setState(191);
      match(SysMLv2Parser::DEFINITION);
    }
    setState(195);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SysMLv2Parser::NAME) {
      setState(194);
      match(SysMLv2Parser::NAME);
    }
    setState(197);
    fuction_arguments();
    setState(198);
    bracketed_content();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AliasContext ------------------------------------------------------------------

SysMLv2Parser::AliasContext::AliasContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysMLv2Parser::AliasContext::ALIAS() {
  return getToken(SysMLv2Parser::ALIAS, 0);
}

std::vector<SysMLv2Parser::AddressContext *> SysMLv2Parser::AliasContext::address() {
  return getRuleContexts<SysMLv2Parser::AddressContext>();
}

SysMLv2Parser::AddressContext* SysMLv2Parser::AliasContext::address(size_t i) {
  return getRuleContext<SysMLv2Parser::AddressContext>(i);
}

tree::TerminalNode* SysMLv2Parser::AliasContext::FOR() {
  return getToken(SysMLv2Parser::FOR, 0);
}

tree::TerminalNode* SysMLv2Parser::AliasContext::DELIMITER() {
  return getToken(SysMLv2Parser::DELIMITER, 0);
}


size_t SysMLv2Parser::AliasContext::getRuleIndex() const {
  return SysMLv2Parser::RuleAlias;
}

void SysMLv2Parser::AliasContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterAlias(this);
}

void SysMLv2Parser::AliasContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitAlias(this);
}

SysMLv2Parser::AliasContext* SysMLv2Parser::alias() {
  AliasContext *_localctx = _tracker.createInstance<AliasContext>(_ctx, getState());
  enterRule(_localctx, 24, SysMLv2Parser::RuleAlias);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(200);
    match(SysMLv2Parser::ALIAS);
    setState(201);
    address();
    setState(202);
    match(SysMLv2Parser::FOR);
    setState(203);
    address();
    setState(204);
    match(SysMLv2Parser::DELIMITER);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- VisibilityContext ------------------------------------------------------------------

SysMLv2Parser::VisibilityContext::VisibilityContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysMLv2Parser::VisibilityContext::PRIVATE() {
  return getToken(SysMLv2Parser::PRIVATE, 0);
}

SysMLv2Parser::StartContext* SysMLv2Parser::VisibilityContext::start() {
  return getRuleContext<SysMLv2Parser::StartContext>(0);
}

tree::TerminalNode* SysMLv2Parser::VisibilityContext::PUBLIC() {
  return getToken(SysMLv2Parser::PUBLIC, 0);
}

tree::TerminalNode* SysMLv2Parser::VisibilityContext::PROTECTED() {
  return getToken(SysMLv2Parser::PROTECTED, 0);
}


size_t SysMLv2Parser::VisibilityContext::getRuleIndex() const {
  return SysMLv2Parser::RuleVisibility;
}

void SysMLv2Parser::VisibilityContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterVisibility(this);
}

void SysMLv2Parser::VisibilityContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitVisibility(this);
}

SysMLv2Parser::VisibilityContext* SysMLv2Parser::visibility() {
  VisibilityContext *_localctx = _tracker.createInstance<VisibilityContext>(_ctx, getState());
  enterRule(_localctx, 26, SysMLv2Parser::RuleVisibility);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(212);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysMLv2Parser::PRIVATE: {
        enterOuterAlt(_localctx, 1);
        setState(206);
        match(SysMLv2Parser::PRIVATE);
        setState(207);
        start();
        break;
      }

      case SysMLv2Parser::PUBLIC: {
        enterOuterAlt(_localctx, 2);
        setState(208);
        match(SysMLv2Parser::PUBLIC);
        setState(209);
        start();
        break;
      }

      case SysMLv2Parser::PROTECTED: {
        enterOuterAlt(_localctx, 3);
        setState(210);
        match(SysMLv2Parser::PROTECTED);
        setState(211);
        start();
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

//----------------- Import_ruleContext ------------------------------------------------------------------

SysMLv2Parser::Import_ruleContext::Import_ruleContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysMLv2Parser::Import_ruleContext::IMPORT() {
  return getToken(SysMLv2Parser::IMPORT, 0);
}

SysMLv2Parser::AddressContext* SysMLv2Parser::Import_ruleContext::address() {
  return getRuleContext<SysMLv2Parser::AddressContext>(0);
}

tree::TerminalNode* SysMLv2Parser::Import_ruleContext::DELIMITER() {
  return getToken(SysMLv2Parser::DELIMITER, 0);
}


size_t SysMLv2Parser::Import_ruleContext::getRuleIndex() const {
  return SysMLv2Parser::RuleImport_rule;
}

void SysMLv2Parser::Import_ruleContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterImport_rule(this);
}

void SysMLv2Parser::Import_ruleContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitImport_rule(this);
}

SysMLv2Parser::Import_ruleContext* SysMLv2Parser::import_rule() {
  Import_ruleContext *_localctx = _tracker.createInstance<Import_ruleContext>(_ctx, getState());
  enterRule(_localctx, 28, SysMLv2Parser::RuleImport_rule);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(214);
    match(SysMLv2Parser::IMPORT);
    setState(215);
    address();
    setState(216);
    match(SysMLv2Parser::DELIMITER);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AbstractionContext ------------------------------------------------------------------

SysMLv2Parser::AbstractionContext::AbstractionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysMLv2Parser::AbstractionContext::ABSTRACT() {
  return getToken(SysMLv2Parser::ABSTRACT, 0);
}

SysMLv2Parser::StartContext* SysMLv2Parser::AbstractionContext::start() {
  return getRuleContext<SysMLv2Parser::StartContext>(0);
}


size_t SysMLv2Parser::AbstractionContext::getRuleIndex() const {
  return SysMLv2Parser::RuleAbstraction;
}

void SysMLv2Parser::AbstractionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterAbstraction(this);
}

void SysMLv2Parser::AbstractionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitAbstraction(this);
}

SysMLv2Parser::AbstractionContext* SysMLv2Parser::abstraction() {
  AbstractionContext *_localctx = _tracker.createInstance<AbstractionContext>(_ctx, getState());
  enterRule(_localctx, 30, SysMLv2Parser::RuleAbstraction);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(218);
    match(SysMLv2Parser::ABSTRACT);
    setState(219);
    start();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- VariationContext ------------------------------------------------------------------

SysMLv2Parser::VariationContext::VariationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysMLv2Parser::VariationContext::VARIATION() {
  return getToken(SysMLv2Parser::VARIATION, 0);
}

SysMLv2Parser::StartContext* SysMLv2Parser::VariationContext::start() {
  return getRuleContext<SysMLv2Parser::StartContext>(0);
}


size_t SysMLv2Parser::VariationContext::getRuleIndex() const {
  return SysMLv2Parser::RuleVariation;
}

void SysMLv2Parser::VariationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterVariation(this);
}

void SysMLv2Parser::VariationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitVariation(this);
}

SysMLv2Parser::VariationContext* SysMLv2Parser::variation() {
  VariationContext *_localctx = _tracker.createInstance<VariationContext>(_ctx, getState());
  enterRule(_localctx, 32, SysMLv2Parser::RuleVariation);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(221);
    match(SysMLv2Parser::VARIATION);
    setState(222);
    start();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- VariantContext ------------------------------------------------------------------

SysMLv2Parser::VariantContext::VariantContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysMLv2Parser::VariantContext::VARIANT() {
  return getToken(SysMLv2Parser::VARIANT, 0);
}

SysMLv2Parser::PartContext* SysMLv2Parser::VariantContext::part() {
  return getRuleContext<SysMLv2Parser::PartContext>(0);
}


size_t SysMLv2Parser::VariantContext::getRuleIndex() const {
  return SysMLv2Parser::RuleVariant;
}

void SysMLv2Parser::VariantContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterVariant(this);
}

void SysMLv2Parser::VariantContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitVariant(this);
}

SysMLv2Parser::VariantContext* SysMLv2Parser::variant() {
  VariantContext *_localctx = _tracker.createInstance<VariantContext>(_ctx, getState());
  enterRule(_localctx, 34, SysMLv2Parser::RuleVariant);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(224);
    match(SysMLv2Parser::VARIANT);
    setState(225);
    part();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- FunctionContext ------------------------------------------------------------------

SysMLv2Parser::FunctionContext::FunctionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysMLv2Parser::FunctionContext::CALC() {
  return getToken(SysMLv2Parser::CALC, 0);
}

SysMLv2Parser::Definition_ruleContext* SysMLv2Parser::FunctionContext::definition_rule() {
  return getRuleContext<SysMLv2Parser::Definition_ruleContext>(0);
}

SysMLv2Parser::Bracketed_contentContext* SysMLv2Parser::FunctionContext::bracketed_content() {
  return getRuleContext<SysMLv2Parser::Bracketed_contentContext>(0);
}


size_t SysMLv2Parser::FunctionContext::getRuleIndex() const {
  return SysMLv2Parser::RuleFunction;
}

void SysMLv2Parser::FunctionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFunction(this);
}

void SysMLv2Parser::FunctionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFunction(this);
}

SysMLv2Parser::FunctionContext* SysMLv2Parser::function() {
  FunctionContext *_localctx = _tracker.createInstance<FunctionContext>(_ctx, getState());
  enterRule(_localctx, 36, SysMLv2Parser::RuleFunction);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(227);
    match(SysMLv2Parser::CALC);
    setState(228);
    definition_rule();
    setState(229);
    bracketed_content();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- InputContext ------------------------------------------------------------------

SysMLv2Parser::InputContext::InputContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysMLv2Parser::InputContext::IN() {
  return getToken(SysMLv2Parser::IN, 0);
}

tree::TerminalNode* SysMLv2Parser::InputContext::NAME() {
  return getToken(SysMLv2Parser::NAME, 0);
}

SysMLv2Parser::Type_definitionContext* SysMLv2Parser::InputContext::type_definition() {
  return getRuleContext<SysMLv2Parser::Type_definitionContext>(0);
}

tree::TerminalNode* SysMLv2Parser::InputContext::DELIMITER() {
  return getToken(SysMLv2Parser::DELIMITER, 0);
}


size_t SysMLv2Parser::InputContext::getRuleIndex() const {
  return SysMLv2Parser::RuleInput;
}

void SysMLv2Parser::InputContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterInput(this);
}

void SysMLv2Parser::InputContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitInput(this);
}

SysMLv2Parser::InputContext* SysMLv2Parser::input() {
  InputContext *_localctx = _tracker.createInstance<InputContext>(_ctx, getState());
  enterRule(_localctx, 38, SysMLv2Parser::RuleInput);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(231);
    match(SysMLv2Parser::IN);
    setState(232);
    match(SysMLv2Parser::NAME);
    setState(233);
    type_definition();
    setState(234);
    match(SysMLv2Parser::DELIMITER);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- OutputContext ------------------------------------------------------------------

SysMLv2Parser::OutputContext::OutputContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysMLv2Parser::OutputContext::OUT() {
  return getToken(SysMLv2Parser::OUT, 0);
}

tree::TerminalNode* SysMLv2Parser::OutputContext::NAME() {
  return getToken(SysMLv2Parser::NAME, 0);
}

SysMLv2Parser::Type_definitionContext* SysMLv2Parser::OutputContext::type_definition() {
  return getRuleContext<SysMLv2Parser::Type_definitionContext>(0);
}

tree::TerminalNode* SysMLv2Parser::OutputContext::DELIMITER() {
  return getToken(SysMLv2Parser::DELIMITER, 0);
}


size_t SysMLv2Parser::OutputContext::getRuleIndex() const {
  return SysMLv2Parser::RuleOutput;
}

void SysMLv2Parser::OutputContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterOutput(this);
}

void SysMLv2Parser::OutputContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitOutput(this);
}

SysMLv2Parser::OutputContext* SysMLv2Parser::output() {
  OutputContext *_localctx = _tracker.createInstance<OutputContext>(_ctx, getState());
  enterRule(_localctx, 40, SysMLv2Parser::RuleOutput);

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
    match(SysMLv2Parser::OUT);
    setState(237);
    match(SysMLv2Parser::NAME);
    setState(238);
    type_definition();
    setState(239);
    match(SysMLv2Parser::DELIMITER);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ReturnContext ------------------------------------------------------------------

SysMLv2Parser::ReturnContext::ReturnContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysMLv2Parser::ReturnContext::RETURN() {
  return getToken(SysMLv2Parser::RETURN, 0);
}

SysMLv2Parser::Type_definitionContext* SysMLv2Parser::ReturnContext::type_definition() {
  return getRuleContext<SysMLv2Parser::Type_definitionContext>(0);
}

tree::TerminalNode* SysMLv2Parser::ReturnContext::DELIMITER() {
  return getToken(SysMLv2Parser::DELIMITER, 0);
}


size_t SysMLv2Parser::ReturnContext::getRuleIndex() const {
  return SysMLv2Parser::RuleReturn;
}

void SysMLv2Parser::ReturnContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterReturn(this);
}

void SysMLv2Parser::ReturnContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitReturn(this);
}

SysMLv2Parser::ReturnContext* SysMLv2Parser::return_() {
  ReturnContext *_localctx = _tracker.createInstance<ReturnContext>(_ctx, getState());
  enterRule(_localctx, 42, SysMLv2Parser::RuleReturn);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(241);
    match(SysMLv2Parser::RETURN);
    setState(242);
    type_definition();
    setState(243);
    match(SysMLv2Parser::DELIMITER);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Command_definitionContext ------------------------------------------------------------------

SysMLv2Parser::Command_definitionContext::Command_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysMLv2Parser::Command_definitionContext::HASHTAG() {
  return getToken(SysMLv2Parser::HASHTAG, 0);
}

tree::TerminalNode* SysMLv2Parser::Command_definitionContext::COMMAND() {
  return getToken(SysMLv2Parser::COMMAND, 0);
}

tree::TerminalNode* SysMLv2Parser::Command_definitionContext::NAME() {
  return getToken(SysMLv2Parser::NAME, 0);
}

tree::TerminalNode* SysMLv2Parser::Command_definitionContext::ATTRIBUTE() {
  return getToken(SysMLv2Parser::ATTRIBUTE, 0);
}

tree::TerminalNode* SysMLv2Parser::Command_definitionContext::DELIMITER() {
  return getToken(SysMLv2Parser::DELIMITER, 0);
}


size_t SysMLv2Parser::Command_definitionContext::getRuleIndex() const {
  return SysMLv2Parser::RuleCommand_definition;
}

void SysMLv2Parser::Command_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterCommand_definition(this);
}

void SysMLv2Parser::Command_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitCommand_definition(this);
}

SysMLv2Parser::Command_definitionContext* SysMLv2Parser::command_definition() {
  Command_definitionContext *_localctx = _tracker.createInstance<Command_definitionContext>(_ctx, getState());
  enterRule(_localctx, 44, SysMLv2Parser::RuleCommand_definition);

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
    match(SysMLv2Parser::HASHTAG);
    setState(246);
    match(SysMLv2Parser::COMMAND);
    setState(247);
    match(SysMLv2Parser::NAME);
    setState(248);
    match(SysMLv2Parser::ATTRIBUTE);
    setState(249);
    match(SysMLv2Parser::DELIMITER);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ConnectToContext ------------------------------------------------------------------

SysMLv2Parser::ConnectToContext::ConnectToContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SysMLv2Parser::AddressContext *> SysMLv2Parser::ConnectToContext::address() {
  return getRuleContexts<SysMLv2Parser::AddressContext>();
}

SysMLv2Parser::AddressContext* SysMLv2Parser::ConnectToContext::address(size_t i) {
  return getRuleContext<SysMLv2Parser::AddressContext>(i);
}

tree::TerminalNode* SysMLv2Parser::ConnectToContext::CONNECT_TO() {
  return getToken(SysMLv2Parser::CONNECT_TO, 0);
}

tree::TerminalNode* SysMLv2Parser::ConnectToContext::DELIMITER() {
  return getToken(SysMLv2Parser::DELIMITER, 0);
}


size_t SysMLv2Parser::ConnectToContext::getRuleIndex() const {
  return SysMLv2Parser::RuleConnectTo;
}

void SysMLv2Parser::ConnectToContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterConnectTo(this);
}

void SysMLv2Parser::ConnectToContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitConnectTo(this);
}

SysMLv2Parser::ConnectToContext* SysMLv2Parser::connectTo() {
  ConnectToContext *_localctx = _tracker.createInstance<ConnectToContext>(_ctx, getState());
  enterRule(_localctx, 46, SysMLv2Parser::RuleConnectTo);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(251);
    address();
    setState(252);
    match(SysMLv2Parser::CONNECT_TO);
    setState(253);
    address();
    setState(254);
    match(SysMLv2Parser::DELIMITER);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Type_definitionContext ------------------------------------------------------------------

SysMLv2Parser::Type_definitionContext::Type_definitionContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysMLv2Parser::AddressContext* SysMLv2Parser::Type_definitionContext::address() {
  return getRuleContext<SysMLv2Parser::AddressContext>(0);
}


size_t SysMLv2Parser::Type_definitionContext::getRuleIndex() const {
  return SysMLv2Parser::RuleType_definition;
}

void SysMLv2Parser::Type_definitionContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterType_definition(this);
}

void SysMLv2Parser::Type_definitionContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitType_definition(this);
}

SysMLv2Parser::Type_definitionContext* SysMLv2Parser::type_definition() {
  Type_definitionContext *_localctx = _tracker.createInstance<Type_definitionContext>(_ctx, getState());
  enterRule(_localctx, 48, SysMLv2Parser::RuleType_definition);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(256);
    match(SysMLv2Parser::T__0);
    setState(257);
    address();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- AboutContext ------------------------------------------------------------------

SysMLv2Parser::AboutContext::AboutContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysMLv2Parser::AboutContext::ABOUT() {
  return getToken(SysMLv2Parser::ABOUT, 0);
}

std::vector<SysMLv2Parser::AddressContext *> SysMLv2Parser::AboutContext::address() {
  return getRuleContexts<SysMLv2Parser::AddressContext>();
}

SysMLv2Parser::AddressContext* SysMLv2Parser::AboutContext::address(size_t i) {
  return getRuleContext<SysMLv2Parser::AddressContext>(i);
}


size_t SysMLv2Parser::AboutContext::getRuleIndex() const {
  return SysMLv2Parser::RuleAbout;
}

void SysMLv2Parser::AboutContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterAbout(this);
}

void SysMLv2Parser::AboutContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitAbout(this);
}

SysMLv2Parser::AboutContext* SysMLv2Parser::about() {
  AboutContext *_localctx = _tracker.createInstance<AboutContext>(_ctx, getState());
  enterRule(_localctx, 50, SysMLv2Parser::RuleAbout);
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
    match(SysMLv2Parser::ABOUT);
    setState(260);
    address();
    setState(265);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysMLv2Parser::T__1) {
      setState(261);
      match(SysMLv2Parser::T__1);
      setState(262);
      address();
      setState(267);
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

//----------------- SpecilizationContext ------------------------------------------------------------------

SysMLv2Parser::SpecilizationContext::SpecilizationContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysMLv2Parser::AddressContext* SysMLv2Parser::SpecilizationContext::address() {
  return getRuleContext<SysMLv2Parser::AddressContext>(0);
}


size_t SysMLv2Parser::SpecilizationContext::getRuleIndex() const {
  return SysMLv2Parser::RuleSpecilization;
}

void SysMLv2Parser::SpecilizationContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterSpecilization(this);
}

void SysMLv2Parser::SpecilizationContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitSpecilization(this);
}

SysMLv2Parser::SpecilizationContext* SysMLv2Parser::specilization() {
  SpecilizationContext *_localctx = _tracker.createInstance<SpecilizationContext>(_ctx, getState());
  enterRule(_localctx, 52, SysMLv2Parser::RuleSpecilization);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(268);
    match(SysMLv2Parser::T__0);
    setState(269);
    address();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- DecriptorContext ------------------------------------------------------------------

SysMLv2Parser::DecriptorContext::DecriptorContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysMLv2Parser::DecriptorContext::LANGUAGE_DESCRIPTOR() {
  return getToken(SysMLv2Parser::LANGUAGE_DESCRIPTOR, 0);
}

tree::TerminalNode* SysMLv2Parser::DecriptorContext::NAME() {
  return getToken(SysMLv2Parser::NAME, 0);
}


size_t SysMLv2Parser::DecriptorContext::getRuleIndex() const {
  return SysMLv2Parser::RuleDecriptor;
}

void SysMLv2Parser::DecriptorContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterDecriptor(this);
}

void SysMLv2Parser::DecriptorContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitDecriptor(this);
}

SysMLv2Parser::DecriptorContext* SysMLv2Parser::decriptor() {
  DecriptorContext *_localctx = _tracker.createInstance<DecriptorContext>(_ctx, getState());
  enterRule(_localctx, 54, SysMLv2Parser::RuleDecriptor);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(271);
    match(SysMLv2Parser::LANGUAGE_DESCRIPTOR);
    setState(272);
    match(SysMLv2Parser::T__2);
    setState(273);
    match(SysMLv2Parser::NAME);
    setState(274);
    match(SysMLv2Parser::T__2);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- NamelistContext ------------------------------------------------------------------

SysMLv2Parser::NamelistContext::NamelistContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SysMLv2Parser::NameContext *> SysMLv2Parser::NamelistContext::name() {
  return getRuleContexts<SysMLv2Parser::NameContext>();
}

SysMLv2Parser::NameContext* SysMLv2Parser::NamelistContext::name(size_t i) {
  return getRuleContext<SysMLv2Parser::NameContext>(i);
}


size_t SysMLv2Parser::NamelistContext::getRuleIndex() const {
  return SysMLv2Parser::RuleNamelist;
}

void SysMLv2Parser::NamelistContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterNamelist(this);
}

void SysMLv2Parser::NamelistContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitNamelist(this);
}

SysMLv2Parser::NamelistContext* SysMLv2Parser::namelist() {
  NamelistContext *_localctx = _tracker.createInstance<NamelistContext>(_ctx, getState());
  enterRule(_localctx, 56, SysMLv2Parser::RuleNamelist);
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
    setState(276);
    name();
    setState(281);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysMLv2Parser::T__1) {
      setState(277);
      match(SysMLv2Parser::T__1);
      setState(278);
      name();
      setState(283);
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

//----------------- NameContext ------------------------------------------------------------------

SysMLv2Parser::NameContext::NameContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> SysMLv2Parser::NameContext::NAME() {
  return getTokens(SysMLv2Parser::NAME);
}

tree::TerminalNode* SysMLv2Parser::NameContext::NAME(size_t i) {
  return getToken(SysMLv2Parser::NAME, i);
}


size_t SysMLv2Parser::NameContext::getRuleIndex() const {
  return SysMLv2Parser::RuleName;
}

void SysMLv2Parser::NameContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterName(this);
}

void SysMLv2Parser::NameContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitName(this);
}

SysMLv2Parser::NameContext* SysMLv2Parser::name() {
  NameContext *_localctx = _tracker.createInstance<NameContext>(_ctx, getState());
  enterRule(_localctx, 58, SysMLv2Parser::RuleName);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(292);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysMLv2Parser::NAME: {
        enterOuterAlt(_localctx, 1);
        setState(284);
        match(SysMLv2Parser::NAME);
        break;
      }

      case SysMLv2Parser::T__3: {
        enterOuterAlt(_localctx, 2);
        setState(285);
        match(SysMLv2Parser::T__3);
        setState(287); 
        _errHandler->sync(this);
        _la = _input->LA(1);
        do {
          setState(286);
          match(SysMLv2Parser::NAME);
          setState(289); 
          _errHandler->sync(this);
          _la = _input->LA(1);
        } while (_la == SysMLv2Parser::NAME);
        setState(291);
        match(SysMLv2Parser::T__3);
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

//----------------- AddressContext ------------------------------------------------------------------

SysMLv2Parser::AddressContext::AddressContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<tree::TerminalNode *> SysMLv2Parser::AddressContext::NAME() {
  return getTokens(SysMLv2Parser::NAME);
}

tree::TerminalNode* SysMLv2Parser::AddressContext::NAME(size_t i) {
  return getToken(SysMLv2Parser::NAME, i);
}

std::vector<tree::TerminalNode *> SysMLv2Parser::AddressContext::STAR() {
  return getTokens(SysMLv2Parser::STAR);
}

tree::TerminalNode* SysMLv2Parser::AddressContext::STAR(size_t i) {
  return getToken(SysMLv2Parser::STAR, i);
}


size_t SysMLv2Parser::AddressContext::getRuleIndex() const {
  return SysMLv2Parser::RuleAddress;
}

void SysMLv2Parser::AddressContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterAddress(this);
}

void SysMLv2Parser::AddressContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitAddress(this);
}

SysMLv2Parser::AddressContext* SysMLv2Parser::address() {
  AddressContext *_localctx = _tracker.createInstance<AddressContext>(_ctx, getState());
  enterRule(_localctx, 60, SysMLv2Parser::RuleAddress);
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
    setState(294);
    match(SysMLv2Parser::NAME);
    setState(305);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysMLv2Parser::T__4) {
      setState(295);
      match(SysMLv2Parser::T__4);
      setState(301);
      _errHandler->sync(this);
      switch (_input->LA(1)) {
        case SysMLv2Parser::NAME: {
          setState(296);
          match(SysMLv2Parser::NAME);
          break;
        }

        case SysMLv2Parser::STAR: {
          setState(297);
          match(SysMLv2Parser::STAR);
          setState(299);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if (_la == SysMLv2Parser::STAR) {
            setState(298);
            match(SysMLv2Parser::STAR);
          }
          break;
        }

      default:
        throw NoViableAltException(this);
      }
      setState(307);
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

//----------------- Bracketed_contentContext ------------------------------------------------------------------

SysMLv2Parser::Bracketed_contentContext::Bracketed_contentContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SysMLv2Parser::ElemementsContext *> SysMLv2Parser::Bracketed_contentContext::elemements() {
  return getRuleContexts<SysMLv2Parser::ElemementsContext>();
}

SysMLv2Parser::ElemementsContext* SysMLv2Parser::Bracketed_contentContext::elemements(size_t i) {
  return getRuleContext<SysMLv2Parser::ElemementsContext>(i);
}


size_t SysMLv2Parser::Bracketed_contentContext::getRuleIndex() const {
  return SysMLv2Parser::RuleBracketed_content;
}

void SysMLv2Parser::Bracketed_contentContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterBracketed_content(this);
}

void SysMLv2Parser::Bracketed_contentContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitBracketed_content(this);
}

SysMLv2Parser::Bracketed_contentContext* SysMLv2Parser::bracketed_content() {
  Bracketed_contentContext *_localctx = _tracker.createInstance<Bracketed_contentContext>(_ctx, getState());
  enterRule(_localctx, 62, SysMLv2Parser::RuleBracketed_content);
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
    setState(308);
    match(SysMLv2Parser::T__5);
    setState(312);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while ((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 5343626210963456) != 0)) {
      setState(309);
      elemements();
      setState(314);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(315);
    match(SysMLv2Parser::T__6);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Fuction_argumentsContext ------------------------------------------------------------------

SysMLv2Parser::Fuction_argumentsContext::Fuction_argumentsContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

std::vector<SysMLv2Parser::ArgumentContext *> SysMLv2Parser::Fuction_argumentsContext::argument() {
  return getRuleContexts<SysMLv2Parser::ArgumentContext>();
}

SysMLv2Parser::ArgumentContext* SysMLv2Parser::Fuction_argumentsContext::argument(size_t i) {
  return getRuleContext<SysMLv2Parser::ArgumentContext>(i);
}


size_t SysMLv2Parser::Fuction_argumentsContext::getRuleIndex() const {
  return SysMLv2Parser::RuleFuction_arguments;
}

void SysMLv2Parser::Fuction_argumentsContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterFuction_arguments(this);
}

void SysMLv2Parser::Fuction_argumentsContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitFuction_arguments(this);
}

SysMLv2Parser::Fuction_argumentsContext* SysMLv2Parser::fuction_arguments() {
  Fuction_argumentsContext *_localctx = _tracker.createInstance<Fuction_argumentsContext>(_ctx, getState());
  enterRule(_localctx, 64, SysMLv2Parser::RuleFuction_arguments);
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
    setState(317);
    match(SysMLv2Parser::T__7);
    setState(319);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SysMLv2Parser::NAME) {
      setState(318);
      argument();
    }

    setState(321);
    match(SysMLv2Parser::T__1);
    setState(325);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysMLv2Parser::NAME) {
      setState(322);
      argument();
      setState(327);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(328);
    match(SysMLv2Parser::T__8);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- ArgumentContext ------------------------------------------------------------------

SysMLv2Parser::ArgumentContext::ArgumentContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysMLv2Parser::ArgumentContext::NAME() {
  return getToken(SysMLv2Parser::NAME, 0);
}

SysMLv2Parser::Type_definitionContext* SysMLv2Parser::ArgumentContext::type_definition() {
  return getRuleContext<SysMLv2Parser::Type_definitionContext>(0);
}


size_t SysMLv2Parser::ArgumentContext::getRuleIndex() const {
  return SysMLv2Parser::RuleArgument;
}

void SysMLv2Parser::ArgumentContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterArgument(this);
}

void SysMLv2Parser::ArgumentContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitArgument(this);
}

SysMLv2Parser::ArgumentContext* SysMLv2Parser::argument() {
  ArgumentContext *_localctx = _tracker.createInstance<ArgumentContext>(_ctx, getState());
  enterRule(_localctx, 66, SysMLv2Parser::RuleArgument);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(330);
    match(SysMLv2Parser::NAME);
    setState(331);
    type_definition();
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Delimiter_ruleContext ------------------------------------------------------------------

SysMLv2Parser::Delimiter_ruleContext::Delimiter_ruleContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

SysMLv2Parser::Bracketed_contentContext* SysMLv2Parser::Delimiter_ruleContext::bracketed_content() {
  return getRuleContext<SysMLv2Parser::Bracketed_contentContext>(0);
}

tree::TerminalNode* SysMLv2Parser::Delimiter_ruleContext::DELIMITER() {
  return getToken(SysMLv2Parser::DELIMITER, 0);
}


size_t SysMLv2Parser::Delimiter_ruleContext::getRuleIndex() const {
  return SysMLv2Parser::RuleDelimiter_rule;
}

void SysMLv2Parser::Delimiter_ruleContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterDelimiter_rule(this);
}

void SysMLv2Parser::Delimiter_ruleContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitDelimiter_rule(this);
}

SysMLv2Parser::Delimiter_ruleContext* SysMLv2Parser::delimiter_rule() {
  Delimiter_ruleContext *_localctx = _tracker.createInstance<Delimiter_ruleContext>(_ctx, getState());
  enterRule(_localctx, 68, SysMLv2Parser::RuleDelimiter_rule);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(335);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysMLv2Parser::T__5: {
        setState(333);
        bracketed_content();
        break;
      }

      case SysMLv2Parser::DELIMITER: {
        setState(334);
        match(SysMLv2Parser::DELIMITER);
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

//----------------- MultiplicityContext ------------------------------------------------------------------

SysMLv2Parser::MultiplicityContext::MultiplicityContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysMLv2Parser::MultiplicityContext::STAR() {
  return getToken(SysMLv2Parser::STAR, 0);
}

tree::TerminalNode* SysMLv2Parser::MultiplicityContext::NUMBER() {
  return getToken(SysMLv2Parser::NUMBER, 0);
}


size_t SysMLv2Parser::MultiplicityContext::getRuleIndex() const {
  return SysMLv2Parser::RuleMultiplicity;
}

void SysMLv2Parser::MultiplicityContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterMultiplicity(this);
}

void SysMLv2Parser::MultiplicityContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitMultiplicity(this);
}

SysMLv2Parser::MultiplicityContext* SysMLv2Parser::multiplicity() {
  MultiplicityContext *_localctx = _tracker.createInstance<MultiplicityContext>(_ctx, getState());
  enterRule(_localctx, 70, SysMLv2Parser::RuleMultiplicity);
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
    setState(337);
    match(SysMLv2Parser::T__9);
    setState(338);
    _la = _input->LA(1);
    if (!(_la == SysMLv2Parser::STAR

    || _la == SysMLv2Parser::NUMBER)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
    setState(339);
    match(SysMLv2Parser::T__10);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- UnitContext ------------------------------------------------------------------

SysMLv2Parser::UnitContext::UnitContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysMLv2Parser::UnitContext::NAME() {
  return getToken(SysMLv2Parser::NAME, 0);
}


size_t SysMLv2Parser::UnitContext::getRuleIndex() const {
  return SysMLv2Parser::RuleUnit;
}

void SysMLv2Parser::UnitContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterUnit(this);
}

void SysMLv2Parser::UnitContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitUnit(this);
}

SysMLv2Parser::UnitContext* SysMLv2Parser::unit() {
  UnitContext *_localctx = _tracker.createInstance<UnitContext>(_ctx, getState());
  enterRule(_localctx, 72, SysMLv2Parser::RuleUnit);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(341);
    match(SysMLv2Parser::T__9);
    setState(342);
    match(SysMLv2Parser::NAME);
    setState(343);
    match(SysMLv2Parser::T__10);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

//----------------- Definition_ruleContext ------------------------------------------------------------------

SysMLv2Parser::Definition_ruleContext::Definition_ruleContext(ParserRuleContext *parent, size_t invokingState)
  : ParserRuleContext(parent, invokingState) {
}

tree::TerminalNode* SysMLv2Parser::Definition_ruleContext::DEFINITION() {
  return getToken(SysMLv2Parser::DEFINITION, 0);
}

tree::TerminalNode* SysMLv2Parser::Definition_ruleContext::NAME() {
  return getToken(SysMLv2Parser::NAME, 0);
}


size_t SysMLv2Parser::Definition_ruleContext::getRuleIndex() const {
  return SysMLv2Parser::RuleDefinition_rule;
}

void SysMLv2Parser::Definition_ruleContext::enterRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->enterDefinition_rule(this);
}

void SysMLv2Parser::Definition_ruleContext::exitRule(tree::ParseTreeListener *listener) {
  auto parserListener = dynamic_cast<SysMLv2Listener *>(listener);
  if (parserListener != nullptr)
    parserListener->exitDefinition_rule(this);
}

SysMLv2Parser::Definition_ruleContext* SysMLv2Parser::definition_rule() {
  Definition_ruleContext *_localctx = _tracker.createInstance<Definition_ruleContext>(_ctx, getState());
  enterRule(_localctx, 74, SysMLv2Parser::RuleDefinition_rule);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(345);
    match(SysMLv2Parser::DEFINITION);
    setState(346);
    match(SysMLv2Parser::NAME);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

void SysMLv2Parser::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  sysmlv2ParserInitialize();
#else
  ::antlr4::internal::call_once(sysmlv2ParserOnceFlag, sysmlv2ParserInitialize);
#endif
}
