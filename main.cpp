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
		std::cerr << "usage: as file1.s -o output.o" << std::endl;

		exit(1);
	}

	assembler->argumentsAnalyzer(argc, argv);

	assembler->generateObj();

	return 0;
}
