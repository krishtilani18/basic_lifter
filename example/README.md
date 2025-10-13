# Lifting examples

In this folder are a collection of simple programs, designed for testing the lifter's functionalities.

To make your own testcases, simply make a C file in the `c` folder, and create a binary in the `obj` folder using `gcc`:
```bash
gcc c/custom.c -o obj/custom
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

# 2. Run the lifter on the generated object file.
./basic_lifter ../example/obj/custom > custom.ll

# 3. Link the IR files together into one binary.
clang++ -o custom custom.ll Intrinsics.ll
```
