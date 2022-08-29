/*A trampoline that can be used to call instance member functions in native C code.
* To use it one must create a c handle function of the instance member function, that
* simply takes the instance as arguments, and calls the chosen function from that instance.
* The trampoline can then use a function pointer to the above, and give back a function
* pointer that takes zero arguments (It only works for instance member functions, that do
* not take any arguments). 
*/

#include <cstdint>

#include "codeBuffer.h"


class Trampoline {
public:
    typedef void (*testFun)();

    template<typename T>
    static testFun Jump0Param(T* instance, void (*functionToJump)(T*)) {
        const size_t codeLen = 4096;
        auto buffer = new CodeBuffer(codeLen);

        MaintainFramePointerStart(buffer);
        AddInstancePointerToArgs(buffer, (intptr_t)instance);
        PrepareFunctionPointer(buffer, (intptr_t)functionToJump);
        CallFunction(buffer);
        MaintainFramePointerEnd(buffer);
        Return(buffer);

        return (testFun) (buffer->GetFunctionPointer());
    }
private:
    const unsigned int BYTES_PR_PTR = 8;
    const unsigned int BITS_PR_BYTE = 8;

    static void MaintainFramePointerStart(CodeBuffer* buffer) {
        uint8_t arr[] = { (uint8_t)0x55,                    // pushq %rbp
            (uint8_t)0x48, (uint8_t)0x89, (uint8_t)0xec };  // movq #rbp, %rsp
        buffer->Add(arr, sizeof(arr));
    }

    static void MaintainFramePointerEnd(CodeBuffer* buffer) {
        uint8_t arr[] { (uint8_t)0x5d };
        buffer->Add(arr, sizeof(arr));
    }

    static void Return(CodeBuffer* buffer) {
        uint8_t arr[] { (uint8_t)0xc3 };
        buffer->Add(arr, sizeof(arr));
    }

    static void AddInstancePointerToArgs(CodeBuffer* buffer, intptr_t ptr) {
        uint8_t arr[] {
            // movabs $imm, %rdi
            (uint8_t)0x48, (uint8_t)0xbf,
            (uint8_t)ptr, (uint8_t)(ptr>>8), (uint8_t)(ptr>>16), (uint8_t)(ptr>>24), (uint8_t)(ptr>>32), (uint8_t)(ptr>>40), (uint8_t)(ptr>>48), (uint8_t)(ptr>>56),};
        buffer->Add(arr, sizeof(arr));
    }

    static void PrepareFunctionPointer(CodeBuffer* buffer, intptr_t ptr) {
        // movabs $imm, %rax
        uint8_t arr[] { (uint8_t)0x48, (uint8_t)0xb8,
            (uint8_t)ptr, (uint8_t)(ptr>>8), (uint8_t)(ptr>>16), (uint8_t)(ptr>>24), (uint8_t)(ptr>>32), (uint8_t)(ptr>>40), (uint8_t)(ptr>>48), (uint8_t)(ptr>>56) };
        buffer->Add(arr, sizeof(arr));
    }

    static void CallFunction(CodeBuffer* buffer) {
        // call %rax
        uint8_t arr[] { (uint8_t)0xff, (uint8_t)0xd0 };
        buffer->Add(arr, sizeof(arr));
    }
};