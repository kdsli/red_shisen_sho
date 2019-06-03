set(CMAKE_CXX_STANDARD 14)

find_package(Qt5Widgets CONFIG REQUIRED)
find_package(Qt5Svg CONFIG REQUIRED)
find_package(Qt5LinguistTools CONFIG REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(OpenGL REQUIRED)

if (${Qt5OpenGL_FOUND})
    message("Found Qt5OpenGL")
    add_definitions(-DDEFINED_OPENGL)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Исходные файлы
file(GLOB_RECURSE SOURCE_FILES "${SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE HEADER_FILES "${SOURCE_DIR}/*.h")
file(GLOB_RECURSE UI_FILES "${SOURCE_DIR}/*.ui")

# Формы
qt5_wrap_ui(UIS ${UI_FILES})

# Resources
set(RESOURCE_DIR ${PROJECT_SOURCE_DIR}/resources)
file(GLOB RESOURCE_FILES "${RESOURCE_DIR}/*.qrc")

qt5_add_resources(RESOURCES ${RESOURCE_FILES})

# Translations

set(QM_DST_DIR ${EXECUTABLE_OUTPUT_PATH}/translations)

file(GLOB TS_FILES ${TS_SRC_DIR}/*.ts)
set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION ${QM_DST_DIR})
qt5_add_translation(QM_FILES ${TS_FILES})

install(DIRECTORY ${PROJECT_SOURCE_DIR}/kmahjongglib DESTINATION ${EXECUTABLE_OUTPUT_PATH})

# Чтобы не появлялось окно командной строки при запуске в Windows
IF(WIN32)
    # WIN32 и для x86, и для x86_64
    set(GUI_TYPE WIN32)
ENDIF(WIN32)

add_executable(
    ${PROJECT_NAME}
    ${GUI_TYPE}
    ${SOURCE_FILES}
    ${HEADER_FILES}
    ${UI_FILES}
    ${RESOURCES}
    ${QM_FILES}
    ${OUTPUT_FILE}
)

target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::Svg ${Qt5OpenGL_LIBRARIES})

