set(CMAKE_CXX_STANDARD 14)

find_package(Qt6Widgets CONFIG REQUIRED)
find_package(Qt6Svg CONFIG REQUIRED)
find_package(Qt6SvgWidgets CONFIG REQUIRED)
find_package(Qt6LinguistTools CONFIG REQUIRED)
find_package(Qt6StateMachine CONFIG REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Исходные файлы
file(GLOB_RECURSE SOURCE_FILES "${SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE HEADER_FILES "${SOURCE_DIR}/*.h")
file(GLOB_RECURSE UI_FILES "${SOURCE_DIR}/*.ui")

# Формы
qt6_wrap_ui(UIS ${UI_FILES})

# Resources
set(RESOURCE_DIR ${PROJECT_SOURCE_DIR}/resources)
file(GLOB RESOURCE_FILES "${RESOURCE_DIR}/*.qrc")

qt6_add_resources(RESOURCES ${RESOURCE_FILES})

# Translations

set(QM_DST_DIR ${EXECUTABLE_OUTPUT_PATH}/translations)

file(GLOB TS_FILES ${TS_SRC_DIR}/*.ts)
set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION ${QM_DST_DIR})
qt6_add_translation(QM_FILES ${TS_FILES})

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

target_link_libraries(
    ${PROJECT_NAME}
    Qt6::Widgets
    Qt6::Svg
    Qt6::SvgWidgets
    Qt6::StateMachine
)

