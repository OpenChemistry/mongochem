# find Qt
find_package(Qt4 4.7 COMPONENTS QtCore QtGui QtNetwork QtWebKit REQUIRED)
include(${QT_USE_FILE})

# VTK is used for the charting and infovis components.
find_package(VTK COMPONENTS vtkChartsCore vtkGUISupportQt vtkViewsContext2D
  vtkViewsInfovis vtkRenderingQt vtkRenderingFreeTypeOpenGL NO_MODULE)
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS ${VTK_DEFINITIONS})
include_directories(SYSTEM ${VTK_INCLUDE_DIRS})

find_package(AvogadroLibs)
include_directories(${AvogadroLibs_INCLUDE_DIRS})

set(SOURCES
  abstractvtkchartwidget.cpp
  abstractclusteringwidget.cpp
  abstractimportdialog.cpp
  addtagdialog.cpp
  computationalresultsmodel.cpp
  computationalresultstableview.cpp
  diagramtooltipitem.cpp
  exportmoleculehandler.cpp
  moleculedetaildialog.cpp
  mongodatabase.cpp
  mongomodel.cpp
  mongotableview.cpp
  openineditorhandler.cpp
  queryprogressdialog.cpp
  quickquerywidget.cpp
  selectionfiltermodel.cpp
  serversettingsdialog.cpp
  substructurefiltermodel.cpp
  svggenerator.cpp
  cjsonexporter.cpp
)

qt4_wrap_ui(UI_SOURCES
  addtagdialog.ui
  moleculedetaildialog.ui
  queryprogressdialog.ui
  quickquerywidget.ui
  serversettingsdialog.ui
)

if(MongoChem_ENABLE_RPC)
  list(APPEND SOURCES rpclistener.cpp)
  # MoleQueue is required for its RPC functions.
  find_package(MoleQueue REQUIRED)
  include_directories(${MoleQueue_INCLUDE_DIRS})
endif()

# Need to add current binary dir to include path for UI file headers
include_directories(${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR})

mongochem_add_library(MongoChemGui ${SOURCES} ${UI_SOURCES})
set_target_properties(MongoChemGui PROPERTIES AUTOMOC TRUE)
target_link_libraries(MongoChemGui
  ${MongoDB_LIBRARIES}
  ${Boost_LIBRARIES}
  ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTWEBKIT_LIBRARY}
  ${CHEMKIT_LIBRARIES}
  ${EXTRA_LINK_LIB}
  vtkChartsCore vtkGUISupportQt vtkViewsContext2D vtkViewsInfovis vtkRenderingQt
  vtkRenderingFreeTypeOpenGL
  AvogadroIO AvogadroQtOpenGL AvogadroQtPlugins AvogadroRendering
)

if(MongoChem_ENABLE_RPC)
  target_link_libraries(MongoChemGui mqlocalsocketconnectionlistener)
endif()
