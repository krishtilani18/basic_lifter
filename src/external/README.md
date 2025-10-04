# Externally-defined functions

We have decided to work on *dynamically-linked binaries* for this lifter -
while our lifting approach theoretically works for statically-linked binaries,
we wanted to work with smaller binaries for our MVP.

The problem with dynamically-linked binaries is that some functions are defined
in external libraries, which we can't rewrite directly using the [Remill ABI](https://github.com/krishtilani18/basic_lifter/wiki/Remill-ABI).
We can work with these external functions by defining them in C++.

## Example: `printf`

For example, we want to try and implement the C standard library function `printf`,
which has a function prototype of:

```c
printf(const char *__restrict __fmt, ...);
```

In this example, we'll consider `printf` only having one extra argument.

Because `printf` is declared as a symbol that is externally-defined in our binary,
the lifter will generate an LLVM function `@EXTERNAL.printf` in our lifted `.ll` file,
following the Remill ABI. Thus, we need to define a function in `External.cpp` that
*implements* `@EXTERNAL.printf` - it takes in three arguments (state pointer, program
counter, memory pointer) and calls `printf` with its correct arguments. An example
implementation is like so:

```cpp
Memory *EXTERNAL_printf(State *state, addr_t program_counter, Memory *memory) {
    // 2nd argument stored in %esi
    int value = state->gpr.rsi.dword;

    // Currently, `.rodata` is not integrated, so use fixed value
    // to demonstrate functionality
    printf("%d\n", value);
    
    return memory;
}
```

Once `printf` is implemented, we can build the `.ll` file using this command from the
`build` folder:

```sh
clang++ -c -S -emit-llvm ../src/external/External.cpp -I"./vcpkg_installed/x64-linux-release/include" -o External.ll
```

Because we are building using C++, the function name is *mangled*, so we have to
manually go into the `.ll` file and rename it to `@EXTERNAL.printf`, which is what
our lifted `.ll` file expects.

Then, our `.ll` file can finally be rebuilt:

```sh
clang++ -o custom custom.ll Intrinsics.ll External.ll
```
