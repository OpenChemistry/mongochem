# add gui tests
add_subdirectory(gui)

# add rpc tests
if(MongoChem_ENABLE_RPC)
  add_subdirectory(rpc)
endif()
