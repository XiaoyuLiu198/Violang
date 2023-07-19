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
    ::operator delete(object, sz);
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
size_t Traceable::bytesAllocated{0};

/**
 * List of all allocated objects.
 */
std::list<Traceable*> Traceable::objects{};

// ----------------------------------------------------------------

/**
 * Base object.
 */
struct Object : public Traceable {
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
struct ClassObject : public Object {
  // Implement here...
};

// ----------------------------------------------------------------

/**
 * Instance object.
 */
struct InstanceObject : public Object {
  // Implement here...
};

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
  CodeObject(const std::string& name, const int arity) : Object(ObjectType::CODE), name(name), arity(arity) {}

  std::string name;
  std::vector<VioValue> constants;
  std::vector<uin8_t> code;
  int arity;

  size_t scopeLevel = 0;

  std::vector<LocalVar> locals;

  void addLocal(const std::string& name) {
    locals.push_back({name, scopeLevel});
  }

  int getLocalIndex(const std::string& name) {
    if (name.size() > 0) {
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
struct CellObject : public Object {
  // Implement here...
};

// ----------------------------------------------------------------

/**
 * Function object.
 */
struct FunctionObject : public Object {
  FunctionObject(CodeObject* co) : Object(ObjectType::FUNCTION), co(co) {}
  CodeObject* co;
};

// ----------------------------------------------------------------
// Constructors:

#define NUMBER(value) ((VioValue){VioValueType::NUMBER, .number = value})

#define ALLOC_STRING(value) ((VioValue){VioValueType::OBJECT, .object = (Object*) new StringObject(value)})

#define ALLOC_CODE(name, arity) ((VioValue){VioValueType::OBJECT, .object = (Object*) new CodeObject(name, arity)})
#define ALLOC_NATIVE(fn, name, arity) ((VioValue){VioValueType::OBJECT, .object = (Object*)new NativeObject(fn, name, arity)})
#define ALLOC_FUNCTION(co) ((VioValue){VioValueType::OBJECT, .object = (Object*)new FunctionObject(co)})

// ----------------------------------------------------------------
// Accessors:

#define AS_NUMBER(evaValue) ((double)(evaValue).number)
#define AS_BOOLEAN(evaValue) ((bool)(evaValue).boolean)
#define AS_STRING(evaValue) ((StringObject*)(evaValue).object)
#define AS_CPPSTRING(evaValue) (AS_STRING(evaValue) -> string)
#define AS_CODE(evaValue) ((CodeObject*)(evaValue).object)
#define AS_NATIVE(evaValue) ((NativeObject*)(evaValue).object)
#define AS_OBJECT(evaValue) ((Object*) (evaValue).object)
#define AS_FUNCTION(evaValue) ((FunctionObject*)(evaValue).object)
// ----------------------------------------------------------------
// Testers:

#define IS_OBJECT_TYPE(evaValue, objectType) IS_OBJECT(evaValue)
#define IS_OBJECT(evaValue) ((evaValue).type == VioValueType::OBJECT)

#define IS_NUMBER(evaValue) ((evaValue).type == VioValueType::NUMBER)
#define IS_BOOLEAN(evaValue) ((evaValue).type == VioValueType::BOOLEAN)
#define IS_STRING(evaValue) IS_OBJECT_TYPE(evaValue, ObjectType::STRING)
#define IS_CODE(evaValue) IS_OBJECT_TYPE(evaValue, ObjectType::CODE)
#define IS_NATIVE(evaValue) IS_OBJECT_TYPE(evaValue, ObjectType::NATIVE)
// Implement here...

// ----------------------------------------------------------------

/**
 * String representation used in constants for debug.
 */
std::string evaValueToTypeString(const VioValue& evaValue) {
  if (IS_NUMBER(evaValue)) {
    return "NUMBER";
  } else if (IS_BOOLEAN(evaValue)) {
    return "BOOLEAN";
  } else if (IS_STRING(evaValue)) {
    return "STRING";
  } else if (IS_CODE(evaValue)) {
    return "CODE";
  } else {
    DIE << "evaValueToTypeString unkown type " << (int)evaValue.type;
  }
  return "";
}

/**
 * String representation used in constants for debug.
 */
std::string evaValueToConstantString(const VioValue& evaValue) {
  std::stringstream ss;
  if (IS_NUMBER(evaValue)) {
    ss << evaValue.number;
  } else if (IS_BOOLEAN(evaValue)) {
    ss << (evaValue.boolean == true ? "true" : "false");
  } else if (IS_STRING(evaValue)) {
    ss << '"' << AS_CPPSTRING(evaValue) << '"';
  } else if (IS_CODE(evaValue)) {
    auto code = AS_CODE(evaValue);
    ss << "code" << code << ": " << code->name;
  } else {
    DIE << "evaValueToConstantString unkown type " << (int)evaValue.type;
  }
  return ss.str();
}

/**
 * Output stream.
 */
std::ostream& operator<<(std::ostream& os, const VioValue& evaValue) {
  return os << "VioValue (" << evaValueToTypeString(evaValue)
            << "): " << evaValueToConstantString(evaValue);
}

#endif