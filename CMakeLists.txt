cmake_minimum_required(VERSION 2.8)

project(ChemData)

set(CMAKE_MODULE_PATH ${ChemData_SOURCE_DIR}/cmake)

find_package(MongoDB)
include_directories(${MongoDB_INCLUDE_DIR})
# MongoDB uses several boost components we need to link in
find_package(Boost COMPONENTS thread filesystem program_options system)
find_package(Qt4 4.7 REQUIRED QtCore QtGui)
find_package(VTK REQUIRED NO_MODULE)

set(chemdata_srcs main.cxx mainwindow.cxx mongomodel.cxx)

qt4_wrap_cpp(moc_srcs mainwindow.h mongomodel.h)
qt4_wrap_ui(ui_srcs mainwindow.ui)

# Need to add current binary dir to include path for UI file headers
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${VTK_INCLUDE_DIRS})

add_executable(chemdata ${chemdata_srcs} ${moc_srcs} ${ui_srcs})
target_link_libraries(chemdata ${MongoDB_LIBRARIES} ${Boost_LIBRARIES}
  ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} vtkCharts QVTK)
