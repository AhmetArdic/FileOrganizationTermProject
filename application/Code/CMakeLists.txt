project(Code)

set(CMAKE_CXX_STANDARD 17)
set(HASHING_LIBRARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/hashingLibs)
set(FOLDER_WATCHER_LIBRARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/folderWatcher)

add_library(md5 ${HASHING_LIBRARY_DIR}/md5.cpp ${HASHING_LIBRARY_DIR}/md5.h)
add_library(sha256 ${HASHING_LIBRARY_DIR}/sha256.cpp ${HASHING_LIBRARY_DIR}/sha256.h)
add_library(sha1 ${HASHING_LIBRARY_DIR}/sha1.cpp ${HASHING_LIBRARY_DIR}/sha1.h)

add_library(folder_watcher ${FOLDER_WATCHER_LIBRARY_DIR}/folderWatcher.cpp ${FOLDER_WATCHER_LIBRARY_DIR}/folderWatcher.h)

add_executable(Code main.cpp)

target_link_libraries(Code PRIVATE md5)
target_link_libraries(Code PRIVATE sha256)
target_link_libraries(Code PRIVATE sha1)

target_link_libraries(Code PRIVATE folder_watcher)

