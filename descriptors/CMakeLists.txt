# We use Open Babel to read in a data file, and output several representations
# that can be used in the database.
find_package(OpenBabel2 REQUIRED NO_MODULE)

include_directories(${OpenBabel2_INCLUDE_DIRS})

add_executable(descriptors descriptors.cpp)
target_link_libraries(descriptors openbabel)

# setup pubchem-import program
find_package(MongoDB)
include_directories(${MongoDB_INCLUDE_DIR})
find_package(Boost COMPONENTS thread filesystem program_options system iostreams)

#find_package(Chemkit COMPONENTS io)
#include_directories(${CHEMKIT_INCLUDE_DIRS})

#add_executable(pubchem-import pubchem_import.cpp)
#target_link_libraries(pubchem-import ${MongoDB_LIBRARIES} ${Boost_LIBRARIES} ${CHEMKIT_LIBRARIES})

# setup cactus-import program
find_package(Qt4 COMPONENTS QtCore QtNetwork REQUIRED)
include(${QT_USE_FILE})

add_executable(cactus-import cactus_import.cpp)
target_link_libraries(cactus-import ${MongoDB_LIBRARIES} ${Boost_LIBRARIES} ${QT_LIBRARIES})
