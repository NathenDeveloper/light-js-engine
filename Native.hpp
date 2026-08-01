#pragma once
#include <iostream>

namespace NativeBindings {
    inline void registerCoreBindings() {
        // Sandboxed environment: No system calls, file access, or unsafe hooks allowed.
    }
}
