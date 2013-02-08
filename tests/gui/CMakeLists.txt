include_directories("${MongoChem_BINARY_DIR}/tests"
  "${MongoChem_SOURCE_DIR}/mongochem/gui"
  "${MongoChem_BINARY_DIR}/mongochem/gui")

find_package(MongoDB)
include_directories(SYSTEM ${MongoDB_INCLUDE_DIR})

find_package(Qt4 4.8.0 COMPONENTS QtTest)
include(${QT_USE_FILE})

find_package(Chemkit COMPONENTS io md REQUIRED)
include_directories(SYSTEM ${CHEMKIT_INCLUDE_DIRS})
link_directories(${CHEMKIT_LIBRARY_DIR})

set(tests
  cjsonexporter
  )

foreach(test ${tests})
  add_executable(${test}test MACOSX_BUNDLE ${test}test.cpp)
  set_target_properties(${test}test PROPERTIES AUTOMOC TRUE)
  target_link_libraries(${test}test
    ${QT_LIBRARIES}
    MongoChemGui
    )
  add_test(NAME mongochem-${test} COMMAND ${test}test)
endforeach()
