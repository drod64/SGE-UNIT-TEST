cmake_minimum_required(VERSION 3.20)

project(TEST)

set(SOURCE_FILES
    src/pector_tester.cpp
    src/SparsePageTable_tester.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)