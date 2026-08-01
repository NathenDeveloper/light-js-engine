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

## Requirements

- A modern C++ compiler supporting C++17 or later (g++, clang++, or MSVC)
- Make or terminal access for compilation

## Building and Running

To compile and run the engine using g++:

```bash
g++ -O3 main.cpp -o weak_js_engine.exe
./weak_js_engine.exe
> ^C

IMONX@Nathen MINGW64 ~ (main)
$ git add LICENSE README.md
git commit -m "Pushing docs to github"
git push origin main
fatal: pathspec 'README.md' did not match any files
warning: could not open directory 'AppData/Local/Application Data/': Permission denied
warning: could not open directory 'Application Data/': Permission denied
warning: could not open directory 'Cookies/': Permission denied
warning: could not open directory 'Documents/My Music/': Permission denied
warning: could not open directory 'Documents/My Pictures/': Permission denied
warning: could not open directory 'Documents/My Videos/': Permission denied
warning: could not open directory 'Local Settings/': Permission denied
warning: could not open directory 'My Documents/': Permission denied
warning: could not open directory 'NetHood/': Permission denied
warning: could not open directory 'PrintHood/': Permission denied
warning: could not open directory 'Recent/': Permission denied
warning: could not open directory 'SendTo/': Permission denied
warning: could not open directory 'Start Menu/': Permission denied
warning: could not open directory 'Templates/': Permission denied
On branch main
Your branch is up to date with 'origin/main'.

Changes not staged for commit:
  (use "git add <file>..." to update what will be committed)
  (use "git restore <file>..." to discard changes in working directory)
        modified:   Native.hpp

Untracked files:
  (use "git add <file>..." to include in what will be committed)
        .bash_history
        .bash_profile
        .bashrc
        .config/
        .gitconfig
        AppData/
        Contacts/
        Downloads/
        Favorites/
        LICENSE
        Links/
        Linux_x64%2F1670261%2Fchrome-linux.zip
        Music/
        NTUSER.DAT
        NTUSER.DAT{62983140-8a67-11f1-8fe2-f1ca2da7fd9c}.TM.blf
        NTUSER.DAT{62983140-8a67-11f1-8fe2-f1ca2da7fd9c}.TMContainer00000000000000000001.regtrans-ms
        NTUSER.DAT{62983140-8a67-11f1-8fe2-f1ca2da7fd9c}.TMContainer00000000000000000002.regtrans-ms
        OneDrive/
        Saved Games/
        Searches/
        SimpleJIT.h
        Videos/
        Win_x64%2F1670191%2Fchrome-win.zip
        chrome-win.zip
        chrome-win/
        index.html
        js_cli_engine.cpp
        js_engine/
        js_runner.cpp
        llvm-dev.zip
        ntuser.dat.LOG1
        ntuser.dat.LOG2
        ntuser.ini
        retro_browser/
        search_engine/
        servo/
        src/
        test262/
        true_jit_engine.cpp

no changes added to commit (use "git add" and/or "git commit -a")
Everything up-to-date

IMONX@Nathen MINGW64 ~ (main)
$ cat << 'EOF' > LICENSE
MIT License

Copyright (c) 2026 NathenDeveloper

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
