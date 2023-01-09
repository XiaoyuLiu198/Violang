//  define the VioValue type

#ifdef VioValue_h
#define VioValue_h

// construct VioValue type enum class
enum class VioValueType{
    NUMBER,
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

// accessors

#define AS_NUMBER(vioValue) ((double)(vioValue).number)

#endif