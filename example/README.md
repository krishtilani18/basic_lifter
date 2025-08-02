# Lifting examples

In this folder are a collection of simple programs, designed for testing the lifter's functionalities. Currently, programs that do not require external functions (this includes functions that are part of the C standard library, like `printf`) work as expected.

To create your own C programs that test the lifter's capabilities, using the example `c/custom.c`:
```bash
# 1. Compile your code down to human-readable LLVM IR.
clang -c -S -emit-llvm c/custom.c -o ll/custom.ll

# 2. Compile the LLVM IR down to x86 assembly code. `llc-18` corresponds with LLVM 18 - if you
#    have a different version of LLVM installed, the command might be different.
llc-18 ll/custom.ll

# 3. Assemble your x86 code (by default a `.s` file with the same name in the same folder) into
#    an x86 object file.
as ll/custom.s -o obj/custom.o

# 4. If you have multiple functions, you will need to change the program's entry point to `main`.
ld obj/custom.o -e main -o obj/custom_main.o
rm obj/custom.o
mv obj/custom_main.o obj/custom.o
```

To lift and run the generated object file, go to the `build` folder (where your `./basic_lifter` should be) and run the following commands:
```bash
# 5. If you haven't already, compile the intrinsics file into LLVM IR.
clang++ -c -S -emit-llvm ../intrinsics/Intrinsics.cpp -o Intrinsics.ll

# 6. Run the lifter on the generated object file.
./basic_lifter ../example/obj/custom.o > custom.ll

# 7. Link the IR files together into one binary.
clang++ -o custom custom.ll Intrinsics.ll
```
