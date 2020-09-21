#include <cstdint>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <iterator>
#include <string>
#include <vector>

#include "auxiliary.hpp"

std::unordered_map<std::string, uint8_t> MAPS::regs = { { "r0", 0x0 }, { "r1", 0x1 },
			{ "r2", 0x2 }, { "r3", 0x3 }, { "r4", 0x4 }, { "r5", 0x5 },
			{ "r6", 0x6 }, { "sp", 0x6 }, { "r7", 0x7 }, { "pc", 0x7 },
			{ "psw", 0xf } };

std::unordered_map<std::string, uint8_t> MAPS::opCode = { { "halt", 0x0 }, { "iret",
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


std::unordered_map<std::string, uint8_t> MAPS::operandSize =
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


std::unordered_map<std::string, uint8_t> MAPS::addressingMode = { { "immed", 0x0 }, {
			"regdir", 0x1 }, { "regind", 0x2 }, { "regind16b", 0x3 }, { "memdir",
			0x4 } };


std::vector<std::string> parserComma(std::string str) {
	std::stringstream parser(str);
	std::string intermediate;
	std::vector<std::string> retVector;
	str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
	while (std::getline(parser, intermediate, ',')) {
		if(intermediate != "")
			retVector.push_back(intermediate);
	}

	return retVector;
}

bool isJump(std::string instruction) {
	if (instruction == "int" || instruction == "call" || instruction == "jmp" || instruction == "jeq" || instruction == "jne"
			|| instruction == "jgt") {
		return true;
	}

	return false;
}

