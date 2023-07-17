/**
 * Vio compiler.
 */

#ifndef VioCompiler_h
#define VioCompiler_h

#include <map>
#include <set>
#include <string>

#include "../disassembler/VioDisassembler.h"
#include "../parser/VioParser.h"
#include "../vm/VioValue.h"
#include "../vm/Global.h"

// -----------------------------------------------------------------
// Allocates new constants in the pool
#define ALLOC_CONST(tester, converter, allocator, value)  \
    do {                                                  \
      for (auto i = 0; i < co->constants.size(); i++) {   \
        if (!tester(co->constants[i])){                   \
          continue;                                       \
        }                                                 \
        if (converter(co->constants[i]) == value) {       \
          return i;                                       \
        }                                                 \
      }                                                   \
      co->constants.push_back(allocator(value));          \
    } while (false)

// Generic binary operator
#define GEN_BINARY_OP(op) do {gen(exp.list[1]); gen(exp.list[2]); emit(op);} while (false)

#define FUNCTION_CALL(exp)                        \
  do {                                            \
    gen(exp.list[0]);                             \
    for (auto i = 1; i < exp.list.size(); i++) {  \
      gen(exp.list[i]);                           \
    }                                             \
    emit(OP_CALL);                                \ 
    emit(exp.list.size() - 1);                    \
  } while (false)             
/**
 * Compiler class, emits bytecode, records constant pool, vars, etc.
 */
class VioCompiler {
 public:
  VioCompiler(std::shared_ptr<Global> global)
      : global(global),
        disassembler(std::make_unique<VioDisassembler>(global)) {}

  /**
   * Main compile API.
   */
  void compile(const Exp& exp) {
    // allocate the new code object
    co = AS_CODE(ALLOC_CODE("main", *exp.list.size()));
    // main = AS_FUNCTION(ALLOC_FUNCTION(co));
    // generate recursively from top-level
    gen(exp);

    emit(OP_HALT);
  }

  /**
   * Main compile loop.
   */
  void gen(const Exp& exp) {
    switch (exp.type) {
      /**
       * ----------------------------------------------
       * Numbers.
       */
      case ExpType::NUMBER:
        emit(OP_CONST);
        emit(numericConstIdx(exp.number));
        break;

      /**
       * ----------------------------------------------
       * Strings.
       */
      case ExpType::STRING:
        emit(OP_CONST);
        emit(stringConstIdx(exp.string));
        break;

      /**
       * ----------------------------------------------
       * Symbols (variables, operators).
       */
      case ExpType::SYMBOL:
        /**
         * Boolean.
         */
        if (exp.string == "true" || exp.string == "false") {
          emit(OP_CONST);
          emit(booleanConstIdx(exp.string == "true" ? true : false));
        } else {
          // Variables:
          auto varName = exp.string; 

          // 1. Local vars:
          auto localIndex = co->getLocalIndex(varName);
          if (localIndex != -1) {
            emit(OP_GET_LOCAL);
            emit(localIndex);
          }
          // 2. Global vars
          else {
            if (!globals->exists(exp.string)) {
              DIE << "[VioCompiler]: Reference error: " << exp.string;
              }
            emit(OP_GET_GLOBAL);
            emit(global->getGlobalIndex(exp.string));
          }
        }
        break;

      /**
       * ----------------------------------------------
       * Lists.
       */
      case ExpType::LIST:
        auto tag = exp.list[0];

        /**
         * ----------------------------------------------
         * Special cases.
         */
        if (tag.type == ExpType::SYMBOL) {
          auto op = tag.string;

          if (op == "+") {
            GEN_BINARY_OP(OP_ADD);
          }

          else if (op == "-") {
            GEN_BINARY_OP(OP_SUB);
          }

          else if (op == "*") {
            GEN_BINARY_OP(OP_MUL);
          }

          else if (op == "/") {
            GEN_BINARY_OP(OP_DIV);
          }

          else if (compareOps_.count(op) != 0) {
            gen(exp.list[1]);
            gen(exp.list[2]);
            emit(OP_COMPARE);
            emit(compareOps_[op]);
          }

          else if (op == "if") {
            // emit test
            gen(exp.list[1]);

            emit(OP_JMP_IF_FALSE);

            // Patch the else branch
            emit(0);
            emit(0);

            auto elseJmpAddr = getOffset() - 2;

            gen(exp.list[2]);

            emit(OP_JMP);
            emit(0);
            emit(0);

            auto endAddr = getOffset() - 2;

            // Patch else branch address
            auto elseBranchAddr = getOffset();
            patchJumpAddress(elseJmpAddr, elseBranchAddr);

            // emit <alternate> if exist
            if (exp.list.size() == 4) {
              gen(exp.list[3]);
            }

            // patch the end
            auto endBranchAddr = getOffset()
            patchJumpAddress(endAddr, endBranchAddr);
          }

          else if (op == "while") {
            auto loopStartAddr = getOffset();

            // emit condition
            gen(exp.list[1]);
            
            emit(OP_JMP_IF_FALSE);
            emit(0);
            emit(0);

            auto lookEndJmpAddr = getOffset() - 2;

            gen(exp.list[2]);
            emit(OP_JMP);
            emit(0);
            emit(0);

            patchJumpAddress(getOffset() - 2, loopStartAddr);
            auto loopEndAddr = getOffset() + 1;
            patchJumpAddress(lookEndJmpAddr, loopEndAddr);

          }

          // variable declaration
          else if (op == "var") {
            auto varName = exp.list[1].string;

            // initialize
            if (isLambda(exp.list[2])) {
              compileFunction(
                exp.list[2],
                exp.list[1].string,
                exp.list[2].list[1],
                exp.list[2].list[2]
                );
            } else {
              gen(exp.list[2]);
            }

            if (isGlobalScope()) {
              // global variables
              global->define(varName);
              emit(OP_SET_GLOBAL);
              emit(global->getGlobalIndex(exp.list[1].string));
            }
            else {
              co->addLocal(varName);
              emit(OP_SET_LOCAL);
              emit(global->getLocalIndex(varName));
            }
          }

          else if (op == "set") {
            auto varName = exp.list[1].string;

            gen(exp.list[2]);
            
            // For local variables
            auto localIndex = co->getLocalIndex(varName);

            if (localIndex != -1) {
              emit(OP_SET_LOCAL);
              emit(localIndex);
            }
            else {
              auto globalIndex = global->getGlobalIndex(varName);
              if (globalIndex == -1) {
                DIE << "Reference error: " << varName << " is not defined.";
              }
              emit(OP_SET_GLOBAL);
              emit(globalIndex);
            }

            
          }

          else if (op == "begin") {
            scopeEnter();
            // compile each expression within the block:
            for (auto i=1; i < exp.list.size(); i++) {
              // the value of the last expression is kept on the stack as final result
              bool isLast = i == exp.list.size() - 1;

              auto isLocalDeclaration = isDeclaration(exp.list[i]) && !isGlobalScope();

              // generate expression code
              gen(exp.list[i]);

              if (!isLast) {
                emit(OP_POP);  
              }
            }
            scopeExit();
          }

          else if (op=="def") {
            auto fnName = exp.list[1].string;

            compileFunction(
              exp,
              exp.list[1].string,
              exp.list[2],
              exp.list[3],
            );

            if (isGlobalScope()) {
              global->define(fnName);
              emit(OP_SET_GLOBAL);
              emit(global->getGlobalIndex(fnName));
            } else {
              co->addLocal(fnName);
              emit(OP_SET_LOCAL);
              emit(co->getLocalIndex(fnName));
            }
          } 

          else if (op == "lambda") {
            compileFunction(
              exp,
              "lambda",
              exp.list[2],
              exp.list[3],
            );
          }

          // Functional call
          else {
            FUNCTION_CALL(exp);
          }
        }


        // --------------------------------------------
        // Lambda function calls:
        // ((lambda (x) (* x x)) 2)

        else {
          // Implement here...
        }

        break;
    }
  }

  /**
   * Disassemble code objects.
   */
  void disassembleBytecode() {
    for (auto& co_ : codeObjects_) {
      disassembler->disassemble(co_);
    }
  }

  /**
   * Returns main function (entry point).
   */
  FunctionObject* getMainFunction() { return main; }

  /**
   * Returns all constant traceable objects.
   */
  std::set<Traceable*>& getConstantObjects() { return constantObjects_; }

 private:
  /**
   * Global object.
   */
  std::shared_ptr<Global> global;

  /**
   * Disassembler.
   */
  std::unique_ptr<VioDisassembler> disassembler;

  /**
   * Enter a new scope
   */
  void scopeEnter() { co->scopeLevel++; }

  /**
   * Exits a new scope
   */
  void scopeExit() { 
    auto varsCount = getVarsCountOnScopeExit();

    if (varsCount > 0 || co->arity > 0) {
      emit(OP_SCOPE_EXIT);

      if (isFunctionBody()) {
        varsCount+= co->arity+1;
      }
      emit(varsCount);
    }
    co->scopeLevel--; 
  }

  /**
   * Compiles a function.
   */
  void compileFunction(const Exp& exp, const std::string fnName,
                       const Exp& params, const Exp& body) {
    auto arity = params.list.size();
    
    auto prevCo = co;
    auto coValue = createCodeObjectValue(fnName, arity);
    co = AS_CODE(coValue);

    prevCo->constants.push_back(coValue); // store as a new constant
    co->addLocal(fnName); //register function name as local variable to enable calling recursively


    for (auto i = 0; i < arity; i++) {
      auto argName = exp.list[2].list[1].string;
      co->addLocal(argName);
    }

    // compile the function body
    gen(body); 
    if (!isBlock(body)) {
      emit(OP_SCOPE_EXIT);
      emit(arity + 1); // +1 for function itself to be set as a local
    }
    emit(OP_RETURN); // explicit return to restore caller address

    auto fn = ALLOC_FUNCTION(co);
    co->constants.push_back(fn);

    // emit code for new constant
    emit(OP_CONST);
    emit(co->constants.size()-1); 
  }

  /**
   * Creates a new code object.
   */
  VioValue createCodeObjectValue(const std::string& name, size_t arity = 0) {
    auto coValue = ALLOC_CODE(name, arity);
    auto co = AS_CODE(coValue);
    codeObjects_.push_back(co);
    constantObjects_.insert((Traceable*)co);
    return coValue;
  }

  /**
   * Enters a new block.
   */
  void blockEnter() { co->scopeLevel++; }

  /**
   * Exits a block.
   */
  void blockExit() {
    // Implement here...
  }

  /**
   * Whether it's the global scope.
   */
  bool isGlobalScope() { return co->name == "main" && co->scopeLevel == 1; }

  /**
   * Whether it's the global scope.
   */
  bool isFunctionBody() { return co->name != "main" && co->scopeLevel == 1; }

  /**
   * Whether the expression is a declaration.
   */
  bool isDeclaration(const Exp& exp) {
    return isVarDeclaration(exp) || isFunctionDeclaration(exp) ||
           isClassDeclaration(exp);
  }

  /**
   * (class ...)
   */
  bool isClassDeclaration(const Exp& exp) { return isTaggedList(exp, "class"); }

  /**
   * (prop ...)
   */
  bool isProp(const Exp& exp) { return isTaggedList(exp, "prop"); }

  /**
   * (var <name> <value>)
   */
  bool isVarDeclaration(const Exp& exp) { return isTaggedList(exp, "var"); }

  /**
   * (lambda ...)
   */
  bool isLambda(const Exp& exp) { return isTaggedList(exp, "lambda"); }

  /**
   * (def <name> ...)
   */
  bool isFunctionDeclaration(const Exp& exp) {
    return isTaggedList(exp, "def");
  }

  /**
   * (begin ...)
   */
  bool isBlock(const Exp& exp) { return isTaggedList(exp, "begin"); }

  /**
   * Tagged lists.
   */
  bool isTaggedList(const Exp& exp, const std::string& tag) {
    return exp.type == ExpType::LIST && exp.list[0].type == ExpType::SYMBOL &&
           exp.list[0].string == tag;
  }

  /**
   * Pop the variables of the current scope,
   * returns number of vars used.
   */
  size_t getVarsCountOnScopeExit() {
    auto varsCount = 0;

    if(co->locals.size() > 0) {
      while (co->locals.back().scopeLevel == co->scopeLevel) {
        co->locals.pop_back();
        varsCount++;
      }
    }
    return varsCount;
  }

  /**
   * Returns current bytecode offset.
   */
  uint16_t getOffset() { return (uint16_t)co->code.size(); }

  /**
   * Allocates a numeric constant.
   */
  size_t numericConstIdx(double value) {
    ALLOC_CONST(IS_NUMBER, AS_NUMBER, NUMBER, value);
    return co->constants.size() - 1;
  }

  /**
   * Allocates a string constant.
   */
  size_t stringConstIdx(const std::string& value) {
    ALLOC_STRING(IS_STRING, AS_CPPSTRING, ALLOC_STRING, value);
    constantObjects_.insert((Traceable*)co->constants.back().object);
    return co->constants.size() - 1;
  }

  /**
   * Allocates a boolean constant.
   */
  size_t booleanConstIdx(bool value) {
    ALLOC_CONST(IS_BOOLEAN, AS_BOOLEAN, BOOLEAN, value);
    return co->constants.size() - 1;
  }

  /**
   * Emits data to the bytecode.
   */
  void emit(uint8_t code) { co->code.push_back(code); }

  /**
   * Writes byte at offset.
   */
  void writeByteAtOffset(size_t offset, uint8_t value) {
    co->code[offset] = value;
  }

  /**
   * Patches jump address.
   */
  void patchJumpAddress(size_t offset, uint16_t value) {
    writeByteAtOffset(offset, (value >> 8) & 0xff);
    writeByteAtOffset(offset + 1, value & 0xff);
  }

  /**
   * Returns a class object by name.
   */
  ClassObject* getClassByName(const std::string& name) {
    // Implement here...
  }

  /**
   * Scope info.
   */
  std::map<const Exp*, std::shared_ptr<Scope>> scopeInfo_;

  /**
   * Scopes stack.
   */
  std::stack<std::shared_ptr<Scope>> scopeStack_;

  /**
   * Compiling code object.
   */
  CodeObject* co;

  /**
   * Main entry point (function).
   */
  FunctionObject* main;

  /**
   * All code objects.
   */
  std::vector<CodeObject*> codeObjects_;

  /**
   * All objects from the constant pools of all code objects.
   */
  std::set<Traceable*> constantObjects_;

  /**
   * Currently compiling class object.
   */
  ClassObject* classObject_;

  /**
   * All class objects.
   */
  std::vector<ClassObject*> classObjects_;

  /**
   * Compare ops map.
   */
  static std::map<std::string, uint8_t> compareOps_;
};

/**
 * Compare ops map.
 */
std::map<std::string, uint8_t> VioCompiler::compareOps_ = {
    {"<", 0}, {">", 1}, {"==", 2}, {">=", 3}, {"<=", 4}, {"!=", 5},
};

#endif