# find Qt
find_package(Qt4 4.7 COMPONENTS QtCore QtGui QtNetwork REQUIRED)
include(${QT_USE_FILE})

# VTK is used for the charting and infovis components.
find_package(VTK COMPONENTS vtkChartsCore vtkGUISupportQt vtkViewsContext2D
  vtkViewsInfovis vtkRenderingQt vtkRenderingFreeTypeOpenGL NO_MODULE)
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS ${VTK_DEFINITIONS})
include_directories(SYSTEM ${VTK_INCLUDE_DIRS}
  ${MongoChem_SOURCE_DIR}/mongochem/gui)

# Need to add current binary dir to include path for UI file headers
include_directories(${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR})

if(MongoChem_ENABLE_RPC)
  # For Qt5Json
  find_package(MoleQueue REQUIRED)
  include_directories(${MoleQueue_INCLUDE_DIRS})
  add_definitions(-DMongoChem_ENABLE_RPC)
endif()

set(SOURCES
  main.cpp
  mainwindow.cpp
)

qt4_wrap_ui(UI_SOURCES mainwindow.ui)

add_executable(mongochem MACOSX_BUNDLE ${SOURCES} ${UI_SOURCES})
set_target_properties(mongochem PROPERTIES AUTOMOC TRUE)
target_link_libraries(mongochem
  ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY}
  MongoChemGui MongoChemPlugins
)
install(TARGETS mongochem DESTINATION "${INSTALL_RUNTIME_DIR}")

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
  install_qt4_executable(bin/mongochem${sfx} "" "" "${dirs}")
endif()
