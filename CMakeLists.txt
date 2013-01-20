cmake_minimum_required(VERSION 2.8.6 FATAL_ERROR)

project(MongoChem)

include(CTest)
enable_testing()

set(CMAKE_MODULE_PATH ${MongoChem_SOURCE_DIR}/cmake)

include(BuildType)
include(BuildLocation)
include(CompilerFlags)
include(InstallLocation)

# Set up our version.
set(MongoChem_VERSION_MAJOR   "0")
set(MongoChem_VERSION_MINOR   "1")
set(MongoChem_VERSION_PATCH   "0")
set(MongoChem_VERSION
  "${MongoChem_VERSION_MAJOR}.${MongoChem_VERSION_MINOR}.${MongoChem_VERSION_PATCH}")

#option(BUILD_SHARED_LIBS "Build with shared libraries" ON)
# FIXME: Shared libs doesn't work right now.
set(BUILD_SHARED_LIBS OFF)

# Before any plugins are defined, and before any add_subdirectory calls:
set_property(GLOBAL PROPERTY MongoChem_PLUGINS)
set_property(GLOBAL PROPERTY MongoChem_STATIC_PLUGINS)

include_directories(BEFORE ${CMAKE_CURRENT_SOURCE_DIR})

option(ENABLE_TESTING "Enable testing and building the tests." OFF)

if(ENABLE_TESTING)
  include(CTest)
  enable_testing()
endif()

# source directory
add_subdirectory(mongochem)

option(BUILD_DOCUMENTATION "Build project documentation" OFF)

if(BUILD_DOCUMENTATION)
  add_subdirectory(docs)
endif()

include(CPack)

if(ENABLE_TESTING)
  include(BuildPackageTest)
  BuildPackageTest_Add("MongoChem" "${CMAKE_CURRENT_BINARY_DIR}")
  add_subdirectory(tests)
endif()
