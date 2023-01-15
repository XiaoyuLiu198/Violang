// compiler for Violang

#ifndef VioCompiler_h
#define VioCompiler_h
#include "VioValue.h"
#include "../bytecode/OpCode.h"
#include "../parser/VioParser.h"

#define ALLOC_CONST(tester, converter, allocator, value) \
    do {                                                 \
        for (auto i=0; i < co->constants.size(); i++){   \
            if(!tester(co->constants[i])){               \
                continue;                                \
            }                                            \
            if(converter(co->constants[i] == value){     \
                return i;                                \
            }                                            \
        }                                                \
        co->constants.push_back(allocator(value));       \
    } while (false)                                      \

// compiler class
class VioCompiler{
    public:
        VioCompiler(){}
        
        // compiler API
        CodeObject* compile(const Exp& exp) {
            // Initiate new code object
            co = AS_CODE(ALLOC_CODE("main"));

            // generate compiled object
            gen(exp);

            // mark stop 
            emit(OP_HALT);

            return co;
            }
        void gen(const Exp& exp) {
            switch (exp.type)
            {
            case ExpTypr::NUMBER:
                emit(OP_CONST);
                emit(numericConstIdx(exp.number));
                break;

            case ExpTypr::STRING:
                emit(OP_CONST);
                emit(stringConstIdx(exp.string));
                break;
            
            default:
                break;
            }
        }

        // compile code object
        CodeObject* co;
        
};

private:
    // allocate numeric constant
    size_t numericConstIdx(double value) {
        ALLOC_CONST(IS_NUMBER, AS_NUMBER, NUMBER, value);
        return co->constants.size() - 1;
    }

    //  allocates string constant
    size_t stringConstIdx(const std::string& value) {
        ALLOC_STRING(IS_STRING, AS_CPPSTRING, ALLOC_STRING, value);
        return co->constants.size() - 1;
    }

    // emit allocate data to bytecode
    void emit(unit8_t code) { co->code.push_back(code);}


#endif