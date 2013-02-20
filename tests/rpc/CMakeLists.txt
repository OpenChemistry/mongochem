find_package(PythonInterp 2.6 REQUIRED)

set(tests
  get-chemical-json
  resolve-name-to-inchi
)

foreach(test ${tests})
  add_test(NAME ${test}
    COMMAND ${PYTHON_EXECUTABLE}
      ${CMAKE_CURRENT_SOURCE_DIR}/${test}.py
      $<TARGET_FILE:mongochem>)
  set_property(TEST ${test} PROPERTY RUN_SERIAL 1)
endforeach()
