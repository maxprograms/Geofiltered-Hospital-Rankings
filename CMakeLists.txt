cmake_minimum_required(VERSION 3.28)
project(GeofilteredHospitalRankings LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include(FetchContent)

set(SFML_BUILD_WINDOW TRUE)
set(SFML_BUILD_GRAPHICS TRUE)
set(SFML_BUILD_AUDIO FALSE)
set(SFML_BUILD_NETWORK FALSE)

FetchContent_Declare(
        SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 2.6.1
        GIT_SHALLOW ON
)
FetchContent_MakeAvailable(SFML)

add_executable(GeofilteredHospitalRankings
        src/main.cpp
        src/assign_coords.cpp
        src/city_coords.cpp
        src/csv_parser.cpp
        src/geofilter.cpp
)

target_link_libraries(GeofilteredHospitalRankings PRIVATE sfml-graphics sfml-window sfml-system)
target_include_directories(GeofilteredHospitalRankings PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)