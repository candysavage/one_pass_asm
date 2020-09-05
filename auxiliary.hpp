#ifndef _auxiliary_hpp_
#define _auxiliary_hpp_

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <sstream>

namespace Aux {
	//EXIT CODES
	constexpr auto ERR_OK = 0, ERR_FOPEN = 1, ERR_ARGUMENT = 2, ERR_SYNTAX = 3,
			ERR_SECTION = 4, ERR_MULTIPLE_DEFINITIONS = 5, ERR_REDEFINITION = 6, ERR_PCREL_ARG = 7, ERR_INVALID_OPERAND = 8,
			ERR_UNDEFINED_SYMBOL = 9;

	constexpr auto UNDEFINED_SECTION = 0;

	constexpr auto ADD = '+', SUB = '-';

	constexpr auto R_16 = "R_16", R_PC16 = "R_PC16", LITERAL = "LITERAL";

	constexpr auto IMMED = "immed", REGDIR = "regdir", REGIND = "regind",
			REGIND16B = "regind16b", MEMDIR = "memdir";

	constexpr uint8_t r0 = 0x0, r1 = 0x1, r2 = 0x2, r3 = 0x3, r4 = 0x4, r5 = 0x5,
			r6 = 0x6, sp = 0x6, r7 = 0x7, pc = 0x7, psw = 0xf;

	namespace MAPS {
	std::unordered_map<std::string, uint8_t> regs = { { "r0", 0x0 }, { "r1", 0x1 },
			{ "r2", 0x2 }, { "r3", 0x3 }, { "r4", 0x4 }, { "r5", 0x5 },
			{ "r6", 0x6 }, { "sp", 0x6 }, { "r7", 0x7 }, { "pc", 0x7 },
			{ "psw", 0xf } };

	std::unordered_map<std::string, uint8_t> opCode = { { "halt", 0x0 }, { "iret",
			0x1 }, { "ret", 0x2 }, { "int", 0x3 }, { "call", 0x4 }, { "jmp", 0x5 },
			{ "jeq", 0x6 }, { "jne", 0x7 }, { "jgt", 0x8 }, { "push", 0x9 }, {
					"pop", 0xa }, { "xchg", 0xb }, { "xchgw", 0xb },
			{ "xchgb", 0xb }, { "mov", 0xc }, { "movw", 0xc }, { "movb", 0xc }, {
					"add", 0xd }, { "addw", 0xd }, { "addb", 0xd }, { "sub", 0xe },
			{ "subw", 0xe }, { "subb", 0xe }, { "mul", 0xf }, { "mulw", 0xf }, {
					"mulb", 0xf }, { "div", 0x10 }, { "divw", 0x10 },
			{ "divb", 0x10 }, { "cmp", 0x11 }, { "cmpw", 0x11 }, { "cmpb", 0x11 }, {
					"not", 0x12 }, { "notw", 0x12 }, { "notb", 0x12 },
			{ "and", 0x13 }, { "andw", 0x13 }, { "andb", 0x13 }, { "or", 0x14 }, {
					"orw", 0x14 }, { "orb", 0x14 }, { "xor", 0x15 },
			{ "xorw", 0x15 }, { "xorb", 0x15 }, { "test", 0x16 }, { "testw", 0x16 },
			{ "testb", 0x16 }, { "shl", 0x17 }, { "shlw", 0x17 }, { "shlb", 0x17 },
			{ "shr", 0x18 }, { "shrw", 0x18 }, { "shrb", 0x18 } };

	std::unordered_map<std::string, uint8_t> operandSize =
			{ { "mov", 1 }, { "movw", 1 }, { "movb", 0 }, { "xchg", 1 }, { "xchgw",
					1 }, { "xchgb", 0 }, { "add", 1 }, { "addw", 1 }, { "addb", 0 },
					{ "sub", 1 }, { "subw", 1 }, { "subb", 0 }, { "mul", 1 }, {
							"mulw", 1 }, { "mulb", 0 }, { "push", 1 }, { "pop", 1 },
					{ "div", 1 }, { "divw", 1 }, { "divb", 0 }, { "cmp", 1 }, {
							"cmpw", 1 }, { "cmpb", 0 }, { "not", 1 }, { "notw", 1 },
					{ "notb", 0 }, { "and", 1 }, { "andw", 1 }, { "andb", 0 }, {
							"or", 1 }, { "orw", 1 }, { "orb", 0 }, { "xor", 1 }, {
							"xorw", 1 }, { "xorb", 0 }, { "test", 1 },
					{ "testw", 1 }, { "testb", 0 }, { "shl", 1 }, { "shlw", 1 }, {
							"shlb", 0 }, { "shr", 1 }, { "shrw", 1 }, { "shrb", 0 } };

	std::unordered_map<std::string, uint8_t> addressingMode = { { "immed", 0x0 }, {
			"regdir", 0x1 }, { "regind", 0x2 }, { "regind16b", 0x3 }, { "memdir",
			0x4 } };
	}

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

	const char *regexes[] ={
			"^[ 	]*(?:#.*)*$",
			"^[ 	]*([a-zA-Z_][a-zA-Z_0-9]*):[ 	]*(?:#.*)*$",
			"^(?:\\.section[ 	]+)?\\.([a-zA-Z]+)[ 	]*(?:#.*)*$",
			"^\\.equ[ 	]+([a-z_A-Z][a-zA-Z0-9_]*),[ 	]*([\\+-]?[0-9a-zA-Z_]+(?:[\\+-][0-9a-zA-Z_]+)*)[ 	]*(?:#.*)*$",
			"^\\.global[ 	]+((?:[a-zA-Z_][a-zA-Z_0-9]*)(?:,[a-zA-Z_][a-zA-Z_0-9]*)*)[ 	]*(?:#.*)*$",
			"^\\.extern[ 	]+((?:[a-zA-Z_][a-zA-Z_0-9]*)(?:,[a-zA-Z_][a-zA-Z_0-9]*)*)[ 	]*(?:#.*)*$",
			"^[ 	]*(?:([a-zA-Z_][a-zA-Z_0-9]*):)?[ 	]*\\.byte[ 	]+((?:-?[a-zA-Z_0-9]+)(?:,-?[a-zA-Z_0-9]+)*)[ 	]*(?:#.*)*$",
			"^[ 	]*(?:([a-zA-Z_][a-zA-Z_0-9]*):)?[ 	]*\\.word[ 	]+((?:-?[a-zA-Z_0-9]+)(?:,-?[a-zA-Z_0-9]+)*)[ 	]*(?:#.*)*$",
			"^[ 	]*(?:([a-zA-Z_][a-zA-Z_0-9]*):)?[ 	]*\\.skip[ 	]+((?:0x)?[0-9a-fA-F]+)[ 	]*(?:#.*)*$",
			"^[ 	]*(?:([a-zA-Z_][a-zA-Z_0-9]*):)?[ 	]*(halt|iret|ret)[ 	]*(?:#.*)*$",
			"^[ 	]*(?:([a-zA-Z_][a-zA-Z_0-9]*):)?[ 	]*(int|call|jmp|jeq|jne|jgt|push|pop)[ 	]+((?:(?:\\*)?(?:0x)?[1-9a-fA-F][0-9a-fA-F]*(?:\\(%(?:r[0-7]|pc|sp)\\))?)|(?:(?:\\*)?[a-zA-Z_][a-zA-Z_0-9]*(?:\\(%(?:r[0-7]|pc|sp)\\))?)|(?:\\*%(?:r[0-7]|pc|sp))|(?:\\*\\(%(?:r[0-7]|pc|sp)\\)))[ 	]*(?:#.*)*$",
			"^[ 	]*(?:([a-zA-Z_][a-zA-Z_0-9]*):)?[ 	]*(xchg[bw]?|not[bw]?|mov[bw]?|add[bw]?|sub[bw]?|mul[bw]?|div[bw]?|cmp[bw]?|and[bw]?|or[bw]?|xor[bw]?|test[bw]?|shl[bw]?|shr[bw]?)[ 	]+((?:(?:\\$)?(?:0x)?[1-9a-fA-F][0-9a-fA-F]*(?:\\(%(?:r[0-7]|pc|sp)\\))?)|(?:(?:\\$)?[a-zA-Z_][a-zA-Z_0-9]*(?:\\(%(?:r[0-7]|pc|sp)\\))?)|(?:%(?:r[0-7]|pc|sp)[lh]?)|(?:\\(%(?:r[0-7]|pc|sp)[lh]?\\))),[ 	]*((?:(?:\\$)?(?:0x)?[1-9a-fA-F][0-9a-fA-F]*(?:\\(%(?:r[0-7]|pc|sp)\\))?)|(?:(?:\\$)?[a-zA-Z_][a-zA-Z_0-9]*(?:\\(%(?:r[0-7]|pc|sp)\\))?)|(?:%(?:r[0-7]|pc|sp)[lh]?)|(?:\\(%(?:r[0-7]|pc|sp)[lh]?\\)))[ 	]*(?:#.*)*$"
	};

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

	std::vector<std::string> parserComma(std::string str) {
		std::stringstream parser(str);
		std::string intermediate;
		std::vector<std::string> retVector;
		while (std::getline(parser, intermediate, ',')) {
			intermediate.erase(
					std::remove(intermediate.begin(), intermediate.end(), ' '),
					intermediate.end());
			retVector.push_back(intermediate);
		}

		return retVector;
	}

	int16_t toInt16_t(std::string str) {
		if (str[0] == '0') {
			if (str[1] == 'b')
				return (int16_t) stoi(str, nullptr, 2);
			if (str[1] == 'o')
				return (int16_t) stoi(str, nullptr, 8);
			if (str[1] == 'x')
				return (int16_t) stoi(str, nullptr, 16);
		}
		return stoi(str, nullptr, 10);
	}

	int8_t toInt8_t(std::string str) {
		if (str[0] == '*') {
			str.erase(0, 1);
		}
		if (str[0] == '0') {
			if (str[1] == 'b')
				return (int8_t) stoi(str, nullptr, 2);
			if (str[1] == 'o')
				return (int8_t) stoi(str, nullptr, 8);
			if (str[1] == 'x')
				return (int8_t) stoi(str, nullptr, 16);
		}
		return stoi(str, nullptr, 10);
	}

	bool isJump(std::string i) {
		if (i == "int" || i == "call" || i == "jmp" || i == "jeq" || i == "jne"
				|| i == "jgt") {
			return true;
		}

		return false;
	}
}

#endif
