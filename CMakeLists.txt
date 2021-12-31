cmake_minimum_required (VERSION 2.8)
project (LocalTGEP)
aux_source_directory(./src DIR_SRCS)
SET( CMAKE_CXX_FLAGS "-std=c++11 -O3")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")

include_directories(/usr/local/include/igraph)
link_directories(/usr/local/lib)

FIND_PACKAGE( OpenMP REQUIRED)
if(OPENMP_FOUND)
    message("OPENMP FOUND")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

add_executable (LocalTGEP
        src/LocalTGEP_main/main_LocalTGEP.cpp
        src/LocalTGEP_main/graph.hpp
        src/LocalTGEP_main/graph.cpp
        src/util/util.cpp
        src/util/memory_monitor.cpp
        src/util/memory_monitor.h
        src/LocalTGEP_main/controller.cpp
        src/LocalTGEP_main/controller.h
        src/LocalTGEP_main/global_graph.cpp
        src/LocalTGEP_main/global_graph.h
        src/LocalTGEP_main/local_p.cpp
        src/LocalTGEP_main/local_p.h)
add_executable (divide_g
        src/graph_gen/DivideG.cpp
        src/graph_gen/DivideG.h
        src/util/util.cpp src/graph_gen/main_DivideG.cpp)
add_executable (graph_gen
        src/graph_gen/graph_gen.cpp
        src/util/util.cpp)
add_executable (vp2ep
        src/vp2ep/convert_vp_to_ep.cpp
        src/vp2ep/edge_parts.cpp
        src/util/util.cpp)

target_link_libraries(LocalTGEP igraph glog gflags)
target_link_libraries(divide_g igraph glog gflags)
target_link_libraries(graph_gen igraph glog gflags)
target_link_libraries(vp2ep igraph glog gflags)