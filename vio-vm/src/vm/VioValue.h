//  define the VioValue type

// #ifdef VioValue_h
#define VioValue_h

// construct VioValue type enum class
enum class VioValueType{
    NUMBER,
    OBJECT,
};

enum class ObjectType {
    STRING,
    CODE,
};

// VioValue
struct VioValue
{
    VioValueType type;
    union {
        double number;
        Object* object;
    }
};

struct CodeObject : public Object {
    CodeObject(const std::string& name) : Object(ObjectType::CODE), name(name) {}
    
    std::string name;
    //  constant pool
    std::vector<VioValue> constants;
    //  bytecode
    std::vector<uint8_t> code;
};

//  constructors
#define NUMBER(value) (VioValue)({VioValueType::NUMBER, .number = value})

#define ALLOC_STRING(value) ((VioValue){VioValueType::OBJECT, .object=(Object*)new StringObject(value)})

#define ALLOC_CODE(value) ((VioValue){VioValueType::OBJECT, .object=(Object*)new CodeObject(name)})

// accessors

#define AS_NUMBER(VioValue) ((double)(VioValue).number)
#define AS_OBJECT(vioValue) ((Object*)(VioValue).object)
#define AS_STRING(VioValue) ((StringObject*)(VioValue).object)
#define AS_CPPSTRING(VioValue) (AS_STRING(VioValue).object)
#define AS_CODE(VioValue) ((CodeObject*)(VioValue).object)

// testers
#define IS_NUMBER(VioValue) ((VioValue).type == VioValue::NUMBER)
#define IS_OBJECT(VioValue) ((VioValue).type == VioValue::OBJECT)
#define IS_OBJECT_TYPE(VioValue, ObjectType) (IS_OBJECT(VioValue) && AS_OBJECT(VioValue)->type == ObjectType)
#define IS_STRING(VioValue) IS_OBJECT_TYPE(VioValue, ObjectType::STRING)
#define IS_CODE(VioValue) IS_OBJECT_TYPE(VioValue, ObjectType::CODE)

#endif