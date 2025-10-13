# Lifting examples

In this folder are a collection of simple programs, designed for testing the lifter's functionalities.

To make your own testcases, simply make a C file in the `c` folder, and create a binary in the `obj` folder using `gcc`:
```bash
gcc c/custom.c -o obj/custom
```

Modern C compilers (like `gcc`) actually compile binaries with an extra function called before `main`, `_start`. This function
instantiates the C runtime, and can differ between compilers and architectures. Because `_start` calls external functions and
is currently unsupported (it passes the address of `main` into another function to actually run `main`, which doesn't play well
with Remill's virtual memory system), you will need to use a tool like `objdump` to find the "functional" entry point of the program,
which for most intents and purposes is the address of `main`:

```
objdump -d obj/custom
# ...
0000000000001149 <main>:
    1149:       f3 0f 1e fa             endbr64
    114d:       55                      push   %rbp
    114e:       48 89 e5                mov    %rsp,%rbp
    1151:       48 83 ec 10             sub    $0x10,%rsp
    1155:       c7 45 f8 01 00 00 00    movl   $0x1,-0x8(%rbp)
# ...
```

To lift and run the generated object file, go to the `build` folder (where your `./basic_lifter` should be) and run the following commands:
```bash
# 1. If you haven't already, compile the intrinsics file into LLVM IR.
clang++ -c -S -emit-llvm ../util/Intrinsics.cpp -I"../include" -o Intrinsics.ll

# 1.1. If you need to work with external functions, write your binding functions
# in `util/External.cpp` and compile that into LLVM IR as well.
# NOTE: make sure to add an extra -I flag for every other place where you can
# find header files you've included.
clang++ -c -S -emit-llvm ../util/External.cpp -I"../include" -I"./vcpkg_installed/x64-linux-release/include" -o External.ll

# 2. Run the lifter on the generated object file, supplying our "functional" entry
# point (in hex) as a command-line argument.
./basic_lifter ../example/obj/custom 1149 > custom.ll

# 3. Link the IR files together into one binary.
clang++ -o custom custom.ll Intrinsics.ll
```
