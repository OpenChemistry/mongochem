find_package(MongoDB)
include_directories(SYSTEM ${MongoDB_INCLUDE_DIR})
# MongoDB built with unicode strings
add_definitions("-D_UNICODE")
if(WIN32)
  if(MSVC)
    add_definitions("-D_CRT_SECURE_NO_WARNINGS")
  endif()
  set(Boost_USE_STATIC_LIBS ON)
endif()

# MongoDB uses several boost components we need to link in
find_package(Boost COMPONENTS thread chrono filesystem program_options system iostreams)
if(WIN32)
  add_definitions("-DBOOST_ALL_NO_LIB")
endif()
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

function(add_chemdata_library name)
  # create the library
  add_library(${name} ${ARGN})

  # generate the export.h header
  generate_export_header(${name} EXPORT_FILE_NAME export.h)
  include_directories(${CMAKE_CURRENT_BINARY_DIR})
endfunction()

# needed for the export headers
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../)

add_subdirectory(core)
add_subdirectory(gui)
add_subdirectory(rpc)
add_subdirectory(plugins)
add_subdirectory(app)
