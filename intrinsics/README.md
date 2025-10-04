# Intrinsic functions

The lifting library that we use, [Remill](https://github.com/lifting-bits/remill), is
agnostic - to fit a wide variety of purposes (static analysis, reverse engineering),
Remill purposefully chooses to not define parts of its system. Two main things that
users have to implement themselves are:

- A **program memory representation**. Because Remill lifts binary procedures into
  functions that *simulate* what a processor would do, theoretically *any* way to
  represent program memory would work.
- **Intrinsic functions**. These are low-level operations that interact with memory
  in some way, like `__remill_read_memory_32` (which reads 4 bytes from memory), or
  perform some basic functionality (comparisons, indicating function calls/returns).

Currently, program memory is presented as a C++ `std::map<uint64_t, uint8_t>`, which maps
64-bit addresses to the information they hold. Ideally this should be a C struct, as C++
mangles class names and a `std::map` introduces lots of overhead (which can be found once
we create `Intrinsics.ll`).

Intrinsics are written on an ad-hoc basis, so some intrinsics have not been implemented
yet. If you build more complex programs and run into issues with undefined functions
starting with `__remill`, it means that you will need to write an implementation in
`Intrinsics.cpp`. For our use-case, if these functions don't interact with memory, they
can just be no-ops.
