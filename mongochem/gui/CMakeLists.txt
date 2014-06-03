# Find the Qt components we use.
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5WebKitWidgets REQUIRED)

# VTK is used for the charting and infovis components.
find_package(VTK COMPONENTS vtkChartsCore vtkGUISupportQt vtkViewsContext2D
  vtkViewsInfovis vtkRenderingQt vtkRenderingFreeTypeOpenGL REQUIRED NO_MODULE)
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS ${VTK_DEFINITIONS})
include_directories(SYSTEM ${VTK_INCLUDE_DIRS})

find_package(AvogadroLibs REQUIRED NO_MODULE)
include_directories(${AvogadroLibs_INCLUDE_DIRS})
# If we are building statically then we need HDF5 targets.
# TODO: Should this be moved to the AvogadroLibs config file?
if(NOT BUILD_SHARED_LIBS)
  find_package(HDF5 REQUIRED COMPONENTS C)
endif()

set(SOURCES
  abstractvtkchartwidget.cpp
  abstractclusteringwidget.cpp
  abstractimportdialog.cpp
  addtagdialog.cpp
  avogadrotools.cpp
  batchjobdecorator.cpp
  batchjobmanager.cpp
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
  chemkit.cpp
  objectref.cpp
)

qt5_wrap_ui(UI_SOURCES
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
qt5_use_modules(MongoChemGui Widgets Network WebKitWidgets)
set_target_properties(MongoChemGui PROPERTIES AUTOMOC TRUE)
target_link_libraries(MongoChemGui
  ${MongoDB_LIBRARIES}
  ${Boost_LIBRARIES}
  ${CHEMKIT_LIBRARIES}
  ${EXTRA_LINK_LIB}
  vtkChartsCore vtkGUISupportQt vtkViewsContext2D vtkViewsInfovis vtkRenderingQt
  vtkRenderingFreeTypeOpenGL
  AvogadroIO AvogadroQtOpenGL AvogadroQtPlugins AvogadroRendering
  MoleQueueClient
)

if(MongoChem_ENABLE_RPC)
  target_link_libraries(MongoChemGui MoleQueueServerCore)
endif()
