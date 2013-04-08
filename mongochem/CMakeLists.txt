find_package(MongoDB)
include_directories(SYSTEM ${MongoDB_INCLUDE_DIR})
# MongoDB built with unicode strings
add_definitions("-D_UNICODE")
if(MSVC)
  add_definitions("-D_CRT_SECURE_NO_WARNINGS")
endif()

# MongoDB uses several boost components we need to link in
find_package(Boost
  COMPONENTS thread chrono filesystem program_options system iostreams)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
# MongoDB also seems to require pthread on Linux (but not Apple)
if(UNIX AND NOT APPLE)
  find_package(Threads)
  set(EXTRA_LINK_LIB ${CMAKE_THREAD_LIBS_INIT})
endif()

# Chemkit is used for substructure queries
find_package(Chemkit COMPONENTS io md REQUIRED)
include_directories(SYSTEM ${CHEMKIT_INCLUDE_DIRS})
link_directories(${CHEMKIT_LIBRARY_DIR})

include(GenerateExportHeader)

# Add a library exposing API to external users.
function(mongochem_add_library name)
  add_library(${name} ${ARGN})
  # Use the new AUTOMOC support for Qt libraries (CMake 2.8.6).
  if(${name} MATCHES "^MongoChemQt")
    set_target_properties(${name} PROPERTIES AUTOMOC TRUE)
  endif()
  string(TOLOWER ${name} lowerName)
  # Generate the necessary export headers.
  generate_export_header(${name} EXPORT_FILE_NAME ${lowerName}export.h)
  list(APPEND HEADERS "${CMAKE_CURRENT_BINARY_DIR}/${lowerName}export.h")
  include_directories(${CMAKE_CURRENT_BINARY_DIR})
  if (BUILD_SHARED_LIBS)
    add_compiler_export_flags(mongochem_export_flags)
    set_property(TARGET ${name} APPEND
      PROPERTY COMPILE_FLAGS ${mongochem_export_flags})
  endif()
  # Now install everything.
  string(REGEX REPLACE "^mongochem" "" module ${lowerName})
  install(FILES ${HEADERS}
    DESTINATION "${INSTALL_INCLUDE_DIR}/mongochem/${module}")
  install(TARGETS ${name}
    EXPORT "MongoChemTargets"
    RUNTIME DESTINATION "${INSTALL_RUNTIME_DIR}"
    LIBRARY DESTINATION "${INSTALL_LIBRARY_DIR}"
    ARCHIVE DESTINATION "${INSTALL_ARCHIVE_DIR}")
endfunction()

# Add a private library, useful for testing/executable building. Not installed.
function(mongochem_add_private_library name)
  add_library(${name} STATIC ${ARGN})
  # Use the new AUTOMOC support for Qt libraries (CMake 2.8.6).
  if(${name} MATCHES "^MongoChemQt")
    set_target_properties(${name} PROPERTIES AUTOMOC TRUE)
  endif()
  string(TOLOWER ${name} lowerName)
  include_directories(${CMAKE_CURRENT_BINARY_DIR})
endfunction()

option(MongoChem_ENABLE_RPC "Enable RPC server" ON)

add_subdirectory(gui)
include_directories("${CMAKE_CURRENT_BINARY_DIR}/gui")
add_subdirectory(plugins)
add_subdirectory(app)
