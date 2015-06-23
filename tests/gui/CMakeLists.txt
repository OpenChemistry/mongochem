file(GLOB test_xml "*.xml")

# connection parameters for the mongo server
set(TEST_SERVER "mongochem.kitwarein.com")
set(TEST_PORT "27017")
set(TEST_COLLECTION "chem")

foreach(xml ${test_xml})
  get_filename_component(name "${xml}" NAME_WE)
  message("Adding test: ${name} from XML")
  add_test(NAME mongochem-${name}
    COMMAND mongochem --test-file "${xml}"
                      --server "${TEST_SERVER}"
                      --port "${TEST_PORT}"
                      --collection "${TEST_COLLECTION}")
  set_tests_properties(
    mongochem-${name}
    PROPERTIES FAIL_REGULAR_EXPRESSION "Couldn't find object"
  )
endforeach()

# Non QtTesting tests
include_directories("${MongoChem_BINARY_DIR}/tests"
  "${MongoChem_BINARY_DIR}/tests/gui"
  "${MongoChem_SOURCE_DIR}/mongochem/gui"
  "${MongoChem_BINARY_DIR}/mongochem/gui")

find_package(MongoDB)
include_directories(SYSTEM ${MongoDB_INCLUDE_DIR})

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5WebKitWidgets REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(Qt5Test REQUIRED)

find_package(Chemkit COMPONENTS io md REQUIRED)
include_directories(SYSTEM ${CHEMKIT_INCLUDE_DIRS})
link_directories(${CHEMKIT_LIBRARY_DIR})

set(tests
  cjsonexporter
  )

foreach(test ${tests})
  add_executable(${test}test MACOSX_BUNDLE ${test}test.cpp)
  set_target_properties(${test}test PROPERTIES AUTOMOC TRUE)
  qt5_use_modules(${test}test Widgets Network WebKitWidgets OpenGL Test)
  target_link_libraries(${test}test MongoChemGui)
  add_test(NAME mongochem-${test} COMMAND ${test}test)
endforeach()
