add_subdirectory(gui)

if(MongoChem_ENABLE_RPC)
  add_subdirectory(rpc)
endif()
