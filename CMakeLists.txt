cmake_minimum_required(VERSION 3.22)

project(cppLBM CXX)
set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE LIBRARY_SOURCES "src/*.cpp")
list(REMOVE_ITEM LIBRARY_SOURCES "src/main.cpp")

add_library (
    lbmcore 
    STATIC
        ${LIBRARY_SOURCES}
)

target_include_directories(
    lbmcore
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)

# target_link_libraries(
#     lbmcore
#     PUBLIC
#         <ADD_EXTERN_LIBRARIES>
# )

set_target_properties( 
    lbmcore
    PROPERTIES
        ENABLE_EXPORTS ON
        POSITION_INDEPENDENT_CODE ON
)

install (
    TARGETS lbmcore
    DESTINATION ${CMAKE_BINARY_DIR}/lib
)

add_executable(
    lbm
        "src/main.cpp"
)



target_include_directories(
    lbm
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include

)

target_link_libraries(
    lbm
    PRIVATE
        lbmcore
)

if (CMAKE_BUILD_TYPE STREQUAL "DEBUG" OR enable_tests)
	add_subdirectory(tests/unit-tests)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Ofast -ffast-math -fno-trapping-math -march=native -mtune=native")
endif()

# set(CMAKE_VERBOSE_MAKEFILE ON)S