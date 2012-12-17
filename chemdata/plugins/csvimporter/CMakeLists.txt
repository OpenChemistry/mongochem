set(SOURCES
  importcsvfiledialog.cpp
  csvimporterplugin.cpp
)

qt4_wrap_ui(UI_SOURCES
  importcsvfiledialog.ui
)

# Need to add current binary dir to include path for UI file headers
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(chemdata_csvimporter_plugin ${SOURCES} ${UI_SOURCES})
set_target_properties(chemdata_csvimporter_plugin PROPERTIES AUTOMOC TRUE)
target_link_libraries(chemdata_csvimporter_plugin
  ChemDataCore ChemDataGui
)
