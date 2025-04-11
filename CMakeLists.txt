cmake_minimum_required(VERSION 3.19)

set(CMAKE_C_COMPILER "/usr/bin/gcc-11")
set(CMAKE_CXX_COMPILER "/usr/bin/g++-11")
project(basic_lifter)

include(GNUInstallDirs)
include("cmake/llvm.cmake")

# Find `elfio` from vcpkg
find_package(elfio CONFIG REQUIRED)

# Find `llvm`
set(_VCPKG_SHARE_DIR "./vcpkg_installed/${VCPKG_TARGET_TRIPLET}/${CMAKE_INSTALL_DATADIR}")
find_llvm(llvm "${_VCPKG_SHARE_DIR}")

# Find `remill` itself
find_package(remill CONFIG REQUIRED)

if(ENABLE_INSTALL)
  export(PACKAGE "${PROJECT_NAME}")
  
  set(cmake_install_dir "lib/cmake/${PROJECT_NAME}")
  
  include(CMakePackageConfigHelpers)
  configure_package_config_file("${PROJECT_NAME}Config.cmake.in"
      "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
      INSTALL_DESTINATION "${cmake_install_dir}"
  )

  install(EXPORT "${PROJECT_NAME}Targets"
      DESTINATION "${cmake_install_dir}"
      NAMESPACE "${PROJECT_NAME}::"
  )
  install(
    TARGETS
      "llvm"
    EXPORT
      "${PROJECT_NAME}Targets"
  )
endif()

file(GLOB_RECURSE SOURCES
    src/*.hpp
    src/*.cpp
)

add_executable(main src/main.cpp src/disass/Disassembler.cpp src/disass/Disassembler.hpp src/elf.cpp src/elf.hpp src/lifter/lifter.hpp src/lifter/lifter.cpp)
target_link_libraries(main PRIVATE elfio::elfio llvm remill)