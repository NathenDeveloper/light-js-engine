#include <iostream>
#include "Chunk.hpp"
#include "Compiler.hpp"
#include "VM.hpp"
#include "Native.hpp"

int main() {
    NativeBindings::registerCoreBindings();

    const char* source = "1000 + 250 - 150";
    std::cout << "[Sandboxed JS Engine] Running script: " << source << "\n";

    Chunk chunk;
    Compiler compiler(source, &chunk);

    try {
        if (compiler.compile()) {
            VM vm;
            vm.interpret(&chunk);
            std::cout << "[Success] Execution finished safely inside sandbox.\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "[Error] " << e.what() << "\n";
        return 1;
    }

    return 0;
}
