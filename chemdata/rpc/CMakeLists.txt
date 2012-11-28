# find Qt
find_package(Qt4 4.7 COMPONENTS QtCore QtNetwork REQUIRED)
include(${QT_USE_FILE})

# MoleQueue is required for its RPC functions
find_package(MoleQueue REQUIRED)
include_directories(${MoleQueue_INCLUDE_DIRS})

# for jsoncpp
include_directories(SYSTEM ../thirdparty/jsoncpp/)

set(SOURCES
  rpclistener.cpp
  ../thirdparty/jsoncpp/jsoncpp.cpp
)

set(HEADERS
  rpclistener.h
)

# create chemdata-rpc library
add_chemdata_library(ChemDataRpc ${SOURCES})
set_target_properties(ChemDataRpc PROPERTIES AUTOMOC TRUE)
target_link_libraries(ChemDataRpc
  ChemDataCore
  ${QT_LIBRARIES}
  mqlocalsocketconnectionlistener
)
