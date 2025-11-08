cmake_minimum_required(VERSION 3.28)
project(GeofilteredHospitalRankings LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

include(FetchContent)
FetchContent_Declare(SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG 2.6.1
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM)

# For SFML 2.6.1, we need to set some variables before making it available
set(SFML_BUILD_WINDOW TRUE)
set(SFML_BUILD_GRAPHICS TRUE)
set(SFML_BUILD_AUDIO FALSE)  # Set to TRUE if you need audio
set(SFML_BUILD_NETWORK FALSE) # Set to TRUE if you need network

FetchContent_MakeAvailable(SFML)

add_executable(GeofilteredHospitalRankings
        src/main.cpp
        src/assign_coords.cpp
        src/city_coords.cpp
        src/csv_parser.cpp
        src/geohash_geofilter.cpp
        src/scoring.cpp
        src/quadtree_geofilter.cpp
        src/find_city_coords.cpp
        src/find_closest.cpp
        src/globals.cpp
)
target_compile_features(GeofilteredHospitalRankings PRIVATE cxx_std_11)

# For SFML 2.6.1, link against the actual target names
target_link_libraries(GeofilteredHospitalRankings PRIVATE sfml-graphics sfml-window)

# Include directories for SFML 2.6.1
target_include_directories(GeofilteredHospitalRankings PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)
