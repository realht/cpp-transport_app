cmake_minimum_required(VERSION 3.11)

project(transport_catalog CXX)
set(CMAKE_CXX_STANDARD 17)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_library(DataLib STATIC 
    data/domain.h
    data/domain.cpp
    data/geo.h
    data/geo.cpp
    data/ranges.h
    data/transport_catalogue.h
    data/transport_catalogue.cpp
)

add_library(ImgLib STATIC 
    img/map_renderer.h
    img/map_renderer.cpp
    img/svg.h
    img/svg.cpp
)

add_library(JsonLib STATIC 
    json/json.h
    json/json.cpp
    json/json_builder.h
    json/json_builder.cpp
    json/json_reader.h
    json/json_reader.cpp
)

add_library(RouteLib STATIC 
    route/graph.h
    route/router.h
    route/transport_router.h
    route/transport_router.cpp
)

add_executable(transport_catalog
    main/main.cpp
    main/request_handler.h
    main/request_handler.cpp
)


target_link_libraries(transport_catalog DataLib ImgLib JsonLib RouteLib)