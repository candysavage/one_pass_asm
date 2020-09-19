#include <iostream>
#include <memory>
#include <utility>

#include "assembler.hpp"

int main(int argc, char *argv[]) {
	// Lonely object of Assembler class
	std::unique_ptr<Assembler> assembler(new Assembler());
	/**
	 * Check if the argument number is satisfying
	 **/
	if (argc != 4) {
		std::cerr << "*** INVALID ARGUMENT NUMBER ***" << std::endl;
		std::cerr << "usage: asm src.s -o obj.o" << std::endl;

		exit(1);
	}

	std::vector<std::string> args;
	args.push_back(std::string(argv[1]));
	args.push_back(std::string(argv[2]));
	args.push_back(std::string(argv[3]));

	assembler->argumentsAnalyzer(3, args);

	assembler->generateObj();

	return 0;
}
