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

add_subdirectory(translations)
add_subdirectory(src)
