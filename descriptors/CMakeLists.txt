# We use Open Babel to read in a data file, and output several representations
# that can be used in the database.
find_package(OpenBabel2 REQUIRED NO_MODULE)

include_directories(${OpenBabel2_INCLUDE_DIRS})

add_executable(descriptors descriptors.cpp)
target_link_libraries(descriptors openbabel)
