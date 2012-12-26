find_package(PythonInterp 2.6 REQUIRED)

set(tests
  get_chemical_json
  resolve_name_to_inchi
)

foreach(test ${tests})
  add_test(NAME ${test}
    COMMAND ${PYTHON_EXECUTABLE}
      ${CMAKE_CURRENT_SOURCE_DIR}/${test}.py
      $<TARGET_FILE:mongochem>)
  set_property(TEST ${test} PROPERTY RUN_SERIAL 1)
endforeach()
