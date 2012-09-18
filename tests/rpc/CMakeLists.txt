find_package(PythonInterp REQUIRED)

set(tests
  get_chemical_json
  resolve_name_to_inchi
)

foreach(test ${tests})
  add_test(NAME ${test}
           COMMAND ${PYTHON_EXECUTABLE}
                   ${CMAKE_CURRENT_SOURCE_DIR}/${test}.py)
endforeach()
