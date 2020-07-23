#ifndef _auxiliary_hpp_
#define _auxiliary_hpp_

#include <cstdint>
#include <string>

constexpr std::string   IMMED="immed",
                        REGDIR="regdir",
                        REGIND="regind",
                        REGIND16B="regind16b",
                        MEMDIR="memdir";

constexpr uint8_t   r0 =    0x0,
                    r1 =    0x1,
                    r2 =    0x2,
                    r3 =    0x3,
                    r4 =    0x4,
                    r5 =    0x5,
                    r6 =    0x6,
                    sp =    0x6,
                    r7 =    0x7,
                    pc =    0x7,
                    psw =   0xf;

namespace MAPS {
    std::unordered_map<std::string, uint8_t> opCode = {
        {"halt", 0x0}, {"iret", 0x1}, {"ret", 0x2}, {"int", 0x3}, {"call", 0x4}, {"jmp", 0x5}, {"jeq", 0x6}, {"jne", 0x7}, {"jgt", 0x8}, {"push", 0x9}, {"pop", 0xa}, 
        {"xchg", 0xb}, {"xchgw", 0xb}, {"xchgb", 0xb}, {"mov", 0xc}, {"movw", 0xc}, {"movb", 0xc}, {"add", 0xd}, {"addw", 0xd}, {"addb", 0xd}, 
        {"sub", 0xe}, {"subw", 0xe}, {"subb", 0xe}, {"mul", 0xf}, {"mulw", 0xf}, {"mulb", 0xf}, {"div", 0x10}, {"divw", 0x10}, {"divb", 0x10}, 
        {"cmp", 0x11}, {"cmpw", 0x11}, {"cmpb", 0x11}, {"not", 0x12}, {"notw", 0x12}, {"notb", 0x12}, {"and", 0x13}, {"andw", 0x13}, {"andb", 0x13}, 
        {"or", 0x14}, {"orw", 0x14}, {"orb", 0x14}, {"xor", 0x15}, {"xorw", 0x15}, {"xorb", 0x15}, {"test", 0x16}, {"testw", 0x16}, {"testb", 0x16}, 
        {"shl", 0x17}, {"shlw", 0x17}, {"shlb", 0x17}, {"shr", 0x18}, {"shrw", 0x18}, {"shrb", 0x18} 
    };

    std::unordered_map<std::string, uint8_t> operandSize = {
        {"mov", 1}, {"movw", 1}, {"movb", 0}, {"xchg", 1}, {"xchgw", 1}, {"xchgb", 0}, 
        {"add",1}, {"addw", 1},{"addb", 0}, {"sub", 1},{"subw", 1},{"subb", 0}, 
        {"mul", 1},{"mulw", 1},{"mulb", 0}, {"push", 1}, {"pop", 1},
        {"div", 1}, {"divw", 1}, {"divb", 0}, {"cmp", 1}, {"cmpw", 1}, {"cmpb", 0}, 
        {"not", 1}, {"notw", 1}, {"notb", 0}, {"and", 1}, {"andw", 1}, {"andb", 0}, 
        {"or", 1}, {"orw", 1}, {"orb", 0}, {"xor", 1}, {"xorw", 1}, {"xorb", 0}, 
        {"test", 1}, {"testw", 1}, {"testb", 0}, {"shl", 1}, {"shlw", 1}, {"shlb", 0}, 
        {"shr", 1}, {"shrw", 1}, {"shrb", 0}
    };

    std::unordered_map<std::string, uint8_t> addressingMode = {
        {"immed", 0x0}, {"regdir", 0x1}, {"regind", 0x2}, {"regind16b", 0x3}, {"memdir", 0x4}
    };
}


// First byte of an instruction (operation code :5, operand size [1 or 2 bytes]:1 , unused :2)
union Mnemonics {
    struct {
        uint8_t val;
    };
    struct {
        uint8_t :2, size:1, opcode:5;
    };
};
// Second byte (addressing type :3, )
union Addressing {
    struct {
        uint8_t val;
    };
    struct {
        uint8_t size:1, regs:4, addressMode:3;
    };
};


enum RegexTypes {
/*0*/    regexComment,               // 1: #komentar 
/*1*/    regexLabel,                 // 1: naziv_labele
/*2*/    regexSection,               // 1: .naziv sekcije
/*3*/    regexEqu,                   // 1: naziv_simbola 2:ceo izraz
/*4*/    regexGlobal,                // 1: naziv_simbola [, vise njih]
/*5*/    regexExtern,                // 1: naziv_simbola [vise njih]
/*6*/    regexByte,                  // 1: [labela] 2: niz izraza odvojenih zarezom
/*7*/    regexWord,                  // 1: [labela] 2: niz izraza odvojenih zarezom
/*8*/    regexSkip,                  // 1: [labela] 2: koliko bajtova skip u dekadnom
/*9*/   regexInstrNoOperand,        // 1: [labela] 2: instr
/*10*/   regexInstrOneOperand,       // 1: [labela] 2: instr 3: literal, simbol
/*11*/   regexInstrTwoOperand        // 1: [labela] 2: instr 3: izraz operanda / registar kod regdir 4: '[' 5: registar 6:']' 7: [izraz pomeraja] 8: izraz operanda / registar kod regdir 9: '[' a: registar b: ']' c: [izraz pomeraja]
};

constexpr uint8_t numberOfRegex = 12;

const char *regexes[] = {
				"^[ 	]*(?:#.*)*$",
				"^[ 	]*([a-zA-Z_][a-zA-Z_0-9]*):[ 	]*(?:#.*)*$",
				"^(?:\\.section[ 	]+)?(\\.[a-zA-Z]+)[ 	]*(?:#.*)*$",
				"^\\.equ[ 	]+([a-z_A-Z][a-zA-Z0-9_]*),[ 	]*([\\+-]?[0-9A-Fa-f_]+(?:[\\+-][0-9A-Fa-f_]+)*)[ 	]*(?:#.*)*$",
				"^\\.global[ 	]+((?:[a-zA-Z_][a-zA-Z_0-9]*)(?:,[a-zA-Z_][a-zA-Z_0-9]*)*)[ 	]*(?:#.*)*$",
				"^\\.extern[ 	]+((?:[a-zA-Z_][a-zA-Z_0-9]*)(?:,[a-zA-Z_][a-zA-Z_0-9]*)*)[ 	]*(?:#.*)*$",
				"^[ 	]*(?:([a-zA-Z_][a-zA-Z_0-9]*):)?[ 	]*\\.byte[ 	]+((?:-?[a-zA-Z_0-9]+)(?:,-?[a-zA-Z_0-9]+)*)[ 	]*(?:#.*)*$",
				"^[ 	]*(?:([a-zA-Z_][a-zA-Z_0-9]*):)?[ 	]*\\.word[ 	]+((?:-?[a-zA-Z_0-9]+)(?:,-?[a-zA-Z_0-9]+)*)[ 	]*(?:#.*)*$",
				"^[ 	]*(?:([a-zA-Z_][a-zA-Z_0-9]*):)?[ 	]*\\.skip[ 	]+((?:0x)?[1-9a-fA-F][0-9a-fA-F]*)[ 	]*(?:#.*)*$",
				"^[ 	]*(?:([a-zA-Z_][a-zA-Z_0-9]*):)?[ 	]*(halt|iret|ret)[ 	]*(?:#.*)*$",
				"^[ 	]*(?:([a-zA-Z_][a-zA-Z_0-9]*):)?[ 	]*(int|call|jmp|jeq|jne|jgt|push|pop)[ 	]+((?:(?:\\*)?(?:0x)?[1-9a-fA-F][0-9a-fA-F]*(?:\\(%r[0-7]\\))?)|(?:(?:\\*)?[a-zA-Z_][a-zA-Z_0-9]*(?:\\(%(?:r[0-7]|pc)\\))?)|(?:\\*%r[0-7])|(?:\\*\\(%r[0-7]\\)))[ 	]*(?:#.*)*$",
                "^[ 	]*(?:([a-zA-Z_][a-zA-Z_0-9]*):)?[ 	]*(xchg[bw]?|not[bw]?|mov[bw]?|add[bw]?|sub[bw]?|mul[bw]?|div[bw]?|cmp[bw]?|and[bw]?|or[bw]?|xor[bw]?|test[bw]?|shl[bw]?|shr[bw]?)[ 	]+((?:%(?:r[0-7]|pc|sp)[lh]?)|(?:(?:(?:0x)?[1-9a-fA-F][0-9a-fA-F]*|[a-zA-Z_][a-zA-Z_0-9]*)?\\(%(?:r[0-7]|pc|sp)\\))|(?:\\$(?:0x)?[1-9a-fA-F][0-9a-fA-F]*)|(?:\\$[a-zA-Z_][a-zA-Z_0-9]*)),[ 	]*((?:%(?:r[0-7]|pc|sp)[lh]?)|(?:(?:(?:0x)?[1-9a-fA-F][0-9a-fA-F]*|[a-zA-Z_][a-zA-Z_0-9]*)?\\(%(?:r[0-7]|pc|sp)\\))|(?:(?:\\$)?(?:0x)?[1-9a-fA-F][0-9a-fA-F]*)|(?:(?:\\$)?[a-zA-Z_][a-zA-Z_0-9]*))[ 	]*(?:#.*)*$"
};

#endif