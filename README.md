# x86 bitcode -> LLVM IR lifter

This project is the first attempt at a binary (x86) lifter into LLVM IR. It relies on two libraries for different stages of the lifting process:

## x86 bitcode -> x86 instructions

`zydis` is used for this.

## x86 instructions -> LLVM IR

`remill` is used for this.
