#ifndef _assembler_hpp_
#define _assembler_hpp_

class Assembler {
public:
    Assembler();
    ~Assembler();

    void generateObj();
    
private:
    void assemble();
    void backpatch();
};

#endif