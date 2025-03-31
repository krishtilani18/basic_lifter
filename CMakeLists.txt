cmake_minimum_required(VERSION 3.19)

project(basic_lifter)

include(GNUInstallDirs)
include("cmake/llvm.cmake")

# Find `zydis` from vcpkg
find_package(zydis CONFIG REQUIRED)

# `remill` setup
set(_VCPKG_SHARE_DIR "./vcpkg_installed/${VCPKG_TARGET_TRIPLET}/${CMAKE_INSTALL_DATADIR}")

find_package(XED CONFIG REQUIRED HINTS "${_VCPKG_SHARE_DIR}")
find_package(gflags CONFIG REQUIRED HINTS "${_VCPKG_SHARE_DIR}")
find_package(glog CONFIG REQUIRED HINTS "${_VCPKG_SHARE_DIR}")
find_package(Z3 CONFIG REQUIRED)
find_llvm(llvm "${_VCPKG_SHARE_DIR}")

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

add_executable(main src/main.cpp)
target_link_libraries(main PRIVATE Zydis::Zydis llvm remill)
