/**
 * Vio value.
 */

#ifndef VioValue_h
#define VioValue_h

#include <list>
#include <string>
#include <vector>
#include <stdint.h>

/**
 * Vio value type.
 */
enum class VioValueType {
  NUMBER,
  BOOLEAN,
  OBJECT,
};

/**
 * Object type.
 */
enum class ObjectType {
  STRING,
  CODE,
  NATIVE,
  FUNCTION,
  CELL,
  CLASS,
  INSTANCE,
};

// ----------------------------------------------------------------

/**
 * Base traceable object.
 */
struct Traceable {
  /**
   * Whether the object was marked during the trace.
   */
  bool marked;

  /**
   * Allocated size.
   */
  size_t size;

  /**
   * Allocator.
   */
  static void* operator new(size_t size) {
    void* object = ::operator new(size);
    ((Traceable*)object)->size = size;
    Traceable::objects.push_back((Traceable*)object);
    Traceable::bytesAllocated += size;

    return object;
  }

  /**
   * Deallocator.
   */
  static void operator delete(void* object, std::size_t sz) {
    Traceable::bytesAllocated -= ((Traceable*)object)->size;
    // ::operator delete(object, sz);
  }

  /**
   * Clean up for all objects.
   */
  static void cleanup() {
    for (auto&object: objects) {
      delete object;
    }
    objects.clear();
  }

  /**
   * Total number of allocated bytes.
   */
  static size_t bytesAllocated;

  /**
   * List of all allocated objects.
   */
  static std::list<Traceable*> objects;
};

/**
 * Total bytes allocated.
 */
// size_t Traceable::bytesAllocated {0};

/**
 * List of all allocated objects.
 */
// std::list<Traceable*> Traceable::objects {};

// ----------------------------------------------------------------

/**
 * Base object.
 */
// struct Object : public Traceable {
//   Object(ObjectType type) : type(type) {}
//   ObjectType type;
// };
struct Object {
  Object(ObjectType type) : type(type) {}
  ObjectType type;
};

// ----------------------------------------------------------------

/**
 * String object.
 */
struct StringObject : public Object {
  StringObject(const std::string& str)
    : Object(ObjectType::STRING), string(str) {}
  std::string string;
};

// ----------------------------------------------------------------

using NativeFn = std::function<void()>;

/**
 * Native function.
 */
struct NativeObject : public Object {
  NativeObject(NativeFn function, const std::string& name, size_t arity)
      : Object(ObjectType::NATIVE),
        function(function),
        name(name),
        arity(arity) {}

  NativeFn function;
  std::string name;
  size_t arity;
};

// ----------------------------------------------------------------

/**
 * Vio value (tagged union).
 */
struct VioValue {
  VioValueType type;
  union {
    double number;
    bool boolean;
    Object* object;
  };
};

// ----------------------------------------------------------------

/**
 * Class object.
 */
// struct ClassObject : public Object {
//   // Implement here...
// };

// ----------------------------------------------------------------

/**
 * Instance object.
 */
// struct InstanceObject : public Object {
//   // Implement here...
// };

// ----------------------------------------------------------------

struct LocalVar {
  std::string name;
  size_t scopeLevel;
};

/**
 * Code object.
 *
 * Contains compiling bytecode, locals and other
 * state needed for function execution.
 */
struct CodeObject : public Object {
  // CodeObject(const std::string& name, const int arity) : Object(ObjectType::CODE), name(name), arity(arity) {}
  CodeObject(const std::string& name, size_t arity) : Object(ObjectType::CODE), name(name), arity(arity) {}

  std::string name;
  std::vector<VioValue> constants;
  std::vector<uint8_t> code;
  size_t arity;

  size_t scopeLevel = 0;

  std::vector<LocalVar> locals;

  void addLocal(const std::string& name) {
    locals.push_back({name, scopeLevel});
    // (LocalVar)
  }

  void addConst(const VioValue& value) {
    constants.push_back(value);
  }

  int getLocalIndex(const std::string& name) {
    // if (globals.size() > 0) {
    if (locals.size() > 0) {
      for (auto i = (int)locals.size() - 1; i >= 0; i--) {
        if (locals[i].name == name) {
          return i;
        }
      }
    }
    return -1;
  }
};

// ----------------------------------------------------------------

/**
 * Heap-allocated cell.
 *
 * Used to capture closured variables.
 */
// struct CellObject : public Object {
//   // Implement here...
// };

// ----------------------------------------------------------------

/**
 * Function object.
 */
struct FunctionObject : public Object {
  FunctionObject(CodeObject* co) : Object(ObjectType::FUNCTION), co(co) {}
  // reference to code object
  CodeObject* co;
};

// ----------------------------------------------------------------
// Constructors:

#define NUMBER(value) ((VioValue){VioValueType::NUMBER, .number = value})
#define BOOLEAN(value) ((VioValue){VioValueType::BOOLEAN, .boolean = value})

#define ALLOC_STRING(value) ((VioValue){VioValueType::OBJECT, .object = (Object*) new StringObject(value)})

#define ALLOC_CODE(name, arity) ((VioValue){VioValueType::OBJECT, .object = (Object*) new CodeObject(name, arity)})
// #define ALLOC_CODE(name) ((VioValue){VioValueType::OBJECT, .object = (Object*) new CodeObject(name)})
#define ALLOC_NATIVE(fn, name, arity) ((VioValue){VioValueType::OBJECT, .object = (Object*)new NativeObject(fn, name, arity)})
#define ALLOC_FUNCTION(co) ((VioValue){VioValueType::OBJECT, .object = (Object*)new FunctionObject(co)})

// ----------------------------------------------------------------
// Accessors:

#define AS_NUMBER(value) ((double)(value).number)
#define AS_BOOLEAN(value) ((bool)(value).boolean)
#define AS_STRING(value) ((StringObject*)(value).object)
#define AS_CPPSTRING(value) (AS_STRING(value) -> string)
#define AS_CODE(value) ((CodeObject*)(value).object)
#define AS_NATIVE(value) ((NativeObject*)(value).object)
#define AS_OBJECT(value) ((Object*) (value).object)
#define AS_FUNCTION(value) ((FunctionObject*)(value).object)
// ----------------------------------------------------------------
// Testers:

#define IS_OBJECT_TYPE(value, objectType) \
  (IS_OBJECT(value) && AS_OBJECT(value)->type==objectType)

#define IS_OBJECT(value) ((value).type == VioValueType::OBJECT)

#define IS_NUMBER(value) ((value).type == VioValueType::NUMBER)
#define IS_BOOLEAN(value) ((value).type == VioValueType::BOOLEAN)
#define IS_STRING(value) IS_OBJECT_TYPE(value, ObjectType::STRING)
#define IS_CODE(value) IS_OBJECT_TYPE(value, ObjectType::CODE)
#define IS_NATIVE(value) IS_OBJECT_TYPE(value, ObjectType::NATIVE)
#define IS_FUNCTION(value) IS_OBJECT_TYPE(value, ObjectType::FUNCTION)

// ----------------------------------------------------------------

/**
 * String representation used in constants for debug.
 */
std::string vioValueToTypeString(const VioValue& vioValue) {
  if (IS_NUMBER(vioValue)) {
    return "NUMBER";
  } else if (IS_BOOLEAN(vioValue)) {
    return "BOOLEAN";
  } else if (IS_STRING(vioValue)) {
    return "STRING";
  } else if (IS_CODE(vioValue)) {
    return "CODE";
  } else if (IS_NATIVE(vioValue)) {
    return "NATIVE";
  } else if (IS_FUNCTION(vioValue)) {
    return "FUNCTION";
  } else {
    DIE << "vioValueToTypeString unkown type " << (int)vioValue.type;
  }
  return "";
}

/**
 * String representation used in constants for debug.
 */
std::string vioValueToConstantString(const VioValue& vioValue) {
  std::stringstream ss;
  if (IS_NUMBER(vioValue)) {
    ss << vioValue.number;
  } else if (IS_BOOLEAN(vioValue)) {
    ss << (vioValue.boolean == true ? "true" : "false");
  } else if (IS_STRING(vioValue)) {
    ss << '"' << AS_CPPSTRING(vioValue) << '"';
  } else if (IS_CODE(vioValue)) {
    auto code = AS_CODE(vioValue);
    ss << "code" << code << ": " << code->name << "/" << code->arity;
  } else if (IS_FUNCTION(vioValue)) {
    auto fn = AS_FUNCTION(vioValue);
    ss << fn->co->name << "/" << fn->co->arity;
  } else if (IS_NATIVE(vioValue)) {
    auto fn = AS_NATIVE(vioValue);
    ss << fn->name << "/" << fn->arity;
  } else {
    DIE << "vioValueToConstantString unkown type " << (int)vioValue.type;
  }
  return ss.str();
}

/**
 * Output stream.
 */
std::ostream& operator<<(std::ostream& os, const VioValue& vioValue) {
  return os << "VioValue (" << vioValueToConstantString(vioValue)
            << "): " << vioValueToConstantString(vioValue);
}

#endif