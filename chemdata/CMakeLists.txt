find_package(MongoDB)
include_directories(${MongoDB_INCLUDE_DIR})
# MongoDB uses several boost components we need to link in
find_package(Boost COMPONENTS thread filesystem program_options system iostreams)
# MongoDB also seems to require pthread on Linux (but not Apple)
if(UNIX AND NOT APPLE)
  find_package(Threads)
  set(EXTRA_LINK_LIB ${CMAKE_THREAD_LIBS_INIT})
endif()
find_package(VTK REQUIRED NO_MODULE)

# Chemkit is used for substructure queries
find_package(Chemkit COMPONENTS io md REQUIRED)
include_directories(${CHEMKIT_INCLUDE_DIRS})
link_directories(${CHEMKIT_LIBRARY_DIR})

# find qt
find_package(Qt4 4.6 COMPONENTS QtCore QtGui QtNetwork REQUIRED)
include(${QT_USE_FILE})

set(chemdata_srcs
  chemdata.cpp
  diagramtooltipitem.cpp
  exportmoleculehandler.cpp
  fingerprintsimilaritydialog.cpp
  graphdialog.cpp
  histogramdialog.cpp
  kmeansclusteringdialog.cpp
  mainwindow.cpp
  moleculedetaildialog.cpp
  mongomodel.cpp
  mongorecordmodel.cpp
  mongotableview.cpp
  openineditorhandler.cpp
  parallelcoordinatesdialog.cpp
  plotmatrixdialog.cpp
  quickquerywidget.cpp
  serversettingsdialog.cpp
  similaritygraphwidget.cpp
  structuresimilaritydialog.cpp
  substructurefiltermodel.cpp
)
qt4_wrap_cpp(moc_srcs
  exportmoleculehandler.h
  fingerprintsimilaritydialog.h
  graphdialog.h
  histogramdialog.h
  kmeansclusteringdialog.h
  mainwindow.h
  moleculedetaildialog.h
  mongomodel.h
  mongorecordmodel.h
  mongotableview.h
  openineditorhandler.h
  parallelcoordinatesdialog.h
  plotmatrixdialog.h
  quickquerywidget.h
  serversettingsdialog.h
  similaritygraphwidget.h
  structuresimilaritydialog.h
  substructurefiltermodel.h
)
qt4_wrap_ui(ui_srcs
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
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${VTK_INCLUDE_DIRS})

add_executable(chemdata ${chemdata_srcs} ${moc_srcs} ${ui_srcs})
target_link_libraries(chemdata ${MongoDB_LIBRARIES} ${Boost_LIBRARIES}
  ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY}
  ${CHEMKIT_LIBRARIES} ${EXTRA_LINK_LIB} vtkCharts QVTK)
install(TARGETS chemdata DESTINATION bin)
