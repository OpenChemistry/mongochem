find_package(MongoDB)
include_directories(${MongoDB_INCLUDE_DIR})
# MongoDB uses several boost components we need to link in
find_package(Boost COMPONENTS thread filesystem program_options system)
find_package(Qt4 4.7 REQUIRED QtCore QtGui QtNetwork)
find_package(VTK REQUIRED NO_MODULE)

# Chemkit is used for substructure queries
find_package(Chemkit REQUIRED)
include_directories(${CHEMKIT_INCLUDE_DIRS})

# find qt
find_package(Qt4 4.6 COMPONENTS QtCore QtGui REQUIRED)
include(${QT_USE_FILE})

set(chemdata_srcs
  chemdata.cxx
  DetailDialog.cxx
  mainwindow.cxx
  MongoModel.cxx
  MongoRecordModel.cxx
  MongoTableView.cxx
  quickquerywidget.cpp
  substructurefiltermodel.cpp
)
qt4_wrap_cpp(moc_srcs
  DetailDialog.h
  mainwindow.h
  MongoModel.h
  MongoRecordModel.h
  MongoTableView.h
  quickquerywidget.h
  substructurefiltermodel.h
)
qt4_wrap_ui(ui_srcs mainwindow.ui quickquerywidget.ui)

# Need to add current binary dir to include path for UI file headers
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${VTK_INCLUDE_DIRS})

add_executable(chemdata ${chemdata_srcs} ${moc_srcs} ${ui_srcs})
target_link_libraries(chemdata ${MongoDB_LIBRARIES} ${Boost_LIBRARIES}
  ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY}
  ${CHEMKIT_LIBRARIES} vtkCharts QVTK)
