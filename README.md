# Light JS Engine

Light JS Engine is a sandboxed, garbage-collected lightweight JavaScript engine written in C++. It executes bytecode expressions through a protected stack, meters instruction quotas safely, and integrates functional native runtime bindings.

## Project Structure

- main.cpp: Entry point and execution harness
- VM.hpp: Core stack-based virtual machine and instruction metering
- Compiler.hpp: Bytecode compiler
- Lexer.hpp: Tokenizer for script parsing
- Chunk.hpp: Bytecode chunk management
- Value.hpp: Dynamic value representation and garbage collection
- Native.hpp: Sandboxed native runtime bindings and function registry
- .gitignore: Excluded build artifacts and system files
