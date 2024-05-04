project(Code)

set(CMAKE_CXX_STANDARD 17)
set(LIBRARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/hashingLibs)

add_library(md5 ${LIBRARY_DIR}/md5.cpp ${LIBRARY_DIR}/md5.h)
add_library(sha256 ${LIBRARY_DIR}/sha256.cpp ${LIBRARY_DIR}/sha256.h)
add_library(sha1 ${LIBRARY_DIR}/sha1.cpp ${LIBRARY_DIR}/sha1.h)

add_executable(Code main.cpp)
target_link_libraries(Code PRIVATE md5)
target_link_libraries(Code PRIVATE sha256)
target_link_libraries(Code PRIVATE sha1)
