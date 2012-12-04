set(SOURCES
  parallelcoordinatesdialog.cpp
  parallelcoordinatesplugin.cpp
)

qt4_wrap_ui(UI_SOURCES
  parallelcoordinatesdialog.ui
)

# Need to add current binary dir to include path for UI file headers
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(chemdata_parallel_coordinates_plugin ${SOURCES} ${UI_SOURCES})
set_target_properties(chemdata_parallel_coordinates_plugin PROPERTIES AUTOMOC TRUE)
target_link_libraries(chemdata_parallel_coordinates_plugin
  ChemDataCore ChemDataGui
  vtkChartsCore vtkGUISupportQt vtkRenderingQt
)
