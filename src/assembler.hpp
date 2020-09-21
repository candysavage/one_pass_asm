#ifndef _assembler_hpp_
#define _assembler_hpp_

#include <regex>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <string>
#include <iostream>
#include <fstream>

#include "auxiliary.hpp"

class Assembler {
public:
	Assembler();
	~Assembler();

	void generateObj();
	void argumentsAnalyzer(int, std::vector<std::string>);
private:
	void assemble();
	void backpatch();

	struct regexWrapper {
		std::regex *regex;
		RegexTypes type;
	} allTheRegex[numberOfRegex];

	std::regex operandJumpRegex;
	std::regex operandInstructionRegex;

	std::smatch matches;
	std::smatch operandType;

	uint16_t locationCounter;
	std::string readLine;
	int readingLineNumber;
	bool foundEnd;
	std::string currentSection;
	uint8_t currentSectionSymbolNumber;
	uint8_t symbolNumber;

	std::fstream logFile;
	std::fstream objectFile;
	std::fstream asmFile;

	std::unordered_map<std::string, symbolTableEntry> symbolTable;
	std::unordered_map<std::string, sectionEntry> sectionTable;
	std::unordered_map<uint8_t, std::string> sectionTranslation;
	std::unordered_map<std::string, std::vector<relocationEntry>> relocationTable;

	std::unordered_map<std::string, literalEntry> literalTable;

	std::unordered_map<std::string, std::vector<backpatchInfo>> TII;

	std::unordered_map<uint8_t, std::vector<uint8_t>> machineCode;

	bool checkSymbolExists(std::string);
	bool checkSymbolIsLiteral(std::string);
	bool checkSymbolIsExtern(std::string);
	bool checkSymbolIsGlobal(std::string);
	bool checkSymbolIsDefined(std::string);

	void defineLabel(std::string);
	void addUndefinedSymbol(std::string);
	void addNewLabel(std::string);
	void addGlobal(std::string);
	void addExtern(std::string);

	void checkSection();

	void calculateLiteral(std::string);
	void calculateExpression(std::string, char, std::string);

	int8_t toInt8_t(std::string);
	int16_t toInt16_t(std::string);

	void resolveSymbol(std::string);
	int autoRelocation(std::string, char, std::string);

	void createRelocation(uint8_t, std::string, char);
	void createBackpatchEntry(std::string, char, uint8_t, std::string relocationType);

	void regexInit();
	void validateRegex();
	void decypherRegex(int);

	void logger(std::string);
	void logger(std::string s, int i);

	template <typename T>
	void printElement(T t);

	std::string get(uint8_t);
};

#endif
