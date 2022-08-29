/*
 * A simple code buffer usable for JIT compiled code.
*/

#include <cstdint>
#include <sys/mman.h>
#include <memory.h>
#include <assert.h>


class CodeBuffer {
public:
    CodeBuffer(size_t size) {
        _buffer = (uint8_t *)mmap(
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

    uint8_t* GetFunctionPointer() {
        return _buffer;
    }

private:
    size_t _index;

    size_t _bufferSize;
    uint8_t* _buffer;
};