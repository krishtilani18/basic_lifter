vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO lifting-bits/remill
    REF 7f091d4256060c254fcd15f35fd8b9bd6abd157f
    SHA512 0b7973de67ac9bf6c7c96a8b709293346108381193461de215b3cbe90416bf0c4df0673d8afb0e59c1955346c1a3258ee454c36cc19accbfc1ca67716dd93597
    HEAD_REF master
    PATCHES
        # We're only building for x86, so remove all libraries of LLVM
        # that Remill originally requires that are for non-x86 platforms
        0001-llvm-remove-non-x86.patch
        # Because we're building Remill as a port using LLVM, it will no
        # longer be a Git repository - thus, remove all references to Git
        0002-remove-git-references.patch
        # Miscellaneous bug fixes
        0003-cpp-bugfixes.patch
        # Our version of `clang` may look in a later version of g++ than we
        # want, use g++-11 explicitly
        0004-clang-include-gpp11.patch
)

set(VCPKG_BUILD_TYPE release)
# trail-of-bits uses its own forks of Ghidra and Sleigh, which are downloaded
# locally, messing with `vcpkg`, so we have to disable this option
set(VCPKG_CMAKE_CONFIGURE_OPTIONS "-DFETCHCONTENT_FULLY_DISCONNECTED=false" "-DREMILL_BUILD_SPARC32_RUNTIME=False")
vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()
vcpkg_copy_tools(
    TOOL_NAMES remill-lift-17
    DESTINATION "${CURRENT_PACKAGES_DIR}/tools/remill"
    AUTO_CLEAN
)

set(VCPKG_POLICY_EMPTY_INCLUDE_FOLDER enabled)

file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
