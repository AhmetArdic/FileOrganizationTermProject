project(Code)

set(CMAKE_CXX_COMPILER "C:/msys64/ucrt64/bin/g++.exe")
set(CMAKE_C_COMPILER "C:/msys64/ucrt64/bin/gcc.exe")  
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

set(CMAKE_CXX_STANDARD 17)
set(INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/inc)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(my_library
    ${INC_DIR}/FolderWatcherCls.h
    ${INC_DIR}/IndexingProcessor.h
    ${INC_DIR}/md5.h
    ${INC_DIR}/sha1.h
    ${INC_DIR}/sha256.h
    ${SRC_DIR}/FolderWatcherCls.cpp
    ${SRC_DIR}/IndexingProcessor.cpp
    ${SRC_DIR}/md5.cpp
    ${SRC_DIR}/sha1.cpp
    ${SRC_DIR}/sha256.cpp
)

target_include_directories(my_library PUBLIC ${INC_DIR})
add_executable(Code main.cpp)

target_link_libraries(Code PUBLIC my_library)

