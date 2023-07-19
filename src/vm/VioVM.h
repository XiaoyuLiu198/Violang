/**
 * Violang Virtual Machine.
 */

#ifndef VioVM_h
#define VioVM_h

#include <array>
#include <stack>
#include <string>
#include <vector>

#include "../Logger.h"
#include "../bytecode/OpCode.h"
#include "../compiler/VioCompiler.h"
#include "../gc/VioCollector.h"
#include "../parser/VioParser.h"
#include "VioValue.h"
#include "Global.h"

using syntax::VioParser;

/**
 * Reads the current byte in the bytecode
 * and advances the ip pointer.
 */
#define READ_BYTE()  *ip++
/**
 * Reads a short word (2 bytes).
 */
#define READ_SHORT()  (ip+=2, (uint16_t)((ip[-2] << 8) | ip[-1]))

/**
 * Converts bytecode index to a pointer.
 */
#define TO_ADDRESS(index) (&fn->co->code[index])

/**
 * Gets a constant from the pool.
 */
#define GET_CONST() (fn->co->constants[READ_BYTE()])
/**
 * Stack top (stack overflow after exceeding).
 */
#define STACK_LIMIT 512

/**
 * Memory threshold after which GC is triggered.
 */
#define GC_TRESHOLD 1024

/**
 * Runtime allocation, can call GC.
 */
#define MEM(allocator, ...)  (maybeGC(), allocator(__VA_ARGS__))

/**
 * Binary operation.
 */
#define BINARY_OP(op) \
  do {\
    auto op2 = AS_NUMBER(pop()); \
    auto op1 = AS_NUMBER(pop()); \
    push(NUMBER(op1 op op2)); \
  } while (false)

/**
 * Generic values comparison.
 */
#define COMPARE_VALUES(op, v1, v2)  \
  do {                              \ 
    bool res;                       \
    switch (op) {                   \
      case 0:                       \
        res = v1 < v2;              \
        break;                      \
      case 1:                       \
        res = v1 > v2;              \
        break;                      \
      case 2:                       \
        res = v1 == v2;             \
        break;                      \
      case 3:                       \
        res = v1 >= v2;             \
        break;                      \
      case 4:                       \
        res = v1 <= v2;             \
        break;                      \
      case 5:                       \
        res = v1 != v2;             \
        break;                      \
    }                               \
    push(AS_BOOLEAN(res));             \
  } while (false)

// --------------------------------------------------
struct Frame {
  // return address of the caller
  uint8_t* ra;

  // base pointer of the caller
  VioValue* bp;

  // refenrence to the running function
  FunctionObject* fn;
};


/**
 * Vio Virtual Machine.
 */
class VioVM {
 public:
  VioVM()
      : global(std::make_shared<Global>()),
        parser(std::make_unique<VioParser>()),
        compiler(std::make_unique<VioCompiler>(global)),
        collector(std::make_unique<VioCollector>()) {
    setGlobalVariables();
  }

  /**
   * VM shutdown.
   */
  ~VioVM() { Traceable::cleanup(); }

  //----------------------------------------------------
  // Stack operations:

  /**
   * Pushes a value onto the stack.
   */
  void push(const VioValue& value) {
    if ((size_t)(sp - stack.begin()) == STACK_LIMIT) {
      DIE << "push(): stack overflow error \n";
    }
    *sp = value;
    sp++;
  }

  /**
   * Pops a value from the stack.
   */
  VioValue pop() {
    if (sp == stack.begin()){
      DIE << "pop(): empty stack. \n";
    }
    --sp;
    return *sp;
  }

  /**
   * Peeks an element from the stack.
   */
  VioValue peek(size_t offset = 0) {
    if (sp == stack.begin()){
      DIE << "peek(): empty stack. \n";
    }
    return *(sp - 1 - offset);
  }

  /**
   * Pops multiple values from the stack.
   */
  void popN(size_t count) {
    if (stack.size() == 0) {
      DIE << "popN(): empty stack.\n";
    }
    sp -= count;
  }

  //----------------------------------------------------
  // GC operations:

  /**
   * Obtains GC roots: variables on the stack, globals, constants.
   */
  std::set<Traceable*> getGCRoots() {
    auto roots = getStackGCRoots();

    auto constantRoots = getConstantGCRoots();
    roots.insert(constantRoots.begin(), constantRoots.end());

    auto globalRoots = getGlobalGCRoots();
    roots.insert(globalRoots.begin(), globalRoots.end());

    return roots;
  }

  /**
   * Returns stack GC roots.
   */
  std::set<Traceable*> getStackGCRoots() {
    std::set<Traceable*> roots;
    auto stackEntry = sp;
    while (stackEntry-- != stack.begin()) {
      if (IS_OBJECT(*stackEntry)) {
        roots.insert((Traceable*)stackEntry->object);
      }
    }
    return roots;
  }

  /**
   * Returns GC roots for constants.
   */
  std::set<Traceable*> getConstantGCRoots() {
    return compiler->getConstantObjects();
  }

  /**
   * Returns global GC roots.
   */
  std::set<Traceable*> getGlobalGCRoots() {
    std::set<Traceable*> roots;
    for (const auto& global : global->globals) {
      if (IS_OBJECT(global.value)) {
        roots.insert((Traceable*)global.value.object);
      }
    }
    return roots;
  }

  /**
   * Spawns a potential GC cycle.
   */
  void maybeGC() {
    if (Traceable::bytesAllocated < GC_TRESHOLD) {
      return;
    }

    auto roots = getGCRoots();
    if (roots.size() == 0) {
      return;
    }
    collector->gc(roots);
  }

  //----------------------------------------------------
  // Program execution

  /**
   * Executes a program.
   */
  VioValue exec(const std::string& program) {
    // 1. Parse the program
    auto ast = parser->parse("(begin " + program + ")");

    // 2. Compile program to bytecode
    compiler->compile(ast);

    // Start from the main entry point:
    fn = compiler->getMainFunction();

    // Set instruction pointer to the beginning:
    ip = &fn ->co->code[0];

    // Init the stack:
    sp = &stack[0];

    // Init the base (frame) pointer:
    bp = sp;

    compiler->disassembleBytecode();

    return eval();
  }

  /**
   * Main eval loop.
   */
  VioValue eval() {
    for (;;) {
      auto opcode = READ_BYTE();
      switch (READ_BYTE()) {

        case OP_HALT: {
          return pop();
        }

        case OP_CONST: {
          push(GET_CONST());
          break;
        }

        // math operations
        case OP_ADD: {
          auto op1 = pop();
          auto op2 = pop();

          if (IS_NUMBER(op1) && IS_NUMBER(op2)) {
            auto v2 = AS_NUMBER(op2); 
            auto v1 = AS_NUMBER(op1); 
            push(NUMBER(v1 + v2)); 
          }
          else if (IS_STRING(op1) && IS_STRING(op2)) {
            auto s1 = AS_CPPSTRING(op1); 
            auto s2 = AS_CPPSTRING(op2);
            push(MEM(ALLOC_STRING(s1 + s2)));
          }
          break;
        }

        case OP_SUB: {
          BINARY_OP(-);
          break;
        }

        case OP_MUL: {
          BINARY_OP(*);
          break;
        }

        case OP_DIV: {
          BINARY_OP(/);
          break;
        }
        
        case OP_COMPARE: {
          auto op = READ_BYTE();

          auto op2 = pop();
          auto op1 = pop();
          if (IS_NUMBER(op1) && IS_NUMBER(op2)) {
            auto v1 = AS_NUMBER(op1);
            auto v2 = AS_NUMBER(op2);
            COMPARE_VALUES(op, v1, v2);
          } else if (IS_STRING(op1) && IS_STRING(op2)) {
            auto s1 = AS_STRING(op1);
            auto s2 = AS_STRING(op2);
            COMPARE_VALUES(op, s1, s2);
          }
          break;
        }

        case OP_JMP_IF_FALSE: {
          auto cond = AS_BOOLEAN(pop());
          auto address = READ_SHORT();

          if (!cond) {
            ip = TO_ADDRESS(address);
          }
          break;
        }

        case OP_JMP: {
          ip = TO_ADDRESS(READ_SHORT());
          break;
        }

        case OP_GET_GLOBAL: {
          auto globalIndex = READ_BYTE();
          push(global->get(globalIndex).value);
          break;
        }

        case OP_SET_GLOBAL: {
          auto globalIndex = READ_BYTE();
          auto value = peek(0);
          global->set(globalIndex, value);
          break;
        }

        case OP_POP:
          pop();
          break;

        case OP_GET_LOCAL: {
          auto localIndex = READ_BYTE();
          if (localIndex < 0 || localIndex >= stack.size()) {
            DIE << "OP_GET_LOCAL: invalid variable index: " << (int)localIndex;
          }
          push(bp[localIndex]);
          break;
        }

        case OP_SET_LOCAL: {
          auto localIndex = READ_BYTE();
          auto value = peek(0);
          if (localIndex < 0 || localIndex >= stack.size()) {
            DIE << "OP_SET_LOCAL: invalid variable index: " << (int)localIndex;
          }
          bp[localIndex] = value;
          break;
        }

        case OP_SCOPE_EXIT: {
          auto count = READ_BYTE();

          *(sp - 1 - count) = peek(0);
          popN(count);
          break;
        }

        case OP_CALL: {
          auto argsCount = READ_BYTE();
          auto fnValue = peek(argsCount);

          // native function
          if (IS_NATIVE(fnValue)) {
            AS_NATIVE(fnValue)->function();
            auto result = pop();

            popN(argsCount+1); // pop args and function object itself
            push(result);
            break;
          }

          // user defined funciton
          auto callee = AS_FUNCTION(fnValue);
          // save the execution context, restored on OP_RETURN
          callStack.push(Frame{ip, bp, fn});

          fn = callee;
          // set base pointer to the callee
          bp = sp - argsCount - 1;
          // jump to the function code
          ip = &callee->co->code[0];
        }

        case OP_RETURN:
          {
          // restore the caller address
          auto callerFrame = callStack.top();

          // restore ip, bp and fn for caller;
          ip = callerFrame.ra;
          bp = callerFrame.bp;
          fn = callerFrame.fn;

          callStack.pop();
          break;
          }

        default:
          DIE << "Unknown opcode: " << std::hex << opcode;
      }
    }
  }

  /**
   * Sets up global variables and function.
   */
  void setGlobalVariables() {
    global->addNativeFunction(
      "square",
      [&]() {
        auto x = AS_NUMBER(peek(0));
        push(NUMBER(x * x));
      },
      1);

    global->addConst("VERSION", 1);
  }

  /**
   * Global object.
   */
  std::shared_ptr<Global> global;


  /**
   * Parser.
   */
  std::unique_ptr<VioParser> parser;

  /**
   * Compiler.
   */
  std::unique_ptr<VioCompiler> compiler;

  /**
   * Garbage collector.
   */
  std::unique_ptr<VioCollector> collector;

  /**
   * Instruction pointer (aka Program counter).
   */
  uint8_t* ip;

  /**
   * Stack pointer.
   */
  VioValue* sp;

  /**
   * Base pointer (aka Frame pointer).
   */
  VioValue* bp;

  /**
   * Operands stack.
   */
  std::array<VioValue, STACK_LIMIT> stack;

  /**
   * Separate stack for calls. Keeps return addresses.
   */
  std::stack<Frame> callStack;

  /**
   * Currently executing function.
   */
  FunctionObject* fn;

};

#endif