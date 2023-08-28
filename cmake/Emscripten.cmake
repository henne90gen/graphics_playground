if (NOT EMSCRIPTEN)
    return()
endif ()

set(CMAKE_EXECUTABLE_SUFFIX ".js")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s ASSERTIONS=1")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s WASM=1")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s ALLOW_MEMORY_GROWTH")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s MIN_WEBGL_VERSION=2")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s LLD_REPORT_UNDEFINED")

# TODO disable this option again and see which target don't build successfully
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s ERROR_ON_UNDEFINED_SYMBOLS=0")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/html)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/html)

set(STATIC_FILES
    ${CMAKE_SOURCE_DIR}/src/web/static/index.html
    ${CMAKE_SOURCE_DIR}/src/web/static/favicon.ico
    ${CMAKE_SOURCE_DIR}/src/web/static/placeholder.png
    ${CMAKE_SOURCE_DIR}/screenshots/gamma-calculation.gif
    ${CMAKE_SOURCE_DIR}/screenshots/meta-balls-2.gif
)

foreach (INPUT_FILE ${STATIC_FILES})
    get_filename_component(INPUT_FILE_NAME ${INPUT_FILE} NAME)
    set(OUTPUT_FILE "${CMAKE_BINARY_DIR}/html/${INPUT_FILE_NAME}")
    add_custom_command(
        OUTPUT ${OUTPUT_FILE}
        COMMAND ${CMAKE_COMMAND} -E copy ${INPUT_FILE} ${OUTPUT_FILE}
        DEPENDS ${INPUT_FILE}
        COMMENT "Copying static file ${INPUT_FILE} to ${OUTPUT_FILE}"
        VERBATIM
    )
    list(APPEND STATIC_FILES_DEST ${OUTPUT_FILE})
endforeach ()

add_custom_target(copy_static_files DEPENDS ${STATIC_FILES_DEST})
