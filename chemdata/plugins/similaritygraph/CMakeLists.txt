set(SOURCES
  fingerprintsimilaritydialog.cpp
  similaritygraphwidget.cpp
  structuresimilaritydialog.cpp
  similaritygraphplugin.cpp
)

qt4_wrap_ui(UI_SOURCES
  fingerprintsimilaritydialog.ui
  structuresimilaritydialog.ui
)

# Need to add current binary dir to include path for UI file headers
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(chemdata_similarity_graph_plugin ${SOURCES} ${UI_SOURCES})
set_target_properties(chemdata_similarity_graph_plugin PROPERTIES AUTOMOC TRUE)
target_link_libraries(chemdata_similarity_graph_plugin
  ChemDataCore ChemDataGui
  vtkChartsCore vtkGUISupportQt vtkRenderingQt vtkViewsContext2D vtkViewsInfovis
)
