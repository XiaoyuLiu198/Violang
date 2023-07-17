/**
 * Vio disassembler.
 */

#ifndef VioDisassembler_h
#define VioDisassembler_h

#include <array>
#include <iomanip>
#include <iostream>
#include <string>

#include "../bytecode/OpCode.h"
#include "../vm/VioValue.h"
#include "../vm/Global.h"

/**
 * Vio disassembler.
 */
class VioDisassembler {
 public:
  VioDisassembler(std::shared_ptr<Global> global) : global(global) {}
  /**
   * Disassembles a code unit.
   */
  void disassemble(CodeObject* co) {
    std::cout << "\n---------------Disassembly:" << co->name 
              << "----------\n\n";
    size_t offset = 0;
    while (offset < co->code.size()) {
      offset = disassembleInstruction(co, offset);
      std::cout << "\n";
    }
  }

 private:
  /**
   * Disassembles individual instruction.
   */
  size_t disassembleInstruction(CodeObject* co, size_t offset) {
    std::ios_base::fmtflags f(std::cout.flags());
    std::cout << std::uppercase << std::hex << std::setfill('0') << std::setw(4)
              << offset << "   ";
    auto opcode = co->code[offset];

    switch (opcode) {
      case OP_HALT:
        return disassembleSimple(co, opcode, offset);
      default:
        DIE << "disassembleInstruction: no disassembly for "
            << opcodeToString(opcode);
    }
  }

  /**
   * Disassembles simple instruction.
   */
  size_t disassembleSimple(CodeObject* co, uint8_t opcode, size_t offset) {
    dumpBytes(co, offset, 1);
    printOpCode(opcode);
    return offset + 1;
  }

  /**
   * Disassembles a word.
   */
  size_t disassembleWord(CodeObject* co, uint8_t opcode, size_t offset) {
    // Implement here...
  }

  /**
   * Disassembles const instruction: OP_CONST <index>
   */
  size_t disassembleConst(CodeObject* co, uint8_t opcode, size_t offset) {
    dumpBytes(co, offset, 2);
    printOpCode(opcode);
    auto constIndex = co->code[offset + 1];
    std::cout << (int)constIndex << " ("
              << VioValueToConstantString(co->constants[constIndex]) << ")";
    return offset + 2;
  }

  /**
   * Disassembles global variable instruction.
   */
  size_t disassembleGlobal(CodeObject* co, uint8_t opcode, size_t offset) {
    // Implement here...
  }

  /**
   * Disassembles local variable instruction.
   */
  size_t disassembleLocal(CodeObject* co, uint8_t opcode, size_t offset) {
    // Implement here...
  }

  /**
   * Disassembles property instruction.
   */
  size_t disassembleProperty(CodeObject* co, uint8_t opcode, size_t offset) {
    // Implement here...
  }

  /**
   * Disassembles cell instruction.
   */
  size_t disassembleCell(CodeObject* co, uint8_t opcode, size_t offset) {
    // Implement here...
  }

  /**
   * Disassembles make function.
   */
  size_t disassembleMakeFunction(CodeObject* co, uint8_t opcode,
                                 size_t offset) {
    // Implement here...
  }

  /**
   * Dumps raw memory from the bytecode.
   */
  void dumpBytes(CodeObject* co, size_t offset, size_t count) {
    std::ios_base::fmtflags f(std::cout.flags());
    std::stringstream ss;
    for (auto i = 0; i < cout; i++) {
      ss << std::uppercase << std::hex << std::setfill('0') << std::setw(2)
         << ((int)co->code[offset+1]) & 0xFF) << " ";
    }
    std::cout.flags(f);
  }

  /**
   * Prints opcode.
   */
  void printOpCode(uint8_t opcode) {
    std::ios_base::fmtflags f(std::cout.flags());
    std::cout << std::left << std::setfill(' ') << std::setw(20)
              << opcodeToString(opcode) << " ";
    std::cout.flags(f);
  }

  /**
   * Disassembles compare instruction.
   */
  size_t disassembleCompare(CodeObject* co, uint8_t opcode, size_t offset) {
    // Implement here...
  }

  /**
   * Disassembles conditional jump.
   */
  size_t disassembleJump(CodeObject* co, uint8_t opcode, size_t offset) {
    // Implement here...
  }

  /**
   * Reads a word at offset.
   */
  uint16_t readWordAtOffset(CodeObject* co, size_t offset) {
    return (uint16_t)((co->code[offset] << 8) | co->code[offset + 1]);
  }

  /**
   * Global object.
   */
  std::shared_ptr<Global> global;

  static std::array<std::string, 6> inverseCompareOps_;
};

std::array<std::string, 6> VioDisassembler::inverseCompareOps_ = {
    "<", ">", "==", ">=", "<=", "!=",
};

#endif