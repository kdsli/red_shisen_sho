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
set(VERSION_FILE ${SOURCE_DIR}/version.h)
include(cmake/get_version.cmake)

getVersion(PROG_VERSION PROG_DATE)
message("Version = ${PROG_VERSION} ${PROG_DATE}")

file(READ ${VERSION_FILE} str)
string(REGEX MATCH "[0-9]*.[0-9]*.[0-9]*-[0-9]*" file_version ${str})
if(${FILE_VERSION} NOT EQUAL ${PROG_VERSION})
    file(WRITE ${VERSION_FILE}
        "// Do not delete this file, created automatically.\n"
        "static const QString = \"${PROG_VERSION} ${PROG_DATE}\";"
        )
    message("Created new version file.")
endif()

add_subdirectory(translations)
add_subdirectory(src)
