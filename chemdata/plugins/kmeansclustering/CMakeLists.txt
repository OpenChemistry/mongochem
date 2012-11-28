set(SOURCES
  kmeansclusteringdialog.cpp
  kmeansclusteringplugin.cpp
)

qt4_wrap_ui(UI_SOURCES
  kmeansclusteringdialog.ui
)

# Need to add current binary dir to include path for UI file headers
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_library(chemdata_kmeans_clustering_plugin ${SOURCES} ${UI_SOURCES})
set_target_properties(chemdata_kmeans_clustering_plugin PROPERTIES AUTOMOC TRUE)
target_link_libraries(chemdata_kmeans_clustering_plugin
  ChemDataCore ChemDataGui
  vtkChartsCore vtkGUISupportQt vtkRenderingQt vtkViewsContext2D vtkViewsInfovis
)
