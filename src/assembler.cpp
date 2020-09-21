#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

#include "assembler.hpp"
#include "auxiliary.hpp"

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
		"^[ 	]*(?:([a-zA-Z_][a-zA-Z_0-9]*):)?[ 	]*(int|call|jmp|jeq|jne|jgt|push|pop)[ 	]+((?:(?:\\*)?(?:0x|-)?[1-9a-fA-F][0-9a-fA-F]*(?:\\(%(?:r[0-7]|pc|sp)\\))?)|(?:(?:\\*)?[a-zA-Z_][a-zA-Z_0-9]*(?:\\(%(?:r[0-7]|pc|sp)\\))?)|(?:(?:\\*)?%(?:r[0-7]|pc|sp))|(?:(?:\\*)?\\(%(?:r[0-7]|pc|sp)\\)))[ 	]*(?:#.*)*$",
		"^[ 	]*(?:([a-zA-Z_][a-zA-Z_0-9]*):)?[ 	]*(xchg[bw]?|not[bw]?|mov[bw]?|add[bw]?|sub[bw]?|mul[bw]?|div[bw]?|cmp[bw]?|and[bw]?|or[bw]?|xor[bw]?|test[bw]?|shl[bw]?|shr[bw]?)[ 	]+((?:(?:\\$)?(?:0x|-)?[0-9a-fA-F]+(?:\\(%(?:r[0-7]|pc|sp)\\))?)|(?:(?:\\$)?[a-zA-Z_][a-zA-Z_0-9]*(?:\\(%(?:r[0-7]|pc|sp)\\))?)|(?:%(?:r[0-7]|pc|sp)[lh]?)|(?:\\(%(?:r[0-7]|pc|sp)\\))),[ 	]*((?:(?:\\$)?(?:0x|-)?[0-9a-fA-F]+(?:\\(%(?:r[0-7]|pc|sp)\\))?)|(?:(?:\\$)?[a-zA-Z_][a-zA-Z_0-9]*(?:\\(%(?:r[0-7]|pc|sp)\\))?)|(?:%(?:r[0-7]|pc|sp)[lh]?)|(?:\\(%(?:r[0-7]|pc|sp)\\)))[ 	]*(?:#.*)*$"
};

const char* jumpRegex = "((?:\\*)?(?:0x[0-9a-fA-F]+|-?[1-9][0-9]*)(?:\\(%(?:r[0-7]|pc|sp)\\))?)|((?:\\*)?[a-zA-Z_][a-zA-Z_0-9]*(?:\\(%(?:r[0-7]|pc|sp)\\))?)|(\\*%(?:r[0-7]|pc|sp))|(\\*\\(%(?:r[0-7]|pc|sp)\\))";
const char* instrOperandRegex = "((?:\\$)?(?:0x[0-9a-fA-F]+|-?[0-9]+)(?:\\(%(?:r[0-7]|pc|sp)\\))?)|((?:\\$)?[a-zA-Z_][a-zA-Z_0-9]*(?:\\(%(?:r[0-7]|pc|sp)\\))?)|(%(?:r[0-7]|pc|sp)[lh]?)|(\\(%(?:r[0-7]|pc|sp)\\))";

void returnErrorCode(const int err) {
	printf("**** Application returned error code %d ****", err);
	exit(err);
}

Assembler::Assembler() {

	readingLineNumber = 0;
	currentSection = "UNDEFINED";
	currentSectionSymbolNumber = 0;
	symbolNumber = 0;
	locationCounter = 0;
	foundEnd = false;
	logFile.open("assemblyLog.txt", std::ios::out);
	if (!logFile.good()) {
		std::cout << "Unable to open log file. Abort.\n" << std::endl;
		returnErrorCode(ERR_FOPEN);
	}
	sectionTable.insert( { currentSection, { locationCounter,
			currentSectionSymbolNumber } });
	sectionTranslation.insert({0, "UNDEFINED"});

	logger("Created Assembler class object\n");

	regexInit();
	logger("Initialized regex objects\n");
}

Assembler::~Assembler() {
	logFile.close();
	asmFile.close();
	objectFile.close();

	for (auto i = 0; i < numberOfRegex; i++) {
		delete allTheRegex[i].regex;
	}
}

void Assembler::regexInit() {
	for (auto i = 0; i < numberOfRegex; i++) {
		allTheRegex[i].regex = new std::regex(regexes[i]);
	}

	/***
	 * (1) *0xff(%r0), *0xff, 0xff
	 * (2) *labela1(%r0), *labela2, labela3
	 * (3) *%r0
	 * (4) *%(r0)
	 */
	operandJumpRegex = std::regex(jumpRegex);
	operandInstructionRegex = std::regex(instrOperandRegex);
}

void Assembler::logger(std::string s) {
	logFile << s << std::endl;
}

void Assembler::logger(std::string s, int i) {
	logFile << s << i << std::endl;
}

void Assembler::argumentsAnalyzer(int argc, std::vector<std::string> args) {
	auto isNextObj = false;
	for (auto i = 0; i < argc; i++) {
		if (isNextObj) {
			objectFile.open(args[i], std::ios::out);
			isNextObj = false;
			if (!objectFile.good()) {
				logger("Error while trying to create obj file");
				returnErrorCode(ERR_FOPEN);
			}
		} else {
			auto first = args[i][0];
			switch (first) {
			case '-':
				if (args[i][1] == 'o') {
					isNextObj = true;
				} else {
					logger("Invalid argument after - ");
					returnErrorCode(ERR_ARGUMENT);
				}
				break;
			default:
				asmFile.open(args[i], std::ios::in);
				if (!asmFile.good()) {
					logger("Error while trying to open src file");
					returnErrorCode(ERR_FOPEN);
				}
				break;
			}
		}
	}
}

bool compareSymbolTypes(std::pair<std::string, symbolTableEntry> a, std::pair<std::string, symbolTableEntry> b) {
	if(a.second.symbolType == "section" && b.second.symbolType == "section") {
		return true;
	}
	if(a.second.symbolType == "section" && b.second.symbolType == "label") {
		return true;
	}
	if(a.second.symbolType == "label" && b.second.symbolType == "section") {
		return false;
	}

	return true;
}

bool compareUINT8_T(std::pair<std::string, symbolTableEntry> a, std::pair<std::string, symbolTableEntry> b) {
	if(a.second.number <= b.second.number) {
		return true;
	}
	return false;
}

bool compareRelocationOffsets(relocationEntry a, relocationEntry b) {
	if(a.offset <= b.offset) {
		return true;
	}
	return false;
}

bool compareValues(std::pair<std::string, literalEntry> a, std::pair<std::string, literalEntry> b) {
	if(a.second.value <= b.second.value) {
		return true;
	}
	return false;
}

template<typename T>
void Assembler::printElement(T t){
    objectFile << std::right << std::setw(20) << std::setfill(' ') << t;
}

void Assembler::generateObj() {
	readLine = "";
	while (std::getline(asmFile, readLine)) {
		++readingLineNumber;
		validateRegex();
		if (foundEnd == true)
			break;
	}

	logger("Finished parsing file");
	// prvo izracunaj sve izraze u literalima
	for(auto iter = literalTable.begin() ; iter != literalTable.end(); ++iter) {
		auto literal = iter->first;
		calculateLiteral(literal);
	}

	logger("Calculated literal symbols");
	// onda backpatching koda i potrebne relokacije
	for(auto iter = TII.begin(); iter != TII.end(); iter++) {
		auto symbol = iter->first;
		for(auto entry : iter->second) {
			auto operation = entry.action;
			auto offset = entry.offset;
			auto section = entry.sectionNumber;
			auto& vect = machineCode[section];
			if(checkSymbolIsLiteral(symbol)) {
				auto& literal = literalTable[symbol];
				if(entry.size == 1) {
					if(literal.relocations.size() != 0 || literal.value < -128 || literal.value > 127) {
						std::stringstream log;
						log << "2B literal used in 1B backpatch section " << section << " offset " << offset;
						logger(log.str());
						returnErrorCode(ERR_INVALID_OPERAND);
					}
					vect[offset] += (operation == ADD) ? literal.value : 0 - literal.value;
				} else {
					ImmedValues immed;
					immed.byte1 = vect[offset];
					immed.byte2 = vect[offset+1];
					immed.val += (operation == ADD) ? literal.value : 0 - literal.value;
					vect[offset] = immed.byte1;
					vect[offset+1] = immed.byte2;
					for(auto reloc : literal.relocations) {
						relocationTable[sectionTranslation[section]].push_back({offset, reloc.type, reloc.op, reloc.symbolNumber});
					}
				}
			} else {
				if(checkSymbolExists(symbol)) {
					auto& sym = symbolTable[symbol];
					if(checkSymbolIsExtern(symbol) || checkSymbolIsGlobal(symbol)) {
						if(checkSymbolIsGlobal(symbol) && entry.relocationType == R_PC16 && section == sym.sectionNumber) {
							ImmedValues immed;
							immed.byte1 = vect[offset];
							immed.byte2 = vect[offset+1];
							immed.val += sym.offset - offset;
							vect[offset] = immed.byte1;
							vect[offset+1] = immed.byte2;
						} else {
							relocationTable[sectionTranslation[section]].push_back({offset, entry.relocationType, operation, sym.number});
						}
					} else {
						if(checkSymbolIsDefined(symbol)) {
							ImmedValues immed;
							immed.byte1 = vect[offset];
							immed.byte2 = vect[offset+1];
							if(entry.relocationType != R_PC16 || section != sym.sectionNumber) {
								immed.val += (operation == ADD) ? sym.offset : 0 - sym.offset;
								relocationTable[sectionTranslation[section]].push_back({offset, entry.relocationType, operation, sym.sectionNumber});
							} else {
								immed.val += sym.offset - offset;
							}
							vect[offset] = immed.byte1;
							vect[offset+1] = immed.byte2;
						} else {
							std::stringstream log;
							log << "Symbol doesn't exist, backpatching failed at section " << entry.sectionNumber << " offset " << entry.offset;
							logger(log.str());
							returnErrorCode(ERR_UNDEFINED_SYMBOL);
						}
					}
				} else {
					std::stringstream log;
					log << "Symbol doesn't exist, backpatching failed at section " << entry.sectionNumber << " offset " << entry.offset;
					logger(log.str());
					returnErrorCode(ERR_UNDEFINED_SYMBOL);
				}
			}
		}
	}

	logger("Done backpatching");
	// tabela simbola
	std::vector<std::pair<std::string, symbolTableEntry>> symbols(symbolTable.begin(), symbolTable.end());
	std::sort(symbols.begin(), symbols.end(), compareSymbolTypes);
	objectFile << "%SYMBOL TABLE%" << std::endl;
	printElement("Symbol");
	printElement("Symbol number");
	printElement("Section");
	printElement("Offset");
	printElement("Type");
	printElement("Size");
	printElement("SymbolType");
	objectFile << std::endl;
	for(auto symbol : symbols) {
		if(symbol.second.sectionNumber == UNDEFINED_SECTION && symbol.second.type != "extern") {
			logger("Undefined non-extern symbol");
			returnErrorCode(ERR_SYNTAX);
		}
		printElement(symbol.first);
		printElement((int)symbol.second.number);
		printElement(sectionTranslation[symbol.second.sectionNumber]);
		printElement(symbol.second.offset);
		printElement(symbol.second.type);
		printElement((int)symbol.second.size);
		printElement(symbol.second.symbolType);
		objectFile << std::endl;
	}
	objectFile << std::endl;

	// tabela equ literala
	objectFile << "%EQU SYMBOLS%" << std::endl;
	printElement("Symbol");
	printElement("Value");
	printElement("Relocations");
	objectFile << std::endl;
	std::vector<std::pair<std::string, literalEntry>> literals(literalTable.begin(), literalTable.end());
	std::sort(literals.begin(), literals.end(), compareValues);
	for(auto literal : literals) {
		printElement(literal.first);
		printElement((int)literal.second.value);
		std::stringstream ss;
		for(auto relocs : literal.second.relocations) {
			ss << relocs.op;
			ss << (int)relocs.symbolNumber;
			ss << ' ';
		}
		printElement(ss.str());
		objectFile << std::endl;
	}

	objectFile << std::endl;

	// tabele relokacija po sekciji
	for(auto it : relocationTable) {
		printElement("%RELOCATION TABLE% - section ");
		printElement(it.first);
		objectFile << std::endl;
		printElement("Symbol number");
		printElement("Offset");
		printElement("Operation");
		printElement("Relocation type");
		objectFile << std::endl;
		std::sort(it.second.begin(), it.second.end(), compareRelocationOffsets);
		for(auto reloc : it.second) {
			printElement((int)reloc.value);
			printElement(reloc.offset);
			printElement(reloc.op);
			printElement(reloc.type);
			objectFile << std::endl;
		}
	objectFile << std::endl;
	}
	objectFile << std::endl;

	// masinski kod po sekcijama
	for (auto it : sectionTable) {
		if(it.first == "UNDEFINED") {
			continue;
		}
		objectFile <<std::dec<< "." <<it.first<<"\t"<<(int)it.second.sectionSize<<std::endl;
		auto breaker = 0;
		for(auto i : machineCode[it.second.number]) {
			objectFile<<std::setfill('0')<<std::hex<<std::setw(2)<<(unsigned)i<<" ";
			++breaker;
			if(breaker%16 == 0){
				objectFile << std::endl;
			}
		}
		objectFile << std::endl;
		objectFile << std::endl;
	}

}

std::string Assembler::get(uint8_t i) {
	return matches.str(i);
}

bool Assembler::checkSymbolExists(std::string label) {
	return (symbolTable.find(label) != symbolTable.end());
}
bool Assembler::checkSymbolIsLiteral(std::string symbol) {
	return (literalTable.find(symbol) != literalTable.end());
}

bool Assembler::checkSymbolIsExtern(std::string symbol) {
	return symbolTable.at(symbol).type == "extern";
}

bool Assembler::checkSymbolIsGlobal(std::string symbol) {
	return symbolTable.at(symbol).type == "global";
}

bool Assembler::checkSymbolIsDefined(std::string label) {
	return symbolTable.at(label).sectionNumber;
}

void Assembler::defineLabel(std::string label) {
	auto& symbol = symbolTable[label];
	symbol.sectionNumber = currentSectionSymbolNumber;
	symbol.offset = locationCounter;
	symbol.size = 0;
	if (symbol.type != "global") {
		symbol.type = "local";
	}
	symbol.symbolType = "label";
}

void Assembler::addUndefinedSymbol(std::string symbol) {
	++symbolNumber;
	symbolTable.insert( { symbol, { symbolNumber, UNDEFINED_SECTION, 0, "local",
			0, "label" } });
}

void Assembler::addNewLabel(std::string label) {
	++symbolNumber;
	symbolTable.insert( { label, { symbolNumber, currentSectionSymbolNumber, locationCounter, "local", 0, "label" } });
}

void Assembler::checkSection() {
	if (currentSection == "UNDEFINED") {
		logger("Out of section code at line ",readingLineNumber);
		returnErrorCode(ERR_SECTION);
	}
}

void Assembler::createRelocation(uint8_t symbol, std::string type, char operation) {
	relocationTable[currentSection].push_back( { locationCounter, type, operation, symbol });
}

void Assembler::addGlobal(std::string expression) {
	auto symbols = parserComma(expression);
	for (auto symbol : symbols) {
		if (checkSymbolIsLiteral(symbol)) {
			logger("Symbol is literal, cannot be global, at line ",readingLineNumber);
			returnErrorCode(ERR_REDEFINITION);
		}
		if (checkSymbolExists(symbol)) {
			if(checkSymbolIsExtern(symbol)) {
				logger("Symbol cannot be extern and global, at line ",readingLineNumber);
				returnErrorCode(ERR_UNDEFINED_SYMBOL);
			}
			symbolTable[symbol].type = "global";
		} else {
			++symbolNumber;
			symbolTable.insert( { symbol, { symbolNumber, UNDEFINED_SECTION, 0,
					"global", 0, "label" } });
		}
	}
}

void Assembler::addExtern(std::string expression) {
	auto symbols = parserComma(expression);
	for (auto symbol : symbols) {
		if (checkSymbolIsLiteral(symbol)) {
			logger(
					"Symbol is literal, cannot be extern, at line ",readingLineNumber);
			returnErrorCode(ERR_REDEFINITION);
		}
		if (checkSymbolExists(symbol)) {
			logger("Symbol declared extern already exists in symbol table, error in line ",readingLineNumber);
			returnErrorCode(ERR_MULTIPLE_DEFINITIONS);
		}
		++symbolNumber;
		symbolTable.insert( { symbol, { symbolNumber, UNDEFINED_SECTION, 0,
				"extern", 0, "label" } });
	}
}

void Assembler::resolveSymbol(std::string symbol) {
	if (symbol == "") {
		return;
	}
	if (checkSymbolIsLiteral(symbol)) {
		logger("Multiple definitions of symbol at line ",readingLineNumber);
		returnErrorCode(ERR_MULTIPLE_DEFINITIONS);
	}
	if (checkSymbolExists(symbol)) {
		if (checkSymbolIsDefined(symbol) || checkSymbolIsExtern(symbol)) {
			logger("Multiple definitions or symbol is extern, symbol at line ",readingLineNumber);
			returnErrorCode(ERR_MULTIPLE_DEFINITIONS);
		} else {
			defineLabel(symbol);
		}
	} else {
		addNewLabel(symbol);
	}
}

void Assembler::calculateExpression(std::string lit, char operation, std::string operand) {
	auto& literal = literalTable[lit];
	if (operand[0] >= '0' && operand[0] <= '9') {
		 literal.value += (operation == ADD) ? toInt16_t(operand) : 0 - toInt16_t(operand);
	} else {
		if(checkSymbolIsLiteral(operand)) {
			logger("Equ defined literal used in equ definition at line ",readingLineNumber);
			returnErrorCode(ERR_SYNTAX);
		}
		if(checkSymbolExists(operand)) {
			if(checkSymbolIsExtern(operand) || checkSymbolIsGlobal(operand)) {
				literal.relocations.push_back({symbolTable[operand].number, operation, R_16});
			} else {
				if(checkSymbolIsDefined(operand)) {
					literal.value += (operation == ADD) ? symbolTable[operand].offset : 0 - symbolTable[operand].offset;
					literal.relocations.push_back({symbolTable[operand].sectionNumber, operation, R_16});
				} else {
					logger("Error, symbol undefined in equ definition for " + lit);
					returnErrorCode(ERR_UNDEFINED_SYMBOL);
				}
			}
		} else {
		logger("Error, symbol undefined in equ definition for " + lit);
		returnErrorCode(ERR_UNDEFINED_SYMBOL);
		}
	}
}


void Assembler::calculateLiteral(std::string literal) {
	auto& literalEntry = literalTable[literal];
	auto expression = literalEntry.expression;
	expression.erase(std::remove(expression.begin(), expression.end(),' '), expression.end());
	char operation = ADD;
	while(expression.length()) {
		if(expression[0] == '+' || expression[0] == '-') {
			operation = expression[0];
			expression.erase(0,1);
		}
		auto positionPlus = expression.find_first_of("+");
		auto positionMinus = expression.find_first_of("-");
		if(positionPlus == std::string::npos) {
			if(positionMinus == std::string::npos) {
				calculateExpression(literal, operation, expression);
				break;
			} else {
				auto operand = expression.substr(0, positionMinus);
				calculateExpression(literal, operation, operand);
				expression.erase(0, positionMinus);
			}
		} else {
			if(positionMinus == std::string::npos) {
				auto operand = expression.substr(0, positionPlus);
				calculateExpression(literal, operation, operand);
				expression.erase(0, positionPlus);
			} else {
				auto min = std::min(positionMinus, positionPlus);
				auto operand = expression.substr(0, min);
				calculateExpression(literal, operation, operand);
				expression.erase(0, min);
			}
		}
	}
}


int16_t Assembler::toInt16_t(std::string str) {
	int val = 0;
	if(str[0] == '*') {
		str.erase(0, 1);
	}
	if (str[0] == '0') {
		if (str[1] == 'b')
			val = stoi(str, nullptr, 2);
		if (str[1] == 'o')
			val = stoi(str, nullptr, 8);
		if (str[1] == 'x')
			val = stoi(str, nullptr, 16);
	} else {
		val = stoi(str, nullptr);
	}

	if(val > 65535) {
		logger("Too large value used in word directive at line ",readingLineNumber);
		returnErrorCode(ERR_ARGUMENT);
	}
	return (int16_t) val;
}

int8_t Assembler::toInt8_t(std::string str) {
	int val = 0;
	if(str[0] == '*') {
		str.erase(0, 1);
	}
	if (str[0] == '0') {
		if (str[1] == 'b')
			val = stoi(str, nullptr, 2);
		if (str[1] == 'o')
			val = stoi(str, nullptr, 8);
		if (str[1] == 'x')
			val = stoi(str, nullptr, 16);
	} else {
		val = stoi(str, nullptr);
	}

	if(val > 255) {
		logger("Too large value used in byte directive at line ",readingLineNumber);
		returnErrorCode(ERR_ARGUMENT);
	}

	return (int8_t)val;
}

void Assembler::validateRegex() {
	auto i = 0;
	for (; i < numberOfRegex; i++) {
		if (std::regex_search(readLine, matches, *allTheRegex[i].regex)) {
			decypherRegex(i);
			break;
		}
	}
	if (i == numberOfRegex) {
		logger("Bad syntax in input file at line ",readingLineNumber);
		returnErrorCode(ERR_SYNTAX);
	}
}

void Assembler::createBackpatchEntry(std::string symbol, char operation,
		uint8_t bytes, std::string relocationType) {
	TII[symbol].push_back( { currentSectionSymbolNumber, locationCounter,
			operation, bytes, relocationType });
}

int Assembler::autoRelocation(std::string symbol, char operation, std::string relocationType) {
	int value = 0;
	if (checkSymbolIsLiteral(symbol)) {
		createBackpatchEntry(symbol, operation, 2, LITERAL);
	} else {
		if (checkSymbolExists(symbol)) {
			if(checkSymbolIsExtern(symbol) || checkSymbolIsGlobal(symbol)) {
				if(checkSymbolIsGlobal(symbol) && relocationType == R_PC16 && symbolTable[symbol].sectionNumber == currentSectionSymbolNumber) {
					value = symbolTable[symbol].offset - locationCounter;
				} else {
					createRelocation(symbolTable[symbol].number, relocationType, operation);
				}
			} else {
				if(checkSymbolIsDefined(symbol)) {
					if(relocationType != R_PC16 || symbolTable[symbol].sectionNumber != currentSectionSymbolNumber){
						value = symbolTable[symbol].offset;
						createRelocation(symbolTable[symbol].sectionNumber, relocationType, operation);
					} else {
						value = symbolTable[symbol].offset - locationCounter;
					}
				} else {
					createBackpatchEntry(symbol, operation, 2, relocationType);
				}
			}
		} else {
			addUndefinedSymbol(symbol);
			createBackpatchEntry(symbol, operation, 2, relocationType);
		}
	}

	return value;
}

void Assembler::decypherRegex(int i) {
	switch (i) {
	case regexComment:
		break;

	case regexLabel:
	{
		checkSection();
		auto label = get(LABEL);
		resolveSymbol(label);
	}
		break;

	case regexSection:
	{
		auto section = get(SECTION);
		if (currentSection != "UNDEFINED") {
			symbolTable[currentSection].size = locationCounter;
			sectionTable[currentSection].sectionSize = locationCounter;
		}

		locationCounter = 0;

		if (section == "end") {
			foundEnd = true;
			return;
		}

		if (checkSymbolIsLiteral(section)) {
			logger("Multiple definitions of symbol at line ",readingLineNumber);
			returnErrorCode(ERR_MULTIPLE_DEFINITIONS);
		}
		if (checkSymbolExists(section)) {
			if (checkSymbolIsDefined(section)) {
				logger("Multiple definitions of section at line ",readingLineNumber);
				returnErrorCode(ERR_MULTIPLE_DEFINITIONS);
			}
			auto& symbol = symbolTable.at(section);
			symbol.sectionNumber = symbol.number;
			symbol.offset = 0;
			symbol.type = "local";
			symbol.symbolType = "section";
			sectionTable.insert( { section, { 0, symbol.number } });
			sectionTranslation.insert({symbol.number, section});
			currentSectionSymbolNumber = symbol.number;
		} else {
			++symbolNumber;
			symbolTable.insert( { section, { symbolNumber, symbolNumber, locationCounter, "local", 0, "section" } });
			sectionTable.insert( { section, { 0, symbolNumber } });
			sectionTranslation.insert( {symbolNumber, section});
			currentSectionSymbolNumber = symbolNumber;
		}
		currentSection = section;
	}
		break;

	case regexEqu:
	{
		if (currentSection != "UNDEFINED") {
			logger("Error: out of section .equ only, line ",readingLineNumber);
			returnErrorCode(ERR_SECTION);
		}
		auto symbol = get(SYMBOL);
		if (checkSymbolIsLiteral(symbol) || checkSymbolExists(symbol)) {
			logger("EQU defined symbol already exists at line ",readingLineNumber);
			returnErrorCode(ERR_MULTIPLE_DEFINITIONS);
		}
		literalTable.insert( { symbol, { get(EXPRESSION), 0 } });
	}
		break;

	case regexGlobal:
	{
		if (currentSection != "UNDEFINED") {
			logger("Error: out of section .global only, line ",readingLineNumber);
			returnErrorCode(ERR_SECTION);
		}
		addGlobal(get(SYMBOL));
	}
		break;

	case regexExtern:
	{
		if (currentSection != "UNDEFINED") {
			logger("Error: out of section .extern only, line ",readingLineNumber);
			returnErrorCode(ERR_SECTION);
		}
		addExtern(get(SYMBOL));
	}
		break;

	case regexByte:
	{
		checkSection();
		auto symbol = get(SYMBOL);
		auto symbols = get(LIST);
		resolveSymbol(symbol);
		auto symbolVector = parserComma(symbols);
		for (auto sym : symbolVector) {
			int8_t value = 0;
			char operation = '+';
			if (sym[0] == '-') {
				operation = '-';
				sym.erase(0, 1);
			}
			if (sym[0] >= '0' && sym[0] <= '9') {
				value = toInt8_t(sym);
				if(operation == '-' && value == INT8_T_MIN) {
					logger("Overflow value at byte directive, line number ",readingLineNumber);
					returnErrorCode(ERR_ARGUMENT);
				}
				value = (operation == '+') ? value : 0 - value;
			} else {
				if (literalTable.find(sym) != literalTable.end()) {
					createBackpatchEntry(sym, operation, 1, LITERAL);
				} else {
					logger("Error, unavailable symbol in byte directive, line ",readingLineNumber);
					returnErrorCode(ERR_SYNTAX);
				}
			}
			machineCode[currentSectionSymbolNumber].push_back(value);
			locationCounter++;
		}
	}
		break;

	case regexWord:
	{
		checkSection();
		auto symbol = get(SYMBOL);
		auto symbols = get(LIST);
		resolveSymbol(symbol);
		auto symbolVector = parserComma(symbols);
		for (auto sym : symbolVector) {
			int16_t value = 0;
			char operation = '+';
			if (sym[0] == '-') {
				operation = '-';
				sym.erase(0, 1);
			}
			if (sym[0] >= '0' && sym[0] <= '9') {
				value = toInt16_t(sym);
				if(operation == '-' && value == INT16_T_MIN) {
					logger("Overflow value at word directive, line number ",readingLineNumber);
					returnErrorCode(ERR_ARGUMENT);
				}
				value = (operation == ADD) ? value : 0 - value;
			} else {
				value = autoRelocation(sym, operation, R_16);
			}
			union ImmedValues val;
			val.val = value;
			machineCode[currentSectionSymbolNumber].push_back(val.byte1);
			machineCode[currentSectionSymbolNumber].push_back(val.byte2);
			locationCounter += 2;
		}
	}
		break;

	case regexSkip: {

		checkSection();
		auto symbol = get(SYMBOL);
		resolveSymbol(symbol);
		auto expr = get(EXPRESSION);
		int16_t value = toInt16_t(expr);
		if (value < 0) {
			logger("Negative value in skip at line ",readingLineNumber);
			returnErrorCode(ERR_SYNTAX);
		}
		for(auto i = 0; i < value; i++) {
		machineCode[currentSectionSymbolNumber].push_back(0x90);
		}
		locationCounter += value;
	}
		break;

	case regexInstrNoOperand:
	{
		checkSection();
		auto symbol = get(SYMBOL);
		auto instruction = get(OPERATION);
		resolveSymbol(symbol);

		union Mnemonics code;
		code.val = 0;
		code.opcode = MAPS::opCode[instruction];
		code.size = 0;
		machineCode[currentSectionSymbolNumber].push_back(code.val);
		++locationCounter;
	}
		break;

	case regexInstrOneOperand:
	{
		checkSection();
		auto symbol = get(SYMBOL);
		auto instruction = get(OPERATION);
		resolveSymbol(symbol);
		auto argument1 = get(ARG1);

		union Mnemonics mnemonic;
		mnemonic.val = 0;
		mnemonic.opcode = MAPS::opCode[instruction];
		machineCode[currentSectionSymbolNumber].push_back(mnemonic.val);
		locationCounter++;

		union Addressing addr;
		addr.val = 0;
		union ImmedValues oper;
		oper.val = 0;

		std::string addrMode = "";

		std::smatch operand;

		/*
		 * (1) *0xff(%r0), *0xff, 0xff
		 * (2) *labela1(%r0), *labela2, labela3
		 * (3) *%r0
		 * (4) *%(r0)
		 */

		if (isJump(instruction)) {
			std::regex_search(argument1, operand, operandJumpRegex);
			for (int i = 1; i < 5; i++) {
				if (operand.str(i) != "") {
					std::string jumpAddr = operand.str(i);
					switch (i) {

					// *0xff(%r0), *0xff, 0xff
					case 1:
						if (jumpAddr[0] == '*') {
							jumpAddr.erase(0, 1);
							auto position = jumpAddr.find("(", 0);
							if (position == std::string::npos) {
								addrMode = MEMDIR;
								addr.addressMode = MAPS::addressingMode[MEMDIR];
								locationCounter++;
								oper.val = toInt16_t(jumpAddr);
								locationCounter += 2;
							} else {
								auto operand1literal = jumpAddr.substr(0, position);
								jumpAddr.erase(0, operand1literal.length() + 2);
								jumpAddr.erase(jumpAddr.length() - 1, 1);
								auto operand1reg = jumpAddr;
								addrMode = REGIND16B;
								addr.addressMode = MAPS::addressingMode[REGIND16B];
								addr.regs = MAPS::regs[operand1reg];
								locationCounter++;
								oper.val = toInt16_t(operand1literal);
								locationCounter += 2;
							}
						} else {
							addrMode = IMMED;
							addr.addressMode = MAPS::addressingMode[IMMED];
							locationCounter++;
							oper.val = toInt16_t(jumpAddr);
							locationCounter += 2;
						}
						machineCode[currentSectionSymbolNumber].push_back(addr.val);
						machineCode[currentSectionSymbolNumber].push_back(oper.byte1);
						machineCode[currentSectionSymbolNumber].push_back(oper.byte2);
						break;

					// *labela1(%r0), *labela2, labela3
					case 2:
						if (jumpAddr[0] == '*') {
							jumpAddr.erase(0, 1);
							auto position = jumpAddr.find('(', 0);
							if (position == std::string::npos) {
								addrMode = MEMDIR;
								addr.addressMode = MAPS::addressingMode[MEMDIR];
								locationCounter++;
								oper.val = autoRelocation(jumpAddr, ADD, R_16);
								locationCounter += 2;
							} else {
								auto operand1label = jumpAddr.substr(0, position);
								jumpAddr.erase(0, operand1label.length() + 2);
								jumpAddr.erase(jumpAddr.length() - 1, 1);
								auto operand1reg = jumpAddr;
								addrMode = REGIND16B;
								addr.addressMode = MAPS::addressingMode[REGIND16B];
								addr.regs = MAPS::regs[operand1reg];
								locationCounter++;
								if (operand1reg == "pc" || operand1reg == "r7") {
									if (checkSymbolIsLiteral(operand1label)) {
										oper.val = autoRelocation(operand1label, ADD, R_16);
									} else {
										oper.val = -2;
										oper.val = oper.val + autoRelocation(operand1label, ADD, R_PC16);
									}
									locationCounter += 2;
								} else {
									oper.val = autoRelocation(operand1label, ADD, R_16);
									locationCounter += 2;
								}
							}
						} else {
							addrMode = IMMED;
							addr.addressMode = MAPS::addressingMode[IMMED];
							locationCounter++;
							oper.val = autoRelocation(jumpAddr, ADD, R_16);
							locationCounter += 2;
						}
						machineCode[currentSectionSymbolNumber].push_back(addr.val);
						machineCode[currentSectionSymbolNumber].push_back(oper.byte1);
						machineCode[currentSectionSymbolNumber].push_back(oper.byte2);
						break;

					// *%r0
					case 3:
						jumpAddr.erase(0, 2);
						addrMode = REGDIR;
						addr.addressMode = MAPS::addressingMode[REGDIR];
						addr.regs = MAPS::regs[jumpAddr];
						locationCounter++;
						machineCode[currentSectionSymbolNumber].push_back(addr.val);
						break;

					// *%(r0)
					case 4:
						jumpAddr.erase(0, 3);
						jumpAddr.erase(jumpAddr.length() - 1, 1);
						addrMode = REGIND;
						addr.addressMode = MAPS::addressingMode[REGIND];
						addr.regs = MAPS::regs[jumpAddr];
						locationCounter++;
						machineCode[currentSectionSymbolNumber].push_back(addr.val);
						break;
					}
					break;
				}
			}

		} else {	// push pop
			std::regex_search(argument1, operand, operandInstructionRegex);
			for (int i = 1; i < 5; i++) {
				if (operand.str(i) != "") {
					std::string operand1 = operand.str(i);
					switch(i) {
					// 0xff(%r0), $0xff, 0xff
					case 1:
						if(operand1[0] == '$' && operand1.find('(', 0) != std::string::npos) {
							logger("Bad operand format, $literal(%r<num) at line ",readingLineNumber);
							returnErrorCode(ERR_ARGUMENT);
						}
						if(operand1[0] == '$') {
							operand1.erase(0, 1);
							addrMode = IMMED;
							addr.addressMode = MAPS::addressingMode[IMMED];
							locationCounter++;
							oper.val = toInt16_t(operand1);
							locationCounter += 2;
						} else {
							auto position = operand1.find('(',0);
							if(position == std::string::npos) {
								addrMode = MEMDIR;
								addr.addressMode = MAPS::addressingMode[MEMDIR];
								locationCounter++;
								oper.val = toInt16_t(operand1);
								locationCounter+= 2;
							} else {
								auto operand1Literal = operand1.substr(0, position);
								operand1.erase(0, operand1Literal.length() + 2);
								operand1.erase(operand1.length() - 1, 1);
								auto operand1Reg = operand1;
								addrMode = REGIND16B;
								addr.addressMode = MAPS::addressingMode[REGIND16B];
								addr.regs = MAPS::regs[operand1Reg];
								locationCounter++;
								oper.val = toInt16_t(operand1Literal);
								locationCounter += 2;
							}
						}
						break;

					// labela1(%r0), $labela2, labela3
					case 2:
						if(operand1[0] == '$' && operand1.find('(', 0) != std::string::npos) {
							logger("Bad operand format, $symbol(%r<num) at line ",readingLineNumber);
							returnErrorCode(ERR_ARGUMENT);
						}
						if(operand1[0] == '$') {
							operand1.erase(0, 1);
							addrMode = IMMED;
							addr.addressMode = MAPS::addressingMode[IMMED];
							locationCounter++;
							oper.val = autoRelocation(operand1, ADD, R_16);
							locationCounter += 2;
						} else {
							auto position = operand1.find('(',0);
							if(position == std::string::npos) {
								addrMode = MEMDIR;
								addr.addressMode = MAPS::addressingMode[MEMDIR];
								locationCounter++;
								oper.val = autoRelocation(operand1, ADD, R_16);
								locationCounter += 2;
							} else {
								auto operand1Label = operand1.substr(0, position);
								operand1.erase(0, operand1Label.length() + 2);
								operand1.erase(operand1.length() - 1, 1);
								auto operand1Reg = operand1;
								addrMode = REGIND16B;
								addr.addressMode = MAPS::addressingMode[REGIND16B];
								addr.regs = MAPS::regs[operand1Reg];
								locationCounter++;
								if(operand1Reg == "pc" || operand1Reg == "r7") {
									if(checkSymbolIsLiteral(operand1Label)) {
										oper.val = autoRelocation(operand1Label, ADD, LITERAL);
									} else {
										oper.val = -2;
										oper.val = oper.val + autoRelocation(operand1Label, ADD, R_PC16);
									}
								} else {
									oper.val = autoRelocation(operand1Label, ADD, R_16);
								}
								locationCounter += 2;
							}
						}
						break;

					// %r0
					case 3:
						operand1.erase(0, 1);
						addrMode = REGDIR;
						addr.addressMode = MAPS::addressingMode[REGDIR];
						addr.regs = MAPS::regs[operand1];
						locationCounter++;
						break;

					// %(r0)
					case 4:
						operand1.erase(0, 2);
						operand1.erase(operand1.length() - 1, 1);
						addrMode = REGIND;
						addr.addressMode = MAPS::addressingMode[REGIND];
						addr.regs = MAPS::regs[operand1];
						locationCounter++;
						break;
					}
					break;
				}
			}

			if(instruction == "pop" && addrMode == IMMED) {
				logger("Pop + immed illegal combination, line number ",readingLineNumber);
				returnErrorCode(ERR_ARGUMENT);
			}

			machineCode[currentSectionSymbolNumber].push_back(addr.val);

			if(addrMode == IMMED || addrMode == REGIND16B || addrMode == MEMDIR) {
				machineCode[currentSectionSymbolNumber].push_back(oper.byte1);
				machineCode[currentSectionSymbolNumber].push_back(oper.byte2);
			}
		}
	}
		break;

	case regexInstrTwoOperand:
	{
		checkSection();
		auto symbol = get(SYMBOL);
		resolveSymbol(symbol);
		auto instruction = get(OPERATION);
		auto argument1 = get(ARG1);
		auto argument2 = get(ARG2);
		std::smatch operand;
		bool isPcRel = false;
		std::regex_search(argument1, operand, operandInstructionRegex);
		union Mnemonics mnemonic;
		mnemonic.val = 0;
		mnemonic.opcode = MAPS::opCode[instruction];
		auto operandSize = MAPS::operandSize[instruction];
		mnemonic.size = operandSize;
		machineCode[currentSectionSymbolNumber].push_back(mnemonic.val);
		locationCounter++;

		// **************************************************
		// Operand 1
		// **************************************************

		union Addressing addr1;
		addr1.val = 0;
		union ImmedValues oper1;
		oper1.val = 0;
		std::string addr1Mode = "";

		/*
		 * (1) 0xff(%r0), $0xff, 0xff
		 * (2) labela1(%r0), $labela2, labela3
		 * (3) %r0
		 * (4) %(r0)
		 */
		for (int i = 1; i < 5; i++) {
			if (operand.str(i) != "") {
				std::string operand1 = operand.str(i);
				switch(i) {
				// 0xff(%r0), $0xff, 0xff
				case 1:
					if(operand1[0] == '$' && operand1.find('(', 0) != std::string::npos) {
						logger("Bad operand format, $literal(%r<num) at line ",readingLineNumber);
						returnErrorCode(ERR_ARGUMENT);
					}
					if(operand1[0] == '$') {
						operand1.erase(0, 1);
						addr1Mode = IMMED;
						addr1.addressMode = MAPS::addressingMode[IMMED];
						locationCounter++;
						if(operandSize) {
							oper1.val = toInt16_t(operand1);
							locationCounter += 2;
						} else {
							oper1.val = toInt8_t(operand1);
							locationCounter++;
						}
					} else {
						auto position = operand1.find('(',0);
						if(position == std::string::npos) {
							if(operand1[0] == '-') {
								logger("Negative address at line ",readingLineNumber);
								returnErrorCode(ERR_SYNTAX);
							}
							addr1Mode = MEMDIR;
							addr1.addressMode = MAPS::addressingMode[MEMDIR];
							locationCounter++;
							oper1.val = toInt16_t(operand1);
							locationCounter+= 2;
						} else {
							auto operand1Literal = operand1.substr(0, position);
							operand1.erase(0,operand1Literal.length() + 2);
							operand1.erase(operand1.length() - 1, 1);
							auto operand1Reg = operand1;
							addr1Mode = REGIND16B;
							addr1.addressMode = MAPS::addressingMode[REGIND16B];
							addr1.regs = MAPS::regs[operand1Reg];
							locationCounter++;
							oper1.val = toInt16_t(operand1Literal);
							locationCounter += 2;
						}
					}
					break;

				// labela1(%r0), $labela2, labela3
				case 2:
					if(operand1[0] == '$' && operand1.find('(', 0) != std::string::npos) {
						logger("Bad operand format, $symbol(%r<num>) at line ",readingLineNumber);
						returnErrorCode(ERR_ARGUMENT);
					}
					if(operand1[0] == '$') {
						operand1.erase(0, 1);
						addr1Mode = IMMED;
						addr1.addressMode = MAPS::addressingMode[IMMED];
						locationCounter++;
						oper1.val = autoRelocation(operand1, ADD, R_16);
						locationCounter += 2;
					} else {
						auto position = operand1.find('(',0);
						if(position == std::string::npos) {
							addr1Mode = MEMDIR;
							addr1.addressMode = MAPS::addressingMode[MEMDIR];
							locationCounter++;
							oper1.val = autoRelocation(operand1, ADD, R_16);
							locationCounter += 2;
						} else {
							auto operand1Label = operand1.substr(0, position);
							operand1.erase(0, operand1Label.length() + 2);
							operand1.erase(operand1.length() - 1, 1);
							auto operand1Reg = operand1;
							addr1Mode = REGIND16B;
							addr1.addressMode = MAPS::addressingMode[REGIND16B];
							addr1.regs = MAPS::regs[operand1Reg];
							locationCounter++;
							if(operand1Reg == "pc" || operand1Reg == "r7") {
								if(checkSymbolIsLiteral(operand1Label)) {
									oper1.val = autoRelocation(operand1Label, ADD, LITERAL);
								} else {
									isPcRel = true;
									oper1.val = autoRelocation(operand1Label, ADD, R_PC16);
								}
							} else {
								oper1.val = autoRelocation(operand1Label, ADD, R_16);
							}
							locationCounter += 2;
						}
					}
					break;

				// %r0
				case 3:
					operand1.erase(0, 1);
					addr1Mode = REGDIR;
					addr1.addressMode = MAPS::addressingMode[REGDIR];
					addr1.regs = MAPS::regs[operand1];
					if(operandSize == 0) {
						addr1.part = (operand1[operand1.length() - 1] == 'l') ? 0 : 1;
					}
					locationCounter++;
					break;

				// %(r0)
				case 4:
					operand1.erase(0, 2);
					operand1.erase(operand1.length() - 1, 1);
					addr1Mode = REGIND;
					addr1.addressMode = MAPS::addressingMode[REGIND];
					addr1.regs = MAPS::regs[operand1];
					locationCounter++;
					break;
				}
				break;
			}
		}

		// **************************************************
		// Operand 2
		// **************************************************

		std::regex_search(argument2, operand, operandInstructionRegex);

		union Addressing addr2;
		addr2.val = 0;
		union ImmedValues oper2;
		oper2.val = 0;
		std::string addr2Mode = "";

		/*
		 * (1) 0xff(%r0), $0xff, 0xff
		 * (2) labela1(%r0), $labela2, labela3
		 * (3) %r0
		 * (4) %(r0)
		 */

		for(int i = 1; i < 5; i++) {
			if(operand.str(i) != "") {
				std::string operand2 = operand.str(i);
				switch(i) {
				// 0xff(%r0), $0xff, 0xff
				case 1:
					if(operand2[0] == '$' && operand2.find('(', 0) != std::string::npos) {
						logger("Bad operand format, $literal(%r<num>) at line ",readingLineNumber);
						returnErrorCode(ERR_ARGUMENT);
					}
					if(operand2[0] == '$') {
						operand2.erase(0, 1);
						addr2Mode = IMMED;
						addr2.addressMode = MAPS::addressingMode[IMMED];
						locationCounter++;
						if(operandSize) {
							oper2.val = toInt16_t(operand2);
							locationCounter += 2;
						} else {
							oper2.val = toInt8_t(operand2);
							locationCounter++;
						}
					} else {
						auto position = operand2.find('(',0);
						if(position == std::string::npos) {
							if(operand2[0] == '-') {
								logger("Negative address at line ",readingLineNumber);
								returnErrorCode(ERR_SYNTAX);
							}
							addr2Mode = MEMDIR;
							addr2.addressMode = MAPS::addressingMode[MEMDIR];
							locationCounter++;
							oper2.val = toInt16_t(operand2);
							locationCounter+= 2;
						} else {
							auto operand2Literal = operand2.substr(0, position);
							operand2.erase(0,operand2Literal.length() + 2);
							operand2.erase(operand2.length() - 1, 1);
							auto operand2Reg = operand2;
							addr2Mode = REGIND16B;
							addr2.addressMode = MAPS::addressingMode[REGIND16B];
							addr2.regs = MAPS::regs[operand2Reg];
							locationCounter++;
							oper2.val = toInt16_t(operand2Literal);
							locationCounter += 2;
						}
					}
					break;

				// labela1(%r0), $labela2, labela3
				case 2:

					if(operand2[0] == '$' && operand2.find('(', 0) != std::string::npos) {
						logger("Bad operand format, $symbol(%r<num) at line ",readingLineNumber);
						returnErrorCode(ERR_ARGUMENT);
					}
					if(operand2[0] == '$') {
						operand2.erase(0, 1);
						addr2Mode = IMMED;
						addr2.addressMode = MAPS::addressingMode[IMMED];
						locationCounter++;
						oper2.val = autoRelocation(operand2, ADD, R_16);
						locationCounter += 2;
					} else {
						auto position = operand2.find('(',0);
						if(position == std::string::npos) {
							addr2Mode = MEMDIR;
							addr2.addressMode = MAPS::addressingMode[MEMDIR];
							locationCounter++;
							oper2.val = autoRelocation(operand2, ADD, R_16);
							locationCounter += 2;
						} else {
							auto operand2Label = operand2.substr(0, position);
							operand2.erase(0, operand2Label.length() + 2);
							operand2.erase(operand2.length() - 1, 1);
							auto operand2Reg = operand2;
							addr2Mode = REGIND16B;
							addr2.addressMode = MAPS::addressingMode[REGIND16B];
							addr2.regs = MAPS::regs[operand2Reg];
							locationCounter++;
							if(operand2Reg == "pc" || operand2Reg == "r7") {
								if(checkSymbolIsLiteral(operand2Label)) {
									oper2.val = autoRelocation(operand2Label, ADD, LITERAL);
								} else {
									oper2.val = -2;
									oper2.val = oper2.val + autoRelocation(operand2Label, ADD, R_PC16);
								}
							} else {
								oper2.val = autoRelocation(operand2Label, ADD, R_16);
							}
							locationCounter += 2;
						}
					}
					break;

				// %r0
				case 3:
					operand2.erase(0, 1);
					addr2Mode = REGDIR;
					addr2.addressMode = MAPS::addressingMode[REGDIR];
					addr2.regs = MAPS::regs[operand2];
					if(operandSize == 0) {
						addr2.part = (operand2[operand2.length() - 1] == 'l') ? 0 : 1;
					}
					locationCounter++;
					break;

				// %(r0)
				case 4:
					operand2.erase(0, 2);
					operand2.erase(operand2.length() - 1, 1);
					addr2Mode = REGIND;
					addr2.addressMode = MAPS::addressingMode[REGIND];
					addr2.regs = MAPS::regs[operand2];
					locationCounter++;
					break;
				}
				break;
			}
		}

// proveri dozvoljena adresiranja sa instrukcijama, shr je jedino src, dst
		if(addr1Mode == IMMED && mnemonic.opcode == MAPS::opCode["shr"]) {
			logger("Illegal addressing for shr dst, src line number ",readingLineNumber);
			returnErrorCode(ERR_SYNTAX);
		}
		if(addr2Mode == IMMED && mnemonic.opcode != MAPS::opCode["shr"]) {
			logger("Illegal addressing IMMED for dst operand at line number ",readingLineNumber);
			returnErrorCode(ERR_SYNTAX);
		}

		if(isPcRel) {
			if(addr2Mode == IMMED) {
					oper1.val += (operandSize) ? -5 : -4;
			}

			if(addr2Mode == REGIND16B || addr2Mode == MEMDIR) {
					oper1.val += -5;
			}

			if(addr2Mode == REGDIR || addr2Mode == REGIND) {
					oper1.val += -3;
			}
		}

		machineCode[currentSectionSymbolNumber].push_back(addr1.val);

		if(addr1Mode == IMMED) {
			if(operandSize) {
				machineCode[currentSectionSymbolNumber].push_back(oper1.byte1);
				machineCode[currentSectionSymbolNumber].push_back(oper1.byte2);
			} else {
				machineCode[currentSectionSymbolNumber].push_back(oper1.signed8);
			}
		}
		if(addr1Mode == REGIND16B || addr1Mode == MEMDIR) {
			machineCode[currentSectionSymbolNumber].push_back(oper1.byte1);
			machineCode[currentSectionSymbolNumber].push_back(oper1.byte2);
		}

		machineCode[currentSectionSymbolNumber].push_back(addr2.val);

		if(addr2Mode == IMMED) {
			if(operandSize) {
				machineCode[currentSectionSymbolNumber].push_back(oper2.byte1);
				machineCode[currentSectionSymbolNumber].push_back(oper2.byte2);
			} else {
				machineCode[currentSectionSymbolNumber].push_back(oper2.signed8);
			}
		}
		if(addr2Mode == REGIND16B || addr2Mode == MEMDIR) {
			machineCode[currentSectionSymbolNumber].push_back(oper2.byte1);
			machineCode[currentSectionSymbolNumber].push_back(oper2.byte2);
		}
	}
		break;
	}
}

