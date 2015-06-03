project(LlamaGame)
cmake_minimum_required(VERSION 2.8)
aux_source_directory(. SRC_LIST)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -Wall -lSDL2 -lSDL2_gfx -std=c++11" )
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake_module")

find_package(SDL2 REQUIRED)
find_package(SDL2_TTF REQUIRED)
find_package(SDL2GFX REQUIRED)
find_package(SDL2_net REQUIRED)
add_definitions(${SDL2_DEFINITIONS} ${SDL2_TTF_DEFINITIONS} ${SDL2_NET_DEFINITIONS})
include_directories(${SDL2_INCLUDE_DIR} ${SDL2_TTF_INCLUDE_DIRS} ${SDL2_NET_INCLUDE_DIR} ${SDL2GFX_INCLUDE_DIR})
link_directories(${SDL2_LIBS} ${SDL2_TTF_LIBRARIES} ${SDL2_NET_LIBRARIES})



#add_executable( ${PROJECT_NAME} ${SRC_LIST})
add_executable( LlamaServer server.cpp)

#target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY} ${SDL2_TTF_LIBRARIES} ${SDL2GFX_LIBRARY} ${SDL2_NET_LIBRARIES})
target_link_libraries(LlamaServer ${SDL2_LIBRARY} ${SDL2_TTF_LIBRARIES} ${SDL2GFX_LIBRARY} ${SDL2_NET_LIBRARIES})
