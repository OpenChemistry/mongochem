find_package(MongoDB)
include_directories(SYSTEM ${MongoDB_INCLUDE_DIR})
# MongoDB built with unicode strings
add_definitions("-D_UNICODE")
if(WIN32)
  if(MSVC)
    add_definitions("-D_CRT_SECURE_NO_WARNINGS")
  endif()
  set(Boost_USE_STATIC_LIBS ON)
endif()
# MongoDB uses several boost components we need to link in
find_package(Boost COMPONENTS thread filesystem program_options system iostreams)
if(WIN32)
  add_definitions("-DBOOST_ALL_NO_LIB")
endif()
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
# MongoDB also seems to require pthread on Linux (but not Apple)
if(UNIX AND NOT APPLE)
  find_package(Threads)
  set(EXTRA_LINK_LIB ${CMAKE_THREAD_LIBS_INIT})
endif()

# VTK is used for the charting and infovis components.
find_package(VTK COMPONENTS vtkChartsCore vtkGUISupportQt vtkViewsContext2D
  vtkViewsInfovis vtkRenderingQt vtkRenderingFreeTypeOpenGL NO_MODULE)
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS ${VTK_DEFINITIONS})
include_directories(SYSTEM ${VTK_INCLUDE_DIRS})

# Chemkit is used for substructure queries
find_package(Chemkit COMPONENTS io md REQUIRED)
include_directories(SYSTEM ${CHEMKIT_INCLUDE_DIRS})
link_directories(${CHEMKIT_LIBRARY_DIR})

# Find the Qt components we need.
find_package(Qt4 4.7 COMPONENTS QtCore QtGui QtNetwork REQUIRED)
include_directories(SYSTEM ${QT_INCLUDES})
add_definitions(${QT_DEFINITIONS})

# MoleQueue is required for its RPC functions
find_package(MoleQueue REQUIRED)
include_directories(${MoleQueue_INCLUDE_DIRS})

set(chemdata_srcs
  addtagdialog.cpp
  chemdata.cpp
  computationalresultsmodel.cpp
  computationalresultstableview.cpp
  diagramtooltipitem.cpp
  exportmoleculehandler.cpp
  fingerprintsimilaritydialog.cpp
  graphdialog.cpp
  histogramdialog.cpp
  kmeansclusteringdialog.cpp
  mainwindow.cpp
  moleculedetaildialog.cpp
  mongodatabase.cpp
  mongomodel.cpp
  mongorecordmodel.cpp
  mongotableview.cpp
  openineditorhandler.cpp
  parallelcoordinatesdialog.cpp
  plotmatrixdialog.cpp
  quickquerywidget.cpp
  rpclistener.cpp
  serversettingsdialog.cpp
  similaritygraphwidget.cpp
  structuresimilaritydialog.cpp
  substructurefiltermodel.cpp
  ../thirdparty/jsoncpp/jsoncpp.cpp
)

qt4_wrap_ui(ui_srcs
  addtagdialog.ui
  fingerprintsimilaritydialog.ui
  graphdialog.ui
  histogramdialog.ui
  kmeansclusteringdialog.ui
  mainwindow.ui
  moleculedetaildialog.ui
  parallelcoordinatesdialog.ui
  plotmatrixdialog.ui
  quickquerywidget.ui
  serversettingsdialog.ui
  structuresimilaritydialog.ui
)

# Need to add current binary dir to include path for UI file headers
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# for jsoncpp
include_directories(../thirdparty/jsoncpp/)

add_executable(chemdata MACOSX_BUNDLE ${chemdata_srcs} ${ui_srcs})
set_target_properties(chemdata PROPERTIES AUTOMOC TRUE)
target_link_libraries(chemdata ${MongoDB_LIBRARIES} ${Boost_LIBRARIES}
  ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY}
  ${CHEMKIT_LIBRARIES} ${EXTRA_LINK_LIB} vtkChartsCore vtkGUISupportQt
  vtkViewsContext2D vtkViewsInfovis vtkRenderingQt vtkRenderingFreeTypeOpenGL
  mqlocalsocketconnectionlistener)
install(TARGETS chemdata DESTINATION "${INSTALL_RUNTIME_DIR}")

if((APPLE OR WIN32) AND NOT ${CMAKE_VERSION} VERSION_LESS 2.8.8)
  set(sfx "")
  if(APPLE)
    set(sfx ".app")
  elseif(WIN32)
    set(sfx ".exe")
  endif()

  set(dirs "")
  if(CMAKE_PREFIX_PATH)
    set(dirs "${CMAKE_PREFIX_PATH}/bin" "${CMAKE_PREFIX_PATH}/lib")
  endif()

  # The VTK dlls are not in the CMAKE_PREFIX_PATH when built against
  # a "build tree" of VTK:
  get_property(loc TARGET vtkChartsCore PROPERTY LOCATION_RELEASE)
  get_filename_component(vtk_dlls_dir "${loc}" PATH)
  set(dirs ${dirs} "${vtk_dlls_dir}")

  include(DeployQt4)
  install_qt4_executable(bin/chemdata${sfx} "" "" "${dirs}")
endif()
