# Find Qt, including QtMain if on Windows.
if(WIN32)
  set(qtmain Main)
  find_package(Qt5Main REQUIRED)
else()
  unset(qtmain)
endif()
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5WebKitWidgets REQUIRED)

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

# find QtTesting
if(ENABLE_TESTING)
  find_package(QtTesting REQUIRED)
  include_directories(${QtTesting_INCLUDE_DIRS})
  link_directories(${QtTesting_LIBRARY_DIR})
  add_definitions(-DQTTESTING)
endif()

configure_file(mongochemconfig.h.in mongochemconfig.h)

set(SOURCES
  main.cpp
  mainwindow.cpp
  aboutdialog.cpp
)

qt5_add_resources(RCC_SOURCES mongochem.qrc)
qt5_wrap_ui(UI_SOURCES
  mainwindow.ui
  aboutdialog.ui
)

# Handle Mac OS X specific icons etc.
if(APPLE)
  list(APPEND SOURCES icons/mongochem.icns)
  set(MACOSX_BUNDLE_ICON_FILE mongochem.icns)
  set(MACOSX_BUNDLE_BUNDLE_VERSION "${MongoChem_VERSION}")
  set_source_files_properties(icons/mongochem.icns PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources)
elseif(WIN32)
  list(APPEND SOURCES icons/mongochem.rc)
endif()

add_executable(mongochem WIN32 MACOSX_BUNDLE
  ${SOURCES} ${UI_SOURCES} ${RCC_SOURCES})
set_target_properties(mongochem PROPERTIES AUTOMOC TRUE)
qt5_use_modules(mongochem Widgets Network WebKitWidgets)
target_link_libraries(mongochem MongoChemGui MongoChemPlugins)
if(APPLE)
  set_target_properties(mongochem PROPERTIES
    OUTPUT_NAME ${MACOSX_BUNDLE_NAME})
endif()
if(ENABLE_TESTING)
  target_link_libraries(mongochem QtTesting)
endif()
install(TARGETS mongochem
  RUNTIME DESTINATION "${INSTALL_RUNTIME_DIR}"
  BUNDLE DESTINATION .)

if((APPLE OR WIN32) AND NOT ${CMAKE_VERSION} VERSION_LESS 2.8.8)
  set(pfx "")
  if(NOT APPLE)
    set(pfx "bin/")
  endif()
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
  # Same for MoleQueue - on Mac these might be in the .app dir
  get_property(loc TARGET MoleQueueClient PROPERTY LOCATION_RELEASE)
  get_filename_component(molequeue_dir "${loc}" PATH)
  list(APPEND dirs "${molequeue_dir}")

  include(DeployQt4)

  get_target_property(output_name mongochem OUTPUT_NAME)
  if(output_name)
    set(exe "${pfx}${output_name}${sfx}")
  else()
    set(exe "${pfx}mongochem${sfx}")
  endif()

  install_qt4_executable(${exe} "" "" "${dirs}")
endif()
