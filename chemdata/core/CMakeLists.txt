# find Qt
find_package(Qt4 4.7 COMPONENTS QtCore REQUIRED)
include(${QT_USE_FILE})

set(SOURCES
  mongodatabase.cpp
)

set(HEADERS
  moleculeref.h
  mongodatabase.h
  objectref.h
)

add_chemdata_library(ChemDataCore ${SOURCES})
target_link_libraries(ChemDataCore
  ${MongoDB_LIBRARIES}
  ${Boost_LIBRARIES}
  ${QT_LIBRARIES}
  ${CHEMKIT_LIBRARIES}
  ${EXTRA_LINK_LIB}
)
