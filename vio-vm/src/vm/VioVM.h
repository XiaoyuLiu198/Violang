#include <string>
#include <vector>

#include "../Logger.h"
#include "../bytecode/OpCode.h"
#include "VioValue.h"

#define STACK_LIMIT = 512

//  read current byte in the bytecode and increment pointer by one
#define READ_BYTE() *ip++

//  get constant from pool
#define GET_CONST() constants[READ_BYTE()]

class VioVM {
    public:
        VioVM() {}
    
    // push value to stack
    void push(VioValue value) {
        if ((size_t)(sp - stack.begin()) == STACK_LIMIT){
            DIE << "push(): stack overflow.\n";
        }
        *sp = value;
        sp++;
    }

    // pop from stack
    void pop() {
        if (sp == stack.begin()) {
            DIE << "pop(): empty stack.\n";
        }
        --sp;
        return *sp;
    }

    // execute
    void exec(const std::string& program) {
        // 1. parse the input program

        // 2. compile to machine bytecode
        constants.push_back(NUMBER(100));
        code = {OP_CONST, 0, OP_HALT};

        // 3. set instruction pointer to the begining
        ip = &code[0];

        // 4. initiate the stack
        sp = &stack[0];

        return eval();
    }

    // Eval
    VioValue eval(){
        for(;;) {
            auto opcode = READ_BYTE();
            switch (opcode){
                case OP_HALT:
                    return pop();

                case OP_CONST:
                    auto constIndex = READ_BYTE();
                    auto constant = constants[constIndex];
                    push(GET_CONST());
                    break;

                default:
                    DIE << "Unknown opcode: " << std::hex << opcode;
            }   
        }
    }

    uint8_t* ip;
    VioValue* sp;

    //  Operands stack
    std::array<VioValue, STACK_LIMIT> stack;

    //  constant pool
    std::vector<VioValue> constants;

}