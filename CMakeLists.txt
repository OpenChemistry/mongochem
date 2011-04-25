cmake_minimum_required(VERSION 2.8)

project(ChemData)

set(CMAKE_MODULE_PATH ${ChemData_SOURCE_DIR}/cmake)

find_package(MongoDB)
include_directories(${MongoDB_INCLUDE_DIRS})
# MongoDB uses several boost components we need to link in
find_package(Boost COMPONENTS thread filesystem program_options)
find_package(Qt4)

add_executable(chemdata main.cxx)
target_link_libraries(chemdata ${MongoDB_LIBRARIES} ${Boost_LIBRARIES}
  ${QT_QTCORE_LIBRARY})
