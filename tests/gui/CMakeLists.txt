file(GLOB test_xml "*.xml")

# Connection parameters for the mongo server
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
endforeach()
