#include <sys/mman.h>
#include <memory.h>
#include <iostream>
#include <assert.h>

class Fish {
private:
    int _classifier;
public:
    Fish(int c) {
        _classifier = c;
    }

    void Thingy() {
        std::cout << "I'm the fish " << _classifier << std::endl;
    }
};

// C handle
void cThingy(Fish* instance) { instance->Thingy(); }

// Here we go creating a trampoline for the C handle function.
class CodeBuffer {
public:
    CodeBuffer(size_t size) {
        _buffer = (unsigned char *)mmap(
            NULL,
            size,
            PROT_READ | PROT_WRITE | PROT_EXEC,
            MAP_ANONYMOUS | MAP_PRIVATE,
            0,
            0);;
        assert(_buffer != NULL && "Could not allocate code region for trampoline.");

        _bufferSize = size;
        _index = 0;
    }

    void Add(const uint8_t* opcode, size_t n) {
        assert(n + _index < _bufferSize && "No room for opcode in code buffer.");

        memcpy((void *)(_buffer + _index), opcode, n);
        _index += n;
    }

    unsigned char* GetFunctionPointer() {
        return _buffer;
    }

private:
    size_t _index;

    size_t _bufferSize;
    unsigned char* _buffer;
};

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

int main() {
    auto fish = new Fish(12);

    auto fun = Trampoline::Jump0Param<Fish>(fish, cThingy);

    (*fun)();
}