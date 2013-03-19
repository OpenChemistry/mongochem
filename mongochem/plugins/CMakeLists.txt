# find Qt
find_package(Qt4 4.7 COMPONENTS QtCore QtGui REQUIRED)
include(${QT_USE_FILE})

# VTK is used for the charting and infovis components.
find_package(VTK COMPONENTS vtkChartsCore vtkGUISupportQt vtkViewsContext2D
  vtkViewsInfovis vtkRenderingQt vtkRenderingFreeTypeOpenGL NO_MODULE)
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS ${VTK_DEFINITIONS})
include_directories(SYSTEM ${VTK_INCLUDE_DIRS})

# Modify the output directory for the build tree.
set(original_library_output_dir "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
  "${original_library_output_dir}/mongochem/plugins")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY
  "${original_library_output_dir}/mongochem/staticplugins")

# Optionally build all plugins statically. Currently always forced to ON.
#option(BUILD_STATIC_PLUGINS "Build plugins statically by default" OFF)
# FIXME: Non-static plugins not working right now.
set(BUILD_STATIC_PLUGINS ON)

# Create a plugin for MongoChem.
# name is the name of the plugin, this will be the name of the target created.
# description Free text description of the plugin.
# type The base class of the plugin.
# header is the header(s) for the class to be instantiated for the plugin.
# pluginClass is the class to be instantiated for the plugin.
# sources is the list of source files for the plugin.
# uis is the list of UI files that need to be compiled (optional).
function(mongochem_plugin name description type header pluginClass sources uis)
  qt4_wrap_ui(ui_srcs ${uis})
  include_directories(${CMAKE_CURRENT_BINARY_DIR})
  unset(PluginIncludes)
  foreach(_header ${header})
    set(PluginIncludes
      "${PluginIncludes}#include \"${CMAKE_CURRENT_SOURCE_DIR}/${_header}\"\n")
  endforeach()
  set(PluginName "${name}")
  set(PluginDescription "${description}")
  set(PluginType "${type}")
  set(PluginClass "${pluginClass}")
  configure_file("${MongoChem_SOURCE_DIR}/cmake/mongochemplugin.cpp.in"
    "${CMAKE_CURRENT_BINARY_DIR}/${name}Plugin.cpp")

  # Figure out which type of plugin is being added, and put it in the right list
  if(BUILD_STATIC_PLUGINS)
    set(_plugin_object "STATIC")
    set_property(GLOBAL APPEND PROPERTY MongoChem_STATIC_PLUGINS ${name})
  else()
    set(_plugin_object "MODULE")
    set_property(GLOBAL APPEND PROPERTY MongoChem_PLUGINS ${name})
  endif()

  add_library(${name} ${_plugin_object} ${sources} ${ui_srcs} ${name}Plugin.cpp)
  target_link_libraries(${name} MongoChemGui)

  if("${_plugin_object}" STREQUAL "STATIC")
    set_target_properties(${name} PROPERTIES COMPILE_DEFINITIONS
      "QT_STATICPLUGIN")
    if(UNIX) # Need -fPIC for static plugins linked to shared libs on Unix.
      set_target_properties(${name} PROPERTIES COMPILE_FLAGS "-fPIC")
    endif()
  endif()

  set_target_properties(${name} PROPERTIES AUTOMOC TRUE PREFIX "")
  install(TARGETS ${name}
    EXPORT "MongoChemTargets"
    RUNTIME DESTINATION "${INSTALL_RUNTIME_DIR}"
    LIBRARY DESTINATION "${INSTALL_LIBRARY_DIR}/mongochem/plugins"
    ARCHIVE DESTINATION "${INSTALL_ARCHIVE_DIR}/mongochem/staticplugins")
endfunction()

include_directories(${MongoChem_SOURCE_DIR}/mongochem/gui)

add_subdirectory(csvimporter)
add_subdirectory(kmeansclustering)
add_subdirectory(histogram)
add_subdirectory(parallelcoordinates)
add_subdirectory(plotmatrix)
add_subdirectory(scatterplot)
add_subdirectory(sdfimporter)
add_subdirectory(similaritygraph)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${original_library_output_dir}")

# Add all of the static plugins to the initialization file.
get_property(MongoChem_STATIC_PLUGINS GLOBAL
  PROPERTY MongoChem_STATIC_PLUGINS)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/mongochemstaticqtplugins.h"
  "// Automatically generated file. Do not edit.
#ifndef MONGOCHEM_STATIC_QTPLUGINS_H
#define MONGOCHEM_STATIC_QTPLUGINS_H

#include <QtCore/QtPlugin>\n\n")

foreach(_plugin ${MongoChem_STATIC_PLUGINS})
  file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/mongochemstaticqtplugins.h"
    "Q_IMPORT_PLUGIN(${_plugin})\n")
endforeach()

file(APPEND "${CMAKE_CURRENT_BINARY_DIR}/mongochemstaticqtplugins.h"
  "\n#endif // MONGOCHEM_STATIC_QTPLUGINS_H\n")

# Now to build the plugin library, which can also statically link plugins
# and initialize them for the application. If MongoChem is built statically
# then the static plugin header must be included by the application.
set(HEADERS
  pluginmanager.h
  "${CMAKE_CURRENT_BINARY_DIR}/mongochemstaticqtplugins.h"
)

set(SOURCES
  pluginmanager.cpp
)

mongochem_add_private_library(MongoChemPlugins ${HEADERS} ${SOURCES})
set_target_properties(MongoChemPlugins PROPERTIES AUTOMOC TRUE)
target_link_libraries(MongoChemPlugins LINK_PUBLIC ${QT_QTCORE_LIBRARY}
  LINK_PRIVATE ${MongoChem_STATIC_PLUGINS})
