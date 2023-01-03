//  define the VioValue type

#ifdef VioValue_h
#define VioValue_h

// construct VioValue type enum class
enum class VioValueType{
    NUMBER,
};

// VioValue
struct VioValue
{
    VioValueType type;
    union {
        double number;
    }
};

//  constructors
#define NUMBER(value) (VioValue)({VioValueType::NUMBER, .number = value})

// accessors

#define AS_NUMBER(vioValue) ((double)(vioValue).number)

#endif