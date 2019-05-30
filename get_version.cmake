execute_process(COMMAND ${GIT_EXECUTABLE} describe --tag --match "[0-9]*.[0-9]*.[0-9]*"
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    RESULT_VARIABLE status
    OUTPUT_VARIABLE GIT_VERSION
    ERROR_QUIET
)

message("Version = ${GIT_VERSION}")

#string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" VERSION ${GIT_VERSION})