set(MongoChem_TESTDATA_DIR "${MongoChem_SOURCE_DIR}/tests/data")
configure_file(mongochemtestconfig.h.in mongochemtestconfig.h)

# add gui tests
add_subdirectory(gui)

# add rpc tests
if(MongoChem_ENABLE_RPC)
  add_subdirectory(rpc)
endif()
