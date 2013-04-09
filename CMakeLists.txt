cmake_minimum_required(VERSION 2.8.6 FATAL_ERROR)

project(MongoChem)

set(CMAKE_MODULE_PATH ${MongoChem_SOURCE_DIR}/cmake)

include(BuildType)
include(BuildLocation)
include(CompilerFlags)
include(InstallLocation)
include(DetermineVersion)

# Set up our version.
set(MongoChem_VERSION_MAJOR   "0")
set(MongoChem_VERSION_MINOR   "1")
set(MongoChem_VERSION_PATCH   "0")
set(MongoChem_VERSION
  "${MongoChem_VERSION_MAJOR}.${MongoChem_VERSION_MINOR}.${MongoChem_VERSION_PATCH}")
find_package(Git)
determine_version(${MongoChem_SOURCE_DIR} ${GIT_EXECUTABLE} "MongoChem")

# Handle installation prefix for Mac bundles...
if(APPLE)
  set(MACOSX_BUNDLE_NAME "MongoChem")
  set(prefix "${MACOSX_BUNDLE_NAME}.app/Contents")
  set(INSTALL_RUNTIME_DIR "${prefix}/MacOS")
  set(INSTALL_INCLUDE_DIR "${prefix}/${INSTALL_INCLUDE_DIR}")
  set(INSTALL_LIBRARY_DIR "${prefix}/${INSTALL_LIBRARY_DIR}")
  set(INSTALL_ARCHIVE_DIR "${prefix}/${INSTALL_ARCHIVE_DIR}")
  set(INSTALL_DATA_DIR    "${prefix}/${INSTALL_DATA_DIR}")
  set(INSTALL_DOC_DIR     "${prefix}/${INSTALL_DOC_DIR}")
  set(INSTALL_CMAKE_DIR   "${prefix}/Resources")
endif()

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

# Make Boost behave on Windows...
if(WIN32)
  add_definitions("-DBOOST_ALL_NO_LIB")
  set(Boost_USE_STATIC_LIBS ON)
endif()

# source directory
add_subdirectory(mongochem)

option(BUILD_DOCUMENTATION "Build project documentation" OFF)

if(BUILD_DOCUMENTATION)
  add_subdirectory(docs)
endif()

install(
  FILES
    README.md
    CONTRIBUTING.md
    COPYING
  DESTINATION "${INSTALL_DOC_DIR}/mongochem")

include(MongoChemCPack)

if(ENABLE_TESTING)
  include(BuildPackageTest)
  BuildPackageTest_Add("MongoChem" "${CMAKE_CURRENT_BINARY_DIR}")
  add_subdirectory(tests)
endif()
