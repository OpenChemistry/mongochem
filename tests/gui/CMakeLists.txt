file(GLOB test_xml "*.xml")

foreach(xml ${test_xml})
  get_filename_component(name "${xml}" NAME_WE)
  message("Adding test: ${name} from XML")
  add_test(NAME mongochem-${name}
    COMMAND mongochem --test-file "${xml}")
endforeach()
