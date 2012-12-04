set(SOURCES
  histogramdialog.cpp
  histogramplugin.cpp
)

qt4_wrap_ui(UI_SOURCES
  histogramdialog.ui
)

# Need to add current binary dir to include path for UI file headers
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(chemdata_histogram_plugin ${SOURCES} ${UI_SOURCES})
set_target_properties(chemdata_histogram_plugin PROPERTIES AUTOMOC TRUE)
target_link_libraries(chemdata_histogram_plugin
  ChemDataCore ChemDataGui
  vtkChartsCore vtkGUISupportQt vtkRenderingQt
)
