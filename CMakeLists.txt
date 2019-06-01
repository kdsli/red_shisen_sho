cmake_minimum_required (VERSION 3.14.0)

project (red_shisen_sho)

add_definitions(-Wall)

# Языки трансляции
set(LANGUAGES ru en)

IF(NOT CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE Release)
   message("-- Set build type to " ${CMAKE_BUILD_TYPE})
ENDIF()

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Директории проекта
set(SOURCE_DIR ${ROOT_DIR}/src)
set(EXECUTABLE_OUTPUT_PATH ${ROOT_DIR}/output_${CMAKE_BUILD_TYPE})
set(TS_SRC_DIR ${ROOT_DIR}/translations)

unset(QT_QMAKE_EXECUTABLE)

# Version
set(VERSION_INC version.inc)
set(VERSION_FILE ${SOURCE_DIR}/${VERSION_INC})
include(cmake/get_version.cmake)

getVersion(PROG_VERSION PROG_DATE)
message("Programm version = ${PROG_VERSION} ${PROG_DATE}")

if(EXISTS ${VERSION_FILE})
    file(READ ${VERSION_FILE} str)
    string(REGEX MATCH "[0-9]+\.[0-9]+\.[0-9]+(-[0-9+]+)?" VERSION_IN_FILE ${str})
    if (${CMAKE_MATCH_0})
        set(VERSION_IN_FILE "0.0.0-0")
    endif()
    message("File version = [${VERSION_IN_FILE}]")
    if(${VERSION_IN_FILE})
        message("Version values are not readed from file src/${VERSION_INC}")
        write_version_file(${VERSION_FILE} ${PROG_VERSION} ${PROG_DATE})
    else()
        if(NOT ${VERSION_IN_FILE} STREQUAL ${PROG_VERSION})
            write_version_file(${VERSION_FILE} ${PROG_VERSION} ${PROG_DATE})
        endif()
    endif()
else()
    write_version_file(${VERSION_FILE} ${PROG_VERSION} ${PROG_DATE})
endif()

add_subdirectory(translations)
add_subdirectory(src)
