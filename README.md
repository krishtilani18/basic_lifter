# basic_lifter

This project aims to be an MVP for lifting x86 binaries into LLVM IR, for static analysis purposes.

## Architecture

`basic_lifter` uses the following libraries to achieve its goals:
- [Remill](https://github.com/lifting-bits/remill) is a general-purpose library that translates machine code instructions into LLVM IR.
- [ELFIO](https://elfio.sourceforge.net/) is a library that can read and generate ELF (x86 executable) files.

Remill itself performs most of the heavy lifting, but can only process *instruction bytes*, not general executables - thus a library like ELFIO is necessary to extract where the code is.

## Project structure

The following are important folders/files within the project:
- `/bin` contains example C programs and their compiled binaries, to test our lifter on.
- `/cmake` contains extra CMake scripts for the build system.
- `/ports` contains libraries that aren't accessible directly through the package manager, `vcpkg` - Remill is one such package.

## Requirements

Make sure the following are installed before building:
- GCC/G++ 11 - read [this guide](https://linuxconfig.org/how-to-switch-between-multiple-gcc-and-g-compiler-versions-on-ubuntu-20-04-lts-focal-fossa) and make sure to have GCC/G++ 11 set as the desired version.
- [`vcpkg`](https://vcpkg.io/) - read [this guide](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-bash) for installation instructions.
- [`ninja`](https://ninja-build.org/)

## Building

Before building, create a file `CMakeUserPresets.json` in the root directory, with the following information:

```json
{
    "version": 2,
    "configurePresets": [
        {
            "name": "default",
            "inherits": "vcpkg",
            "environment": {
                "VCPKG_ROOT": "~/.vcpkg"
            },
            "cacheVariables": {
                "VCPKG_TARGET_TRIPLET": "x64-linux-release",
                "VCPKG_HOST_TRIPLET": "x64-linux-release"
            }
        }
    ]
}
```

Then, create a `/build` folder and run the following instructions inside that folder:

```
cmake .. --preset default
ninja
```

To run the lifter, simply do `./basic_lifter <BINARY_PATH>`.

## Testing

Remill provides a basic tool, `remill-lift`, which is documented [here](https://github.com/lifting-bits/remill/tree/master/bin/lift). It can be found in `./build/vcpkg_installed/x64-linux-release/tools/remill-lift-17` (17 being the LLVM version).
