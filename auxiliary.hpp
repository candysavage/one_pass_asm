#ifndef _auxiliary_hpp_
#define _auxiliary_hpp_

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

//EXIT CODES
static constexpr auto ERR_OK = 0, ERR_FOPEN = 1, ERR_ARGUMENT = 2, ERR_SYNTAX = 3,
		ERR_SECTION = 4, ERR_MULTIPLE_DEFINITIONS = 5, ERR_REDEFINITION = 6, ERR_PCREL_ARG = 7, ERR_INVALID_OPERAND = 8,
		ERR_UNDEFINED_SYMBOL = 9;

static constexpr auto UNDEFINED_SECTION = 0;

static constexpr auto ADD = '+', SUB = '-';

static constexpr auto R_16 = "R_16", R_PC16 = "R_PC16", LITERAL = "LITERAL";

static constexpr auto IMMED = "immed", REGDIR = "regdir", REGIND = "regind",
		REGIND16B = "regind16b", MEMDIR = "memdir";

static constexpr uint8_t r0 = 0x0, r1 = 0x1, r2 = 0x2, r3 = 0x3, r4 = 0x4, r5 = 0x5,
		r6 = 0x6, sp = 0x6, r7 = 0x7, pc = 0x7, psw = 0xf;

static constexpr auto INT16_T_MAX = 32767;
static constexpr auto INT16_T_MIN = -32768;
static constexpr auto INT8_T_MAX = 127;
static constexpr auto INT8_T_MIN = -128;

class MAPS {
public:
	static std::unordered_map<std::string, uint8_t> regs;

	static std::unordered_map<std::string, uint8_t> opCode;

	static std::unordered_map<std::string, uint8_t> operandSize;

	static std::unordered_map<std::string, uint8_t> addressingMode;
};

// First byte of an instruction (operation code :5, operand size [1 or 2 bytes]:1 , unused :2)
union Mnemonics {
	struct {
		uint8_t val;
	};
	struct {
		uint8_t :2, size :1, opcode :5;
	};
};
// Second byte (addressing type :3, )
union Addressing {
	struct {
		uint8_t val;
	};
	struct {
		uint8_t part :1, regs :4, addressMode :3;
	};
};

union ImmedValues {
	struct {
		int16_t val;
	};
	struct {
		uint8_t signed8;
	};
	struct {
		uint8_t byte1, byte2; //byte1 - nizi; byte2 - visi
	};
};

enum RegexTypes {
	/*0*/regexComment,               // 1: #komentar
	/*1*/
	regexLabel,                 // 1: naziv_labele
	/*2*/
	regexSection,               // 1: .naziv sekcije
	/*3*/
	regexEqu,                   // 1: naziv_simbola 2:ceo izraz
	/*4*/
	regexGlobal,                // 1: naziv_simbola [, vise njih]
	/*5*/
	regexExtern,                // 1: naziv_simbola [vise njih]
	/*6*/
	regexByte,                  // 1: [labela] 2: niz izraza odvojenih zarezom
	/*7*/
	regexWord,                  // 1: [labela] 2: niz izraza odvojenih zarezom
	/*8*/
	regexSkip,                  // 1: [labela] 2: koliko bajtova skip u dekadnom
	/*9*/
	regexInstrNoOperand,        // 1: [labela] 2: instr
	/*10*/
	regexInstrOneOperand,       // 1: [labela] 2: instr 3: literal, simbol
	/*11*/
	regexInstrTwoOperand // 1: [labela] 2: instr 3: izraz operanda / registar kod regdir 4: '[' 5: registar 6:']' 7: [izraz pomeraja] 8: izraz operanda / registar kod regdir 9: '[' a: registar b: ']' c: [izraz pomeraja]
};

constexpr uint8_t numberOfRegex = 12;

constexpr uint8_t LABEL = 1, SECTION = 1, SYMBOL = 1, EXPRESSION = 2,    // .equ
		LIST = 2,           //.byte .word .skip
		OPERATION = 2, ARG1 = 3, ARG2 = 4;

/*  std::string symbolName;   ulaz u hashmapu
 uint8_t number;     // symbol number
 uint8_t sectionNumber;  // broj sekcije u kojoj se nalazi, iskoristi u tabeli relokacija
 uint16_t offset;    // 0 for section
 std::string type; // global, local, extern
 uint16_t size;  // 0 for label
 std::string symbolType;   // label, section
 */
typedef struct {
	uint8_t number;
	uint8_t sectionNumber;
	uint16_t offset;
	std::string type;
	uint16_t size;
	std::string symbolType;
} symbolTableEntry;

/*  std::string sectionName; ulaz u hashmapu
 uint16_t sectionSize;
 uint8_t number;
 */
typedef struct {
	uint16_t sectionSize;
	uint8_t number;
} sectionEntry;

typedef struct {
	char operation;
	std::string symbol;
} expressionStruct;

typedef struct {
	uint8_t sectionNumber;
	uint16_t offset;
	char action;
	uint8_t size;   //number of bytes 1 or 2
	std::string relocationType;
} backpatchInfo;

// Tabela relokacija
typedef struct {
	uint16_t offset;
	std::string type;   // "R_16" ili "R_PC16" za skokove
	char op;     		// "+" or "-"
	uint8_t value;      // section/symbol number
} relocationEntry;

typedef struct {
	uint8_t symbolNumber;
	char op;
	std::string type;
} relocationInfo;

typedef struct {
	std::string expression;
	int16_t value;
	std::vector<relocationInfo> relocations;
} literalEntry;

std::vector<std::string> parserComma(std::string str);

int16_t toInt16_t(std::string str);

int8_t toInt8_t(std::string str);

bool isJump(std::string i);

#endif
