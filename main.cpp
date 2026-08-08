#include <iostream>
#include "Chunk.hpp"
#include "Compiler.hpp"
#include "VM.hpp"
#include "NativeBindings.hpp"

int main() {
    const char* source = "print(1000 + 250 - 150);";
    std::cout << "[Sandboxed JS Engine] Running script: " << source << "\n";

    // VM's constructor registers core natives (print, sqrt, abs, ...)
    // internally now, so there's no separate registration call here.
    VM vm;
    Chunk chunk;
    Compiler compiler(source, &chunk, vm);

    try {
        if (!compiler.compile()) {
            std::cerr << "[Error] Compilation failed.\n";
            return 1;
        }

        InterpretResult result = vm.interpret(&chunk);
        if (result != InterpretResult::Ok) {
            std::cerr << "[Error] Execution failed inside sandbox.\n";
            return 1;
        }

        std::cout << "[Success] Execution finished safely inside sandbox.\n";
    }
    catch (const std::exception& e) {
        std::cerr << "[Error] " << e.what() << "\n";
        return 1;
    }

    return 0;
}
