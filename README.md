# Light JS Engine

Light JS Engine is a sandboxed, garbage-collected lightweight JavaScript engine written in C++. It compiles a small JS-like language to bytecode and executes it on a protected stack-based VM, with instruction quotas, call-depth limits, and output budgets enforced at runtime. Native functions (print, Math-style helpers) are exposed through a restricted, allocation-tracked binding layer with no filesystem, network, process, or OS-entropy access.

## Project Structure

- `main.cpp`: Entry point and execution harness
- `Lexer.hpp`: Tokenizer for script source (numbers, strings, identifiers, keywords, operators, comments)
- `Compiler.hpp`: Single-pass Pratt parser and bytecode compiler
- `Chunk.hpp`: Bytecode storage, constants table, line tracking, and disassembler
- `Value.hpp`: Dynamic value representation (undefined, null, boolean, number, object) and the object model used by the garbage collector
- `VM.hpp`: Stack-based virtual machine, instruction metering, call-depth limiting, and mark-sweep garbage collection
- `NativeBindings.hpp`: Sandboxed native function registry (print, sqrt, abs, pow, floor, ceil, min, max) with an output budget and no OS-level access
- `.gitignore`: Excluded build artifacts and system files

## Current Capabilities

- Expression statements: numbers, strings, `true`/`false`/`null`, unary `-`/`!`, binary `+ - * /`, comparisons (`== != < <= > >=`), string concatenation via `+`
- Global native function calls, e.g. `print(1 + 2);`, `print(sqrt(16));`
- Panic-mode error recovery during compilation (reports multiple syntax errors per run instead of stopping at the first one)
- Sandboxing limits enforced by the VM:
  - Instruction quota (default 10,000 per `interpret()` call), guarding against infinite loops
  - Fixed-size value stack (256 slots) with overflow checking
  - Call-depth limit (default 64), guarding against runaway recursion
  - Output budget on `print` (default 64 KB per run), guarding against unbounded stdout writes
- Mark-sweep garbage collection for heap-allocated values (currently strings); native function objects are allocated once at VM startup and live for the VM's lifetime

## Not Yet Implemented

- Variable declarations (`var` / `let` / `const`) and assignment
- Control flow (`if` / `else`, `while`, `for`)
- User-defined functions and closures
- Local variable scoping (only global lookups exist today)
- Arrays and objects (only strings exist as a reference type so far)
- Template literals and regular expressions

## Building and Running

```
g++ -std=c++17 -o light-js main.cpp
./light-js
```

`main.cpp` currently compiles and runs a single hardcoded script string as a demonstration harness. Point it at a different source string, or extend it to read from a file or stdin, to run other scripts.
