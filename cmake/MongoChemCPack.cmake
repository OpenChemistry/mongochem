set(CPACK_PACKAGE_NAME "MongoChem")
set(CPACK_PACKAGE_VERSION_MAJOR ${MongoChem_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${MongoChem_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${MongoChem_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${MongoChem_VERSION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "MongoChem")
set(CPACK_PACKAGE_VENDOR "http://openchemistry.org/")
set(CPACK_PACKAGE_DESCRIPTION
  "Qt MongoDB desktop cheminformatics application.")

if(APPLE)
  configure_file("${MongoChem_SOURCE_DIR}/COPYING"
    "${MongoChem_BINARY_DIR}/COPYING.txt" @ONLY)
  set(CPACK_RESOURCE_FILE_LICENSE "${MongoChem_BINARY_DIR}/COPYING.txt")
  set(CPACK_PACKAGE_ICON
    "${MongoChem_SOURCE_DIR}/mongochem/app/icons/mongochem.icns")
  set(CPACK_BUNDLE_ICON "${CPACK_PACKAGE_ICON}")
else()
  set(CPACK_RESOURCE_FILE_LICENSE "${MongoChem_SOURCE_DIR}/COPYING")
endif()

set(CPACK_PACKAGE_EXECUTABLES "mongochem" "MongoChem")
set(CPACK_CREATE_DESKTOP_LINKS "mongochem")

configure_file("${CMAKE_CURRENT_LIST_DIR}/MongoChemCPackOptions.cmake.in"
  "${MongoChem_BINARY_DIR}/MongoChemCPackOptions.cmake" @ONLY)
set(CPACK_PROJECT_CONFIG_FILE
  "${MongoChem_BINARY_DIR}/MongoChemCPackOptions.cmake")

include(CPack)
