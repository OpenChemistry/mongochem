set(CPACK_PACKAGE_NAME "MongoChem")
set(CPACK_PACKAGE_VERSION_MAJOR ${MongoChem_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${MongoChem_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${MongoChem_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${MongoChem_VERSION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "MongoChem")

if(APPLE)
  configure_file("${MongoChem_SOURCE_DIR}/COPYING"
    "${MongoChem_BINARY_DIR}/COPYING.txt" @ONLY)
  set(CPACK_RESOURCE_FILE_LICENSE "${MongoChem_BINARY_DIR}/COPYING.txt")
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
