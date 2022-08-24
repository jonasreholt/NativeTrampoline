#include <sys/mman.h>
#include <iostream>
#include <stdexcept>

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

        _bufferSize = size;
        _index = 0;
    }

    void Add(int8_t* opcode, size_t n) {
        if (n + _index >= _bufferSize) {
            throw std::invalid_argument("No room for opcode in code buffer.");
        }

        for (size_t i = 0; i < n; i++, _index++) {
            _buffer[_index] = opcode[i];
        }
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
    static testFun Jump(T* instance, void (*functionToJump)(T*)) {
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
        int8_t arr[] = { (int8_t)0x55,  // pushq %rbp
            (int8_t)0x48, (int8_t)0x89, (int8_t)0xec }; // movq #rbp, %rsp
        buffer->Add(arr, 4);
    }

    static void MaintainFramePointerEnd(CodeBuffer* buffer) {
        int8_t arr[] { (int8_t)0x5d };
        buffer->Add(arr, sizeof(arr));
    }

    static void Return(CodeBuffer* buffer) {
        int8_t arr[] { (int8_t)0xc3 };
        buffer->Add(arr, sizeof(arr));
    }

    static void AddInstancePointerToArgs(CodeBuffer* buffer, intptr_t instancePtr) {
        // movq %rdi <- instancePtr
        int8_t arr[] { (int8_t)0x48, (int8_t)0xbf,
            (int8_t)instancePtr, (int8_t)(instancePtr>>8), (int8_t)(instancePtr>>16),
            (int8_t)(instancePtr>>24), (int8_t)(instancePtr>>32), (int8_t)(instancePtr>>40),
            (int8_t)(instancePtr>>48), (int8_t)(instancePtr>>56) };
        buffer->Add(arr, sizeof(arr));
    }

    static void PrepareFunctionPointer(CodeBuffer* buffer, intptr_t functionPtr) {
        // movq %rax <- functionPtr
        int8_t arr[] { (int8_t)0x48, (int8_t)0xA1,
            (int8_t)functionPtr, (int8_t)(functionPtr>>8), (int8_t)(functionPtr>>16),
            (int8_t)(functionPtr>>24), (int8_t)(functionPtr>>32), (int8_t)(functionPtr>>40),
            (int8_t)(functionPtr>>48), (int8_t)(functionPtr>>56) };
        buffer->Add(arr, sizeof(arr));
    }

    static void CallFunction(CodeBuffer* buffer) {
        // call %rax
        int8_t arr[] { (int8_t)0x5d, (int8_t)0xd0 };
        buffer->Add(arr, sizeof(arr));
    }
};

int main() {
    auto fish = new Fish(12);

    auto fun = Trampoline::Jump<Fish>(fish, &cThingy);

    (*fun)();
}