cmake_minimum_required(VERSION 3.27)
project(FileOrgTermProject)

##############################
### Global Defines
##############################
set(EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/external)
set(GIT_REPO_DIR_NAME SecLists)
set(PROJECT_DIR application)

set(UNPROCESSED_PASSWORDS_DIR ${CMAKE_SOURCE_DIR}/${PROJECT_DIR}/Unprocessed-Passwords)
set(PROCESSED_PASSWORDS_DIR ${CMAKE_SOURCE_DIR}/${PROJECT_DIR}/Processed)
set(CODE_DIR ${CMAKE_SOURCE_DIR}/${PROJECT_DIR}/Code)
set(INDEX_DIR ${CMAKE_SOURCE_DIR}/${PROJECT_DIR}/Index)

##############################
### 1 GitHub’dan Veri Indirme
##############################

##############################
## 1.1 SecLists repo'sunu cekme
##############################
include(cmake/GitUtils.cmake)

file(MAKE_DIRECTORY ${EXTERNAL_DIR})
git_clone(
        PROJECT_NAME    ${GIT_REPO_DIR_NAME}
        GIT_URL         https://github.com/danielmiessler/SecLists.git
        GIT_BRANCH      master
		DIRECTORY       ${EXTERNAL_DIR})


##############################
### 2 Proje Klasor Yapisini Olusturma
##############################
file(MAKE_DIRECTORY ${PROJECT_DIR})
file(MAKE_DIRECTORY ${UNPROCESSED_PASSWORDS_DIR})
file(MAKE_DIRECTORY ${PROCESSED_PASSWORDS_DIR})
file(MAKE_DIRECTORY ${CODE_DIR})
file(MAKE_DIRECTORY ${INDEX_DIR})

##############################
## 2.2 "Unprocessed-Passwords" dizininde eksik olan dosyalarin "external/SecLists/Passwords/Common-Credentials" dizininden kopyalanmasi
##############################
file(GLOB SOURCE_FILES "${EXTERNAL_DIR}/SecLists/Passwords/Common-Credentials/*")	# Kaynak dizindeki dosyalarin listesi
file(GLOB DESTINATION_FILES "${UNPROCESSED_PASSWORDS_DIR}/*")						# Hedef dizindeki dosyalarin listesi
foreach(SOURCE_FILE ${SOURCE_FILES})
    get_filename_component(FILE_NAME ${SOURCE_FILE} NAME)
    list(FIND DESTINATION_FILES "${UNPROCESSED_PASSWORDS_DIR}/${FILE_NAME}" INDEX)

    if(INDEX EQUAL -1)
        message(STATUS "Kopyalanacak dosya: ${FILE_NAME}")
        file(COPY ${SOURCE_FILE} DESTINATION ${UNPROCESSED_PASSWORDS_DIR})
    endif()
endforeach()

##############################
## 2.3 Indirilen SecList repo'sunu silme
##############################
#file(REMOVE_RECURSE ${EXTERNAL_DIR}/${GIT_REPO_DIR_NAME})

##############################
### 3 Derleme 
##############################
add_subdirectory(${PROJECT_DIR}/Code)