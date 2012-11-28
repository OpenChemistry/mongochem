# find Qt
find_package(Qt4 4.7 COMPONENTS QtCore QtGui QtNetwork REQUIRED)
include(${QT_USE_FILE})

# VTK is used for the charting and infovis components.
find_package(VTK COMPONENTS vtkChartsCore vtkGUISupportQt vtkViewsContext2D
  vtkViewsInfovis vtkRenderingQt vtkRenderingFreeTypeOpenGL NO_MODULE)
set_property(DIRECTORY APPEND PROPERTY COMPILE_DEFINITIONS ${VTK_DEFINITIONS})
include_directories(SYSTEM ${VTK_INCLUDE_DIRS})

# for jsoncpp
include_directories(SYSTEM ../thirdparty/jsoncpp/)

set(SOURCES
  main.cpp
)

add_executable(chemdata MACOSX_BUNDLE ${SOURCES})
target_link_libraries(chemdata
  ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY}
  ChemDataCore ChemDataGui ChemDataRpc
)
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
