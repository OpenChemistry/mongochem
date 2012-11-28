set(SOURCES
  scatterplotdialog.cpp
  scatterplotplugin.cpp
)

qt4_wrap_ui(UI_SOURCES
  scatterplotdialog.ui
)

# Need to add current binary dir to include path for UI file headers
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(chemdata_scatter_plot_plugin ${SOURCES} ${UI_SOURCES})
set_target_properties(chemdata_scatter_plot_plugin PROPERTIES AUTOMOC TRUE)
target_link_libraries(chemdata_scatter_plot_plugin
  ChemDataCore ChemDataGui
  vtkChartsCore vtkGUISupportQt vtkRenderingQt
)
