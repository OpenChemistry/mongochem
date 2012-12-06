# find Qt
find_package(Qt4 4.7 COMPONENTS QtCore QtGui REQUIRED)
include(${QT_USE_FILE})

# VTK is used for the charting and infovis components.
find_package(VTK COMPONENTS vtkChartsCore vtkGUISupportQt vtkViewsContext2D
  vtkViewsInfovis vtkRenderingQt vtkRenderingFreeTypeOpenGL NO_MODULE)
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS ${VTK_DEFINITIONS})
include_directories(SYSTEM ${VTK_INCLUDE_DIRS})

set(SOURCES
  abstractvtkchartwidget.cpp
  abstractclusteringwidget.cpp
  abstractimportdialog.cpp
  addtagdialog.cpp
  computationalresultsmodel.cpp
  computationalresultstableview.cpp
  diagramtooltipitem.cpp
  exportmoleculehandler.cpp
  mainwindow.cpp
  moleculedetaildialog.cpp
  mongomodel.cpp
  mongotableview.cpp
  openineditorhandler.cpp
  quickquerywidget.cpp
  selectionfiltermodel.cpp
  serversettingsdialog.cpp
  substructurefiltermodel.cpp
  ../thirdparty/jsoncpp/jsoncpp.cpp
)

qt4_wrap_ui(UI_SOURCES
  addtagdialog.ui
  mainwindow.ui
  moleculedetaildialog.ui
  quickquerywidget.ui
  serversettingsdialog.ui
)

# Need to add current binary dir to include path for UI file headers
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# for jsoncpp
include_directories(SYSTEM ../thirdparty/jsoncpp/)

add_chemdata_library(ChemDataGui ${SOURCES} ${UI_SOURCES})
set_target_properties(ChemDataGui PROPERTIES AUTOMOC TRUE)
target_link_libraries(ChemDataGui
  ${MongoDB_LIBRARIES}
  ${Boost_LIBRARIES}
  ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY}
  ${CHEMKIT_LIBRARIES}
  ChemDataCore
  vtkChartsCore vtkGUISupportQt vtkViewsContext2D vtkViewsInfovis vtkRenderingQt vtkRenderingFreeTypeOpenGL
)
