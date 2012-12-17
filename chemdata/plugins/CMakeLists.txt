# find Qt
find_package(Qt4 4.7 COMPONENTS QtCore QtGui REQUIRED)
include(${QT_USE_FILE})

# VTK is used for the charting and infovis components.
find_package(VTK COMPONENTS vtkChartsCore vtkGUISupportQt vtkViewsContext2D
  vtkViewsInfovis vtkRenderingQt vtkRenderingFreeTypeOpenGL NO_MODULE)
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS ${VTK_DEFINITIONS})
include_directories(SYSTEM ${VTK_INCLUDE_DIRS})

add_subdirectory(csvimporter)
add_subdirectory(histogram)
add_subdirectory(kmeansclustering)
add_subdirectory(parallelcoordinates)
add_subdirectory(plotmatrix)
add_subdirectory(scatterplot)
add_subdirectory(similaritygraph)
