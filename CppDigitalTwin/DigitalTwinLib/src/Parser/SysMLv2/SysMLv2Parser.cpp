
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
      "start", "dependency", "textual_representaion", "comment", "part", 
      "port", "attribute", "item", "package", "assertion", "actions", "alias", 
      "visibility", "import_rule", "abstraction", "variation", "variant", 
      "type_definition", "about", "specilization", "decriptor", "namelist", 
      "name", "address", "bracketed_content", "fuction_arguments", "argument", 
      "delimiter_rule", "multiplicity", "unit"
    },
    std::vector<std::string>{
      "", "':'", "','", "'\"'", "'''", "'::'", "'{'", "'}'", "'('", "')'", 
      "'['", "']'", "'dependency'", "'rep'", "'language'", "'doc'", "'comment'", 
      "'about'", "'part'", "", "", "'to'", "'from'", "'for'", "';'", "'port'", 
      "'item'", "'package'", "'def'", "'attribute'", "'measruable'", "'controllable'", 
      "'assert'", "'action'", "'alias'", "'private'", "'protected'", "'public'", 
      "'import'", "'abstract'", "'variation'", "'variant'", "'*'"
    },
    std::vector<std::string>{
      "", "", "", "", "", "", "", "", "", "", "", "", "DEPENDENCY", "REPRESENTATION", 
      "LANGUAGE_DESCRIPTOR", "DOCUMENTATION", "COMMENT", "ABOUT", "PART", 
      "BLOCK_COMMENT", "LINE_COMMENT", "TO", "FROM", "FOR", "DELIMITER", 
      "PORT", "ITEM", "PACKAGE", "DEFINITION", "ATTRIBUTE", "MEASURABLE", 
      "CONTROLLABLE", "ASSERT", "ACTION", "ALIAS", "PRIVATE", "PROTECTED", 
      "PUBLIC", "IMPORT", "ABSTRACT", "VARIATION", "VARIANT", "STAR", "NUMBER", 
      "NAME", "INT", "WS"
    }
  );
  static const int32_t serializedATNSegment[] = {
  	4,1,46,292,2,0,7,0,2,1,7,1,2,2,7,2,2,3,7,3,2,4,7,4,2,5,7,5,2,6,7,6,2,
  	7,7,7,2,8,7,8,2,9,7,9,2,10,7,10,2,11,7,11,2,12,7,12,2,13,7,13,2,14,7,
  	14,2,15,7,15,2,16,7,16,2,17,7,17,2,18,7,18,2,19,7,19,2,20,7,20,2,21,7,
  	21,2,22,7,22,2,23,7,23,2,24,7,24,2,25,7,25,2,26,7,26,2,27,7,27,2,28,7,
  	28,2,29,7,29,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,
  	1,0,1,0,1,0,3,0,78,8,0,1,0,1,0,5,0,82,8,0,10,0,12,0,85,9,0,1,1,1,1,1,
  	1,1,1,3,1,91,8,1,1,1,1,1,1,1,1,1,1,2,3,2,98,8,2,1,2,3,2,101,8,2,1,2,1,
  	2,1,2,1,3,1,3,1,3,3,3,109,8,3,1,3,3,3,112,8,3,1,3,1,3,1,3,3,3,117,8,3,
  	1,3,1,3,3,3,121,8,3,1,4,1,4,3,4,125,8,4,1,4,1,4,3,4,129,8,4,1,4,1,4,1,
  	5,1,5,3,5,135,8,5,1,5,1,5,3,5,139,8,5,1,5,1,5,1,6,1,6,3,6,145,8,6,1,6,
  	1,6,3,6,149,8,6,1,6,3,6,152,8,6,1,6,3,6,155,8,6,1,6,1,6,1,7,1,7,1,7,1,
  	7,1,8,1,8,1,8,1,8,1,9,1,9,1,9,1,9,1,10,1,10,3,10,173,8,10,1,10,3,10,176,
  	8,10,1,10,1,10,1,10,1,11,1,11,1,11,1,11,1,11,1,11,1,12,1,12,1,12,1,12,
  	1,12,1,12,3,12,193,8,12,1,13,1,13,1,13,1,13,1,14,1,14,1,14,1,15,1,15,
  	1,15,1,16,1,16,1,16,1,17,1,17,1,17,1,18,1,18,1,18,1,18,5,18,215,8,18,
  	10,18,12,18,218,9,18,1,19,1,19,1,19,1,20,1,20,1,20,1,20,1,20,1,21,1,21,
  	1,21,5,21,231,8,21,10,21,12,21,234,9,21,1,22,1,22,1,22,4,22,239,8,22,
  	11,22,12,22,240,1,22,3,22,244,8,22,1,23,1,23,1,23,1,23,1,23,3,23,251,
  	8,23,3,23,253,8,23,5,23,255,8,23,10,23,12,23,258,9,23,1,24,1,24,1,24,
  	1,24,1,25,1,25,3,25,266,8,25,1,25,1,25,5,25,270,8,25,10,25,12,25,273,
  	9,25,1,25,1,25,1,26,1,26,1,26,1,27,1,27,3,27,282,8,27,1,28,1,28,1,28,
  	1,28,1,29,1,29,1,29,1,29,1,29,0,1,0,30,0,2,4,6,8,10,12,14,16,18,20,22,
  	24,26,28,30,32,34,36,38,40,42,44,46,48,50,52,54,56,58,0,2,1,0,29,31,1,
  	0,42,43,308,0,77,1,0,0,0,2,86,1,0,0,0,4,97,1,0,0,0,6,120,1,0,0,0,8,122,
  	1,0,0,0,10,132,1,0,0,0,12,142,1,0,0,0,14,158,1,0,0,0,16,162,1,0,0,0,18,
  	166,1,0,0,0,20,170,1,0,0,0,22,180,1,0,0,0,24,192,1,0,0,0,26,194,1,0,0,
  	0,28,198,1,0,0,0,30,201,1,0,0,0,32,204,1,0,0,0,34,207,1,0,0,0,36,210,
  	1,0,0,0,38,219,1,0,0,0,40,222,1,0,0,0,42,227,1,0,0,0,44,243,1,0,0,0,46,
  	245,1,0,0,0,48,259,1,0,0,0,50,263,1,0,0,0,52,276,1,0,0,0,54,281,1,0,0,
  	0,56,283,1,0,0,0,58,287,1,0,0,0,60,61,6,0,-1,0,61,78,3,2,1,0,62,78,3,
  	4,2,0,63,78,3,6,3,0,64,78,3,8,4,0,65,78,3,10,5,0,66,78,3,14,7,0,67,78,
  	3,16,8,0,68,78,3,12,6,0,69,78,3,18,9,0,70,78,3,20,10,0,71,78,3,22,11,
  	0,72,78,3,24,12,0,73,78,3,26,13,0,74,78,3,28,14,0,75,78,3,30,15,0,76,
  	78,3,32,16,0,77,60,1,0,0,0,77,62,1,0,0,0,77,63,1,0,0,0,77,64,1,0,0,0,
  	77,65,1,0,0,0,77,66,1,0,0,0,77,67,1,0,0,0,77,68,1,0,0,0,77,69,1,0,0,0,
  	77,70,1,0,0,0,77,71,1,0,0,0,77,72,1,0,0,0,77,73,1,0,0,0,77,74,1,0,0,0,
  	77,75,1,0,0,0,77,76,1,0,0,0,78,83,1,0,0,0,79,80,10,17,0,0,80,82,3,0,0,
  	18,81,79,1,0,0,0,82,85,1,0,0,0,83,81,1,0,0,0,83,84,1,0,0,0,84,1,1,0,0,
  	0,85,83,1,0,0,0,86,87,5,12,0,0,87,90,3,42,21,0,88,89,5,22,0,0,89,91,3,
  	42,21,0,90,88,1,0,0,0,90,91,1,0,0,0,91,92,1,0,0,0,92,93,5,21,0,0,93,94,
  	3,42,21,0,94,95,5,24,0,0,95,3,1,0,0,0,96,98,5,13,0,0,97,96,1,0,0,0,97,
  	98,1,0,0,0,98,100,1,0,0,0,99,101,5,44,0,0,100,99,1,0,0,0,100,101,1,0,
  	0,0,101,102,1,0,0,0,102,103,3,40,20,0,103,104,5,19,0,0,104,5,1,0,0,0,
  	105,121,5,19,0,0,106,108,5,16,0,0,107,109,5,44,0,0,108,107,1,0,0,0,108,
  	109,1,0,0,0,109,111,1,0,0,0,110,112,3,36,18,0,111,110,1,0,0,0,111,112,
  	1,0,0,0,112,113,1,0,0,0,113,121,5,19,0,0,114,116,5,15,0,0,115,117,5,44,
  	0,0,116,115,1,0,0,0,116,117,1,0,0,0,117,118,1,0,0,0,118,121,5,19,0,0,
  	119,121,5,20,0,0,120,105,1,0,0,0,120,106,1,0,0,0,120,114,1,0,0,0,120,
  	119,1,0,0,0,121,7,1,0,0,0,122,124,5,18,0,0,123,125,5,28,0,0,124,123,1,
  	0,0,0,124,125,1,0,0,0,125,126,1,0,0,0,126,128,5,44,0,0,127,129,3,38,19,
  	0,128,127,1,0,0,0,128,129,1,0,0,0,129,130,1,0,0,0,130,131,3,54,27,0,131,
  	9,1,0,0,0,132,134,5,25,0,0,133,135,5,28,0,0,134,133,1,0,0,0,134,135,1,
  	0,0,0,135,136,1,0,0,0,136,138,5,44,0,0,137,139,3,38,19,0,138,137,1,0,
  	0,0,138,139,1,0,0,0,139,140,1,0,0,0,140,141,3,54,27,0,141,11,1,0,0,0,
  	142,144,7,0,0,0,143,145,5,28,0,0,144,143,1,0,0,0,144,145,1,0,0,0,145,
  	146,1,0,0,0,146,148,5,44,0,0,147,149,3,56,28,0,148,147,1,0,0,0,148,149,
  	1,0,0,0,149,151,1,0,0,0,150,152,3,34,17,0,151,150,1,0,0,0,151,152,1,0,
  	0,0,152,154,1,0,0,0,153,155,3,58,29,0,154,153,1,0,0,0,154,155,1,0,0,0,
  	155,156,1,0,0,0,156,157,3,54,27,0,157,13,1,0,0,0,158,159,5,26,0,0,159,
  	160,5,44,0,0,160,161,3,54,27,0,161,15,1,0,0,0,162,163,5,27,0,0,163,164,
  	5,44,0,0,164,165,3,54,27,0,165,17,1,0,0,0,166,167,5,32,0,0,167,168,5,
  	44,0,0,168,169,3,54,27,0,169,19,1,0,0,0,170,172,5,33,0,0,171,173,5,28,
  	0,0,172,171,1,0,0,0,172,173,1,0,0,0,173,175,1,0,0,0,174,176,5,44,0,0,
  	175,174,1,0,0,0,175,176,1,0,0,0,176,177,1,0,0,0,177,178,3,50,25,0,178,
  	179,3,48,24,0,179,21,1,0,0,0,180,181,5,34,0,0,181,182,3,46,23,0,182,183,
  	5,23,0,0,183,184,3,46,23,0,184,185,5,24,0,0,185,23,1,0,0,0,186,187,5,
  	35,0,0,187,193,3,0,0,0,188,189,5,37,0,0,189,193,3,0,0,0,190,191,5,36,
  	0,0,191,193,3,0,0,0,192,186,1,0,0,0,192,188,1,0,0,0,192,190,1,0,0,0,193,
  	25,1,0,0,0,194,195,5,38,0,0,195,196,3,46,23,0,196,197,5,24,0,0,197,27,
  	1,0,0,0,198,199,5,39,0,0,199,200,3,0,0,0,200,29,1,0,0,0,201,202,5,40,
  	0,0,202,203,3,0,0,0,203,31,1,0,0,0,204,205,5,41,0,0,205,206,3,8,4,0,206,
  	33,1,0,0,0,207,208,5,1,0,0,208,209,3,46,23,0,209,35,1,0,0,0,210,211,5,
  	17,0,0,211,216,3,46,23,0,212,213,5,2,0,0,213,215,3,46,23,0,214,212,1,
  	0,0,0,215,218,1,0,0,0,216,214,1,0,0,0,216,217,1,0,0,0,217,37,1,0,0,0,
  	218,216,1,0,0,0,219,220,5,1,0,0,220,221,3,46,23,0,221,39,1,0,0,0,222,
  	223,5,14,0,0,223,224,5,3,0,0,224,225,5,44,0,0,225,226,5,3,0,0,226,41,
  	1,0,0,0,227,232,3,44,22,0,228,229,5,2,0,0,229,231,3,44,22,0,230,228,1,
  	0,0,0,231,234,1,0,0,0,232,230,1,0,0,0,232,233,1,0,0,0,233,43,1,0,0,0,
  	234,232,1,0,0,0,235,244,5,44,0,0,236,238,5,4,0,0,237,239,5,44,0,0,238,
  	237,1,0,0,0,239,240,1,0,0,0,240,238,1,0,0,0,240,241,1,0,0,0,241,242,1,
  	0,0,0,242,244,5,4,0,0,243,235,1,0,0,0,243,236,1,0,0,0,244,45,1,0,0,0,
  	245,256,5,44,0,0,246,252,5,5,0,0,247,253,5,44,0,0,248,250,5,42,0,0,249,
  	251,5,42,0,0,250,249,1,0,0,0,250,251,1,0,0,0,251,253,1,0,0,0,252,247,
  	1,0,0,0,252,248,1,0,0,0,253,255,1,0,0,0,254,246,1,0,0,0,255,258,1,0,0,
  	0,256,254,1,0,0,0,256,257,1,0,0,0,257,47,1,0,0,0,258,256,1,0,0,0,259,
  	260,5,6,0,0,260,261,3,0,0,0,261,262,5,7,0,0,262,49,1,0,0,0,263,265,5,
  	8,0,0,264,266,3,52,26,0,265,264,1,0,0,0,265,266,1,0,0,0,266,267,1,0,0,
  	0,267,271,5,2,0,0,268,270,3,52,26,0,269,268,1,0,0,0,270,273,1,0,0,0,271,
  	269,1,0,0,0,271,272,1,0,0,0,272,274,1,0,0,0,273,271,1,0,0,0,274,275,5,
  	9,0,0,275,51,1,0,0,0,276,277,5,44,0,0,277,278,3,34,17,0,278,53,1,0,0,
  	0,279,282,3,48,24,0,280,282,5,24,0,0,281,279,1,0,0,0,281,280,1,0,0,0,
  	282,55,1,0,0,0,283,284,5,10,0,0,284,285,7,1,0,0,285,286,5,11,0,0,286,
  	57,1,0,0,0,287,288,5,10,0,0,288,289,5,44,0,0,289,290,5,11,0,0,290,59,
  	1,0,0,0,30,77,83,90,97,100,108,111,116,120,124,128,134,138,144,148,151,
  	154,172,175,192,216,232,240,243,250,252,256,265,271,281
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

SysMLv2Parser::DependencyContext* SysMLv2Parser::StartContext::dependency() {
  return getRuleContext<SysMLv2Parser::DependencyContext>(0);
}

SysMLv2Parser::Textual_representaionContext* SysMLv2Parser::StartContext::textual_representaion() {
  return getRuleContext<SysMLv2Parser::Textual_representaionContext>(0);
}

SysMLv2Parser::CommentContext* SysMLv2Parser::StartContext::comment() {
  return getRuleContext<SysMLv2Parser::CommentContext>(0);
}

SysMLv2Parser::PartContext* SysMLv2Parser::StartContext::part() {
  return getRuleContext<SysMLv2Parser::PartContext>(0);
}

SysMLv2Parser::PortContext* SysMLv2Parser::StartContext::port() {
  return getRuleContext<SysMLv2Parser::PortContext>(0);
}

SysMLv2Parser::ItemContext* SysMLv2Parser::StartContext::item() {
  return getRuleContext<SysMLv2Parser::ItemContext>(0);
}

SysMLv2Parser::PackageContext* SysMLv2Parser::StartContext::package() {
  return getRuleContext<SysMLv2Parser::PackageContext>(0);
}

SysMLv2Parser::AttributeContext* SysMLv2Parser::StartContext::attribute() {
  return getRuleContext<SysMLv2Parser::AttributeContext>(0);
}

SysMLv2Parser::AssertionContext* SysMLv2Parser::StartContext::assertion() {
  return getRuleContext<SysMLv2Parser::AssertionContext>(0);
}

SysMLv2Parser::ActionsContext* SysMLv2Parser::StartContext::actions() {
  return getRuleContext<SysMLv2Parser::ActionsContext>(0);
}

SysMLv2Parser::AliasContext* SysMLv2Parser::StartContext::alias() {
  return getRuleContext<SysMLv2Parser::AliasContext>(0);
}

SysMLv2Parser::VisibilityContext* SysMLv2Parser::StartContext::visibility() {
  return getRuleContext<SysMLv2Parser::VisibilityContext>(0);
}

SysMLv2Parser::Import_ruleContext* SysMLv2Parser::StartContext::import_rule() {
  return getRuleContext<SysMLv2Parser::Import_ruleContext>(0);
}

SysMLv2Parser::AbstractionContext* SysMLv2Parser::StartContext::abstraction() {
  return getRuleContext<SysMLv2Parser::AbstractionContext>(0);
}

SysMLv2Parser::VariationContext* SysMLv2Parser::StartContext::variation() {
  return getRuleContext<SysMLv2Parser::VariationContext>(0);
}

SysMLv2Parser::VariantContext* SysMLv2Parser::StartContext::variant() {
  return getRuleContext<SysMLv2Parser::VariantContext>(0);
}

std::vector<SysMLv2Parser::StartContext *> SysMLv2Parser::StartContext::start() {
  return getRuleContexts<SysMLv2Parser::StartContext>();
}

SysMLv2Parser::StartContext* SysMLv2Parser::StartContext::start(size_t i) {
  return getRuleContext<SysMLv2Parser::StartContext>(i);
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
   return start(0);
}

SysMLv2Parser::StartContext* SysMLv2Parser::start(int precedence) {
  ParserRuleContext *parentContext = _ctx;
  size_t parentState = getState();
  SysMLv2Parser::StartContext *_localctx = _tracker.createInstance<StartContext>(_ctx, parentState);
  SysMLv2Parser::StartContext *previousContext = _localctx;
  (void)previousContext; // Silence compiler, in case the context is not used by generated code.
  size_t startState = 0;
  enterRecursionRule(_localctx, 0, SysMLv2Parser::RuleStart, precedence);

    

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
    setState(77);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysMLv2Parser::DEPENDENCY: {
        setState(61);
        dependency();
        break;
      }

      case SysMLv2Parser::REPRESENTATION:
      case SysMLv2Parser::LANGUAGE_DESCRIPTOR:
      case SysMLv2Parser::NAME: {
        setState(62);
        textual_representaion();
        break;
      }

      case SysMLv2Parser::DOCUMENTATION:
      case SysMLv2Parser::COMMENT:
      case SysMLv2Parser::BLOCK_COMMENT:
      case SysMLv2Parser::LINE_COMMENT: {
        setState(63);
        comment();
        break;
      }

      case SysMLv2Parser::PART: {
        setState(64);
        part();
        break;
      }

      case SysMLv2Parser::PORT: {
        setState(65);
        port();
        break;
      }

      case SysMLv2Parser::ITEM: {
        setState(66);
        item();
        break;
      }

      case SysMLv2Parser::PACKAGE: {
        setState(67);
        package();
        break;
      }

      case SysMLv2Parser::ATTRIBUTE:
      case SysMLv2Parser::MEASURABLE:
      case SysMLv2Parser::CONTROLLABLE: {
        setState(68);
        attribute();
        break;
      }

      case SysMLv2Parser::ASSERT: {
        setState(69);
        assertion();
        break;
      }

      case SysMLv2Parser::ACTION: {
        setState(70);
        actions();
        break;
      }

      case SysMLv2Parser::ALIAS: {
        setState(71);
        alias();
        break;
      }

      case SysMLv2Parser::PRIVATE:
      case SysMLv2Parser::PROTECTED:
      case SysMLv2Parser::PUBLIC: {
        setState(72);
        visibility();
        break;
      }

      case SysMLv2Parser::IMPORT: {
        setState(73);
        import_rule();
        break;
      }

      case SysMLv2Parser::ABSTRACT: {
        setState(74);
        abstraction();
        break;
      }

      case SysMLv2Parser::VARIATION: {
        setState(75);
        variation();
        break;
      }

      case SysMLv2Parser::VARIANT: {
        setState(76);
        variant();
        break;
      }

    default:
      throw NoViableAltException(this);
    }
    _ctx->stop = _input->LT(-1);
    setState(83);
    _errHandler->sync(this);
    alt = getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 1, _ctx);
    while (alt != 2 && alt != atn::ATN::INVALID_ALT_NUMBER) {
      if (alt == 1) {
        if (!_parseListeners.empty())
          triggerExitRuleEvent();
        previousContext = _localctx;
        _localctx = _tracker.createInstance<StartContext>(parentContext, parentState);
        pushNewRecursionContext(_localctx, startState, RuleStart);
        setState(79);

        if (!(precpred(_ctx, 17))) throw FailedPredicateException(this, "precpred(_ctx, 17)");
        setState(80);
        start(18); 
      }
      setState(85);
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
  enterRule(_localctx, 2, SysMLv2Parser::RuleDependency);
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
    setState(86);
    match(SysMLv2Parser::DEPENDENCY);
    setState(87);
    namelist();
    setState(90);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SysMLv2Parser::FROM) {
      setState(88);
      match(SysMLv2Parser::FROM);
      setState(89);
      namelist();
    }
    setState(92);
    match(SysMLv2Parser::TO);
    setState(93);
    namelist();
    setState(94);
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
  enterRule(_localctx, 4, SysMLv2Parser::RuleTextual_representaion);
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
    setState(97);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SysMLv2Parser::REPRESENTATION) {
      setState(96);
      match(SysMLv2Parser::REPRESENTATION);
    }
    setState(100);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SysMLv2Parser::NAME) {
      setState(99);
      match(SysMLv2Parser::NAME);
    }
    setState(102);
    decriptor();
    setState(103);
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
  enterRule(_localctx, 6, SysMLv2Parser::RuleComment);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(120);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysMLv2Parser::BLOCK_COMMENT: {
        enterOuterAlt(_localctx, 1);
        setState(105);
        match(SysMLv2Parser::BLOCK_COMMENT);
        break;
      }

      case SysMLv2Parser::COMMENT: {
        enterOuterAlt(_localctx, 2);
        setState(106);
        match(SysMLv2Parser::COMMENT);
        setState(108);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == SysMLv2Parser::NAME) {
          setState(107);
          match(SysMLv2Parser::NAME);
        }
        setState(111);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == SysMLv2Parser::ABOUT) {
          setState(110);
          about();
        }
        setState(113);
        match(SysMLv2Parser::BLOCK_COMMENT);
        break;
      }

      case SysMLv2Parser::DOCUMENTATION: {
        enterOuterAlt(_localctx, 3);
        setState(114);
        match(SysMLv2Parser::DOCUMENTATION);
        setState(116);
        _errHandler->sync(this);

        _la = _input->LA(1);
        if (_la == SysMLv2Parser::NAME) {
          setState(115);
          match(SysMLv2Parser::NAME);
        }
        setState(118);
        match(SysMLv2Parser::BLOCK_COMMENT);
        break;
      }

      case SysMLv2Parser::LINE_COMMENT: {
        enterOuterAlt(_localctx, 4);
        setState(119);
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
  enterRule(_localctx, 8, SysMLv2Parser::RulePart);
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
    setState(122);
    match(SysMLv2Parser::PART);
    setState(124);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SysMLv2Parser::DEFINITION) {
      setState(123);
      match(SysMLv2Parser::DEFINITION);
    }
    setState(126);
    match(SysMLv2Parser::NAME);
    setState(128);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SysMLv2Parser::T__0) {
      setState(127);
      specilization();
    }
    setState(130);
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
  enterRule(_localctx, 10, SysMLv2Parser::RulePort);
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
    setState(132);
    match(SysMLv2Parser::PORT);
    setState(134);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SysMLv2Parser::DEFINITION) {
      setState(133);
      match(SysMLv2Parser::DEFINITION);
    }
    setState(136);
    match(SysMLv2Parser::NAME);
    setState(138);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SysMLv2Parser::T__0) {
      setState(137);
      specilization();
    }
    setState(140);
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
  enterRule(_localctx, 12, SysMLv2Parser::RuleAttribute);
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
    _la = _input->LA(1);
    if (!((((_la & ~ 0x3fULL) == 0) &&
      ((1ULL << _la) & 3758096384) != 0))) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
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

    switch (getInterpreter<atn::ParserATNSimulator>()->adaptivePredict(_input, 14, _ctx)) {
    case 1: {
      setState(147);
      multiplicity();
      break;
    }

    default:
      break;
    }
    setState(151);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SysMLv2Parser::T__0) {
      setState(150);
      type_definition();
    }
    setState(154);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SysMLv2Parser::T__9) {
      setState(153);
      unit();
    }
    setState(156);
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
  enterRule(_localctx, 14, SysMLv2Parser::RuleItem);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(158);
    match(SysMLv2Parser::ITEM);
    setState(159);
    match(SysMLv2Parser::NAME);
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
  enterRule(_localctx, 16, SysMLv2Parser::RulePackage);

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
    match(SysMLv2Parser::PACKAGE);
    setState(163);
    match(SysMLv2Parser::NAME);
    setState(164);
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
  enterRule(_localctx, 18, SysMLv2Parser::RuleAssertion);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(166);
    match(SysMLv2Parser::ASSERT);
    setState(167);
    match(SysMLv2Parser::NAME);
    setState(168);
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
  enterRule(_localctx, 20, SysMLv2Parser::RuleActions);
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
    setState(170);
    match(SysMLv2Parser::ACTION);
    setState(172);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SysMLv2Parser::DEFINITION) {
      setState(171);
      match(SysMLv2Parser::DEFINITION);
    }
    setState(175);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SysMLv2Parser::NAME) {
      setState(174);
      match(SysMLv2Parser::NAME);
    }
    setState(177);
    fuction_arguments();
    setState(178);
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
  enterRule(_localctx, 22, SysMLv2Parser::RuleAlias);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(180);
    match(SysMLv2Parser::ALIAS);
    setState(181);
    address();
    setState(182);
    match(SysMLv2Parser::FOR);
    setState(183);
    address();
    setState(184);
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
  enterRule(_localctx, 24, SysMLv2Parser::RuleVisibility);

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
      case SysMLv2Parser::PRIVATE: {
        enterOuterAlt(_localctx, 1);
        setState(186);
        match(SysMLv2Parser::PRIVATE);
        setState(187);
        start(0);
        break;
      }

      case SysMLv2Parser::PUBLIC: {
        enterOuterAlt(_localctx, 2);
        setState(188);
        match(SysMLv2Parser::PUBLIC);
        setState(189);
        start(0);
        break;
      }

      case SysMLv2Parser::PROTECTED: {
        enterOuterAlt(_localctx, 3);
        setState(190);
        match(SysMLv2Parser::PROTECTED);
        setState(191);
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
  enterRule(_localctx, 26, SysMLv2Parser::RuleImport_rule);

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
    match(SysMLv2Parser::IMPORT);
    setState(195);
    address();
    setState(196);
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
  enterRule(_localctx, 28, SysMLv2Parser::RuleAbstraction);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(198);
    match(SysMLv2Parser::ABSTRACT);
    setState(199);
    start(0);
   
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
  enterRule(_localctx, 30, SysMLv2Parser::RuleVariation);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(201);
    match(SysMLv2Parser::VARIATION);
    setState(202);
    start(0);
   
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
  enterRule(_localctx, 32, SysMLv2Parser::RuleVariant);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(204);
    match(SysMLv2Parser::VARIANT);
    setState(205);
    part();
   
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
  enterRule(_localctx, 34, SysMLv2Parser::RuleType_definition);

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
    match(SysMLv2Parser::T__0);
    setState(208);
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
  enterRule(_localctx, 36, SysMLv2Parser::RuleAbout);
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
    setState(210);
    match(SysMLv2Parser::ABOUT);
    setState(211);
    address();
    setState(216);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysMLv2Parser::T__1) {
      setState(212);
      match(SysMLv2Parser::T__1);
      setState(213);
      address();
      setState(218);
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
  enterRule(_localctx, 38, SysMLv2Parser::RuleSpecilization);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(219);
    match(SysMLv2Parser::T__0);
    setState(220);
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
  enterRule(_localctx, 40, SysMLv2Parser::RuleDecriptor);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(222);
    match(SysMLv2Parser::LANGUAGE_DESCRIPTOR);
    setState(223);
    match(SysMLv2Parser::T__2);
    setState(224);
    match(SysMLv2Parser::NAME);
    setState(225);
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
  enterRule(_localctx, 42, SysMLv2Parser::RuleNamelist);
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
    setState(227);
    name();
    setState(232);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysMLv2Parser::T__1) {
      setState(228);
      match(SysMLv2Parser::T__1);
      setState(229);
      name();
      setState(234);
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
  enterRule(_localctx, 44, SysMLv2Parser::RuleName);
  size_t _la = 0;

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    setState(243);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysMLv2Parser::NAME: {
        enterOuterAlt(_localctx, 1);
        setState(235);
        match(SysMLv2Parser::NAME);
        break;
      }

      case SysMLv2Parser::T__3: {
        enterOuterAlt(_localctx, 2);
        setState(236);
        match(SysMLv2Parser::T__3);
        setState(238); 
        _errHandler->sync(this);
        _la = _input->LA(1);
        do {
          setState(237);
          match(SysMLv2Parser::NAME);
          setState(240); 
          _errHandler->sync(this);
          _la = _input->LA(1);
        } while (_la == SysMLv2Parser::NAME);
        setState(242);
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
  enterRule(_localctx, 46, SysMLv2Parser::RuleAddress);
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
    setState(245);
    match(SysMLv2Parser::NAME);
    setState(256);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysMLv2Parser::T__4) {
      setState(246);
      match(SysMLv2Parser::T__4);
      setState(252);
      _errHandler->sync(this);
      switch (_input->LA(1)) {
        case SysMLv2Parser::NAME: {
          setState(247);
          match(SysMLv2Parser::NAME);
          break;
        }

        case SysMLv2Parser::STAR: {
          setState(248);
          match(SysMLv2Parser::STAR);
          setState(250);
          _errHandler->sync(this);

          _la = _input->LA(1);
          if (_la == SysMLv2Parser::STAR) {
            setState(249);
            match(SysMLv2Parser::STAR);
          }
          break;
        }

      default:
        throw NoViableAltException(this);
      }
      setState(258);
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

SysMLv2Parser::StartContext* SysMLv2Parser::Bracketed_contentContext::start() {
  return getRuleContext<SysMLv2Parser::StartContext>(0);
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
  enterRule(_localctx, 48, SysMLv2Parser::RuleBracketed_content);

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
    match(SysMLv2Parser::T__5);
    setState(260);
    start(0);
    setState(261);
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
  enterRule(_localctx, 50, SysMLv2Parser::RuleFuction_arguments);
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
    setState(263);
    match(SysMLv2Parser::T__7);
    setState(265);
    _errHandler->sync(this);

    _la = _input->LA(1);
    if (_la == SysMLv2Parser::NAME) {
      setState(264);
      argument();
    }

    setState(267);
    match(SysMLv2Parser::T__1);
    setState(271);
    _errHandler->sync(this);
    _la = _input->LA(1);
    while (_la == SysMLv2Parser::NAME) {
      setState(268);
      argument();
      setState(273);
      _errHandler->sync(this);
      _la = _input->LA(1);
    }
    setState(274);
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
  enterRule(_localctx, 52, SysMLv2Parser::RuleArgument);

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
    match(SysMLv2Parser::NAME);
    setState(277);
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
  enterRule(_localctx, 54, SysMLv2Parser::RuleDelimiter_rule);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(281);
    _errHandler->sync(this);
    switch (_input->LA(1)) {
      case SysMLv2Parser::T__5: {
        setState(279);
        bracketed_content();
        break;
      }

      case SysMLv2Parser::DELIMITER: {
        setState(280);
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
  enterRule(_localctx, 56, SysMLv2Parser::RuleMultiplicity);
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
    setState(283);
    match(SysMLv2Parser::T__9);
    setState(284);
    _la = _input->LA(1);
    if (!(_la == SysMLv2Parser::STAR

    || _la == SysMLv2Parser::NUMBER)) {
    _errHandler->recoverInline(this);
    }
    else {
      _errHandler->reportMatch(this);
      consume();
    }
    setState(285);
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
  enterRule(_localctx, 58, SysMLv2Parser::RuleUnit);

#if __cplusplus > 201703L
  auto onExit = finally([=, this] {
#else
  auto onExit = finally([=] {
#endif
    exitRule();
  });
  try {
    enterOuterAlt(_localctx, 1);
    setState(287);
    match(SysMLv2Parser::T__9);
    setState(288);
    match(SysMLv2Parser::NAME);
    setState(289);
    match(SysMLv2Parser::T__10);
   
  }
  catch (RecognitionException &e) {
    _errHandler->reportError(this, e);
    _localctx->exception = std::current_exception();
    _errHandler->recover(this, _localctx->exception);
  }

  return _localctx;
}

bool SysMLv2Parser::sempred(RuleContext *context, size_t ruleIndex, size_t predicateIndex) {
  switch (ruleIndex) {
    case 0: return startSempred(antlrcpp::downCast<StartContext *>(context), predicateIndex);

  default:
    break;
  }
  return true;
}

bool SysMLv2Parser::startSempred(StartContext *, size_t predicateIndex) {
  switch (predicateIndex) {
    case 0: return precpred(_ctx, 17);

  default:
    break;
  }
  return true;
}

void SysMLv2Parser::initialize() {
#if ANTLR4_USE_THREAD_LOCAL_CACHE
  sysmlv2ParserInitialize();
#else
  ::antlr4::internal::call_once(sysmlv2ParserOnceFlag, sysmlv2ParserInitialize);
#endif
}
