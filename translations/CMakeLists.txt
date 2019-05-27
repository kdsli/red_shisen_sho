#SET_DIRECTORY_PROPERTIES(PROPERTIES CLEAN_NO_CUSTOM TRUE)
cmake_minimum_required (VERSION 3.14.0)

find_package(Qt5LinguistTools CONFIG REQUIRED)

add_custom_target(translations)
foreach(LANG ${LANGUAGES})
    add_custom_target(
        ts_${LANG}
        COMMAND ${Qt5_LUPDATE_EXECUTABLE} ${SOURCE_DIR}
            -locations relative -no-ui-lines -no-obsolete -silent
            -ts "${TS_SRC_DIR}/${PROJECT_NAME}_${LANG}.ts"
        WORKING_DIRECTORY ${ROOT_DIR}
    )
    add_dependencies(translations ts_${LANG})
endforeach()
