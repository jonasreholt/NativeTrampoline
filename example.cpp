/* Small example showing the usage of the native trampoline.
*/

#include <iostream>

#include "trampoline.h"

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

int main() {
    auto fish = new Fish(12);

    auto fun = Trampoline::Jump0Param<Fish>(fish, cThingy);

    (*fun)();
}