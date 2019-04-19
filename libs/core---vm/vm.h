#ifndef _PXT_VM_H
#define _PXT_VM_H

#include <pthread.h>

#define VM_MAGIC0 0x000a34365458500aULL // \nPXT64\n\0
#define VM_MAGIC1 0x6837215e2bfe7154ULL

#define VM_OPCODE_BASE_SIZE 7 // up to 127 base opcodes
#define VM_OPCODE_BASE_MASK ((1 << VM_OPCODE_BASE_SIZE) - 1)

#define VM_FUNCTION_CODE_OFFSET 24

#define VM_NUM_CPP_METHODS 4

// maximum size (in words) of stack in a single function
#define VM_MAX_FUNCTION_STACK 200
#define VM_STACK_SIZE 1000

#define VM_ENCODE_PC(pc) ((TValue)(((pc) << 9) | 2))
#define VM_DECODE_PC(pc) (((uintptr_t)pc) >> 9)
#define TAG_STACK_BOTTOM VM_ENCODE_PC(1)

#define DLLEXPORT extern "C"


namespace pxt {

struct FiberContext;
typedef void (*OpFun)(FiberContext *ctx, unsigned arg);
typedef void (*ApiFun)(FiberContext *ctx);

// keep in sync with backvm.ts
enum class SectionType : uint8_t {
    Invalid = 0x00,

    // singular sections
    InfoHeader = 0x01,       // VMImageHeader
    OpCodeMap = 0x02,        // \0-terminated names of opcodes and APIs (shims)
    NumberLiterals = 0x03,   // array of boxed doubles and ints
    ConfigData = 0x04,       // sorted array of pairs of int32s; zero-terminated
    IfaceMemberNames = 0x05, // array of 32 bit offsets, that point to string literals

    // repetitive sections
    Function = 0x20,
    Literal = 0x21, // aux field contains literal type (string, hex, image, ...)
    VTable = 0x22,
};

struct VMImageSection {
    SectionType type;
    uint8_t flags;
    uint16_t aux;
    uint32_t size; // in bytes, including this header
    uint8_t data[0];
};

struct OpcodeDesc {
    const char *name;
    OpFun fn;
    int numArgs;
};

struct IfaceEntry {
    uint16_t memberId;
    uint16_t aux;
    uint32_t method;
};

extern const OpcodeDesc staticOpcodes[];

struct VMImageHeader {
    uint64_t magic0;
    uint64_t magic1;
    uint64_t hexHash;
    uint64_t programHash;

    uint32_t allocGlobals;
    uint32_t nonPointerGlobals;
};

struct VMImage {
    TValue *numberLiterals;
    TValue *pointerLiterals;
    OpFun *opcodes;
    int32_t *configData;
    uintptr_t *ifaceMemberNames;

    uint64_t *dataStart, *dataEnd;
    VMImageSection **sections;
    VMImageHeader *infoHeader;
    const OpcodeDesc **opcodeDescs;
    RefAction *entryPoint;

    uint32_t numSections;
    uint32_t numNumberLiterals;
    uint32_t numConfigDataEntries;
    uint32_t numOpcodes;
    uint32_t numIfaceMemberNames;
    uint32_t errorCode;
    uint32_t errorOffset;
    int toStringKey;
};

// not doing this, likely
struct StackFrame {
    StackFrame *caller;
    uint32_t *retPC;
    TValue *stackBase;
    uint32_t *fnbase;
};

struct FiberContext {
    FiberContext *next;

    uint16_t *imgbase;
    VMImage *img;
    uint16_t *pc;
    uint16_t *resumePC;
    uint16_t *foreverPC;
    TValue *sp;
    TValue r0;
    RefAction *currAction;

    TValue *stackBase;
    TValue *stackLimit;

    // wait_for_event
    int waitSource;
    int waitValue;

    // for sleep
    uint64_t wakeTime;
};

extern VMImage *vmImg;
extern FiberContext *currentFiber;

void vmStart();
VMImage *loadVMImage(void *data, unsigned length);
VMImage *setVMImgError(VMImage *img, int code, void *pos);
void exec_loop(FiberContext *ctx);

#define DEF_CONVERSION(retp, tp, btp)                                                              \
    static inline retp tp(TValue v) {                                                              \
        if (!isPointer(v))                                                                         \
            failedCast(v);                                                                         \
        if (getVTable((RefObject *)v)->classNo != btp)                                             \
            failedCast(v);                                                                         \
        return (retp)v;                                                                            \
    }

DEF_CONVERSION(RefCollection *, asRefCollection, BuiltInType::RefCollection)
DEF_CONVERSION(RefAction *, asRefAction, BuiltInType::RefAction)
DEF_CONVERSION(RefRefLocal *, asRefRefLocal, BuiltInType::RefRefLocal)
DEF_CONVERSION(RefMap *, asRefMap, BuiltInType::RefMap)

DEF_CONVERSION(Buffer, asBuffer, BuiltInType::BoxedBuffer)
DEF_CONVERSION(Image_, asImage_, BuiltInType::RefImage)

String convertToString(FiberContext *ctx, TValue v);

} // namespace pxt

#endif