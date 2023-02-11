function(create_scene)
    get_filename_component(SCENE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

    if (EMSCRIPTEN)
        configure_file(${CMAKE_SOURCE_DIR}/src/web/scene.html ${CMAKE_BINARY_DIR}/html/${SCENE_NAME}.html)
    endif ()

    file(GLOB_RECURSE SHADER_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.glsl")
    foreach (INPUT_FILE ${SHADER_FILES})
        get_filename_component(INPUT_FILE_NAME ${INPUT_FILE} NAME)
        set(SHADER_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)
        set(OUTPUT_FILE_NAME ${INPUT_FILE_NAME}.cpp)
        set(OUTPUT_FILE ${SHADER_DIR}/${OUTPUT_FILE_NAME})
        add_custom_command(
                OUTPUT ${OUTPUT_FILE}
                COMMAND cd ${CMAKE_SOURCE_DIR} && python scripts/compile_shader.py ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE} ${OUTPUT_FILE}
                COMMENT "Building shader object ${OUTPUT_FILE_NAME}"
                DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE})
        list(APPEND SHADERS ${OUTPUT_FILE})
    endforeach ()

    add_executable(${SCENE_NAME} ${ARGN} ${SHADERS})
    target_link_libraries(${SCENE_NAME} core opengl)
    set_target_properties(
            ${SCENE_NAME} PROPERTIES
            CXX_STANDARD 20
            CXX_STANDARD_REQUIRED ON
    )
endfunction()

function(create_scene_test)
    if (EMSCRIPTEN)
        return()
    endif ()

    get_filename_component(SCENE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    set(TEST_NAME ${SCENE_NAME}_test)

    add_executable(${TEST_NAME} ${ARGN})
    target_link_libraries(${TEST_NAME} core GTest::gtest_main)
    set_target_properties(${TEST_NAME} PROPERTIES
            CXX_STANDARD 20
            CXX_STANDARD_REQUIRED ON)

    gtest_discover_tests(${TEST_NAME})
endfunction()

function(create_scene_benchmark)
    get_filename_component(SCENE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    set(BENCH_NAME ${SCENE_NAME}_bench)

    add_executable(${BENCH_NAME} ${ARGN})
    target_link_libraries(${BENCH_NAME} PRIVATE core benchmark::benchmark)
    set_target_properties(${BENCH_NAME} PROPERTIES
            CXX_STANDARD 20
            CXX_STANDARD_REQUIRED ON)
endfunction()

function(add_scene_resource_directory RESOURCE_DIR)
    get_filename_component(SCENE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

    add_custom_target(${SCENE_NAME}_copy_resources
            COMMAND cd ${CMAKE_SOURCE_DIR} && python scripts/copy_resources.py ${CMAKE_CURRENT_SOURCE_DIR} ${RESOURCE_DIR} ${CMAKE_BINARY_DIR}/bin/${SCENE_NAME}_resources)
    add_dependencies(${SCENE_NAME} ${SCENE_NAME}_copy_resources)
endfunction()
