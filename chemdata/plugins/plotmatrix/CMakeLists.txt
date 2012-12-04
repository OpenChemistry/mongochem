set(SOURCES
  plotmatrixdialog.cpp
  plotmatrixplugin.cpp
)

qt4_wrap_ui(UI_SOURCES
  plotmatrixdialog.ui
)

# Need to add current binary dir to include path for UI file headers
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(chemdata_plot_matrix_plugin ${SOURCES} ${UI_SOURCES})
set_target_properties(chemdata_plot_matrix_plugin PROPERTIES AUTOMOC TRUE)
target_link_libraries(chemdata_plot_matrix_plugin
  ChemDataCore ChemDataGui
  vtkChartsCore vtkGUISupportQt vtkRenderingQt
)
