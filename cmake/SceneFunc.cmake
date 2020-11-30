function(create_scene)
    get_filename_component(SCENE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

    file(GLOB_RECURSE SHADER_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.glsl")
    foreach (INPUT_FILE ${SHADER_FILES})
        get_filename_component(INPUT_FILE_DIR ${INPUT_FILE} DIRECTORY)
        get_filename_component(INPUT_FILE_NAME ${INPUT_FILE} NAME)
        set(SHADER_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)
        set(OUTPUT_FILE_NAME ${INPUT_FILE_DIR}_${INPUT_FILE_NAME}.cpp)
        set(OUTPUT_FILE ${SHADER_DIR}/${OUTPUT_FILE_NAME})
        add_custom_command(
                OUTPUT ${OUTPUT_FILE}
                COMMAND cd ${CMAKE_SOURCE_DIR} && python -m scripts compile-shader ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE} ${OUTPUT_FILE}
                COMMENT "Building shader object ${OUTPUT_FILE_NAME}"
                DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${INPUT_FILE})
        list(APPEND SHADERS ${OUTPUT_FILE})
    endforeach ()

    add_executable(${SCENE_NAME} ${ARGN} ${SHADERS})
    target_include_directories(${SCENE_NAME} PRIVATE
            ${CMAKE_SOURCE_DIR}/src/core
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${GLFW_DIR}/include
            ${GLAD_DIR}/include
            ${GLM_DIR}
            ${IMGUI_DIR}
            ${LOGIC_DIR}
            )
    target_link_libraries(${SCENE_NAME} core)
    set_target_properties(
            ${SCENE_NAME} PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
    )
endfunction()

function(create_scene_test)
    get_filename_component(SCENE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

    add_executable(${SCENE_NAME}_test ${ARGN})

    message("catch dir: ${CATCH_INCLUDE_DIR}")
    message("exec: ${SCENE_NAME}_test")
    message("sources: ${ARGN}")

    set_target_properties(${SCENE_NAME}_test PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON)

    target_include_directories(${SCENE_NAME}_test PRIVATE
            ${CATCH_INCLUDE_DIR}
            ${LOGIC_DIR}
            ${GLM_DIR}
            ${FFMPEG_INCLUDE_DIR})

    catch_discover_tests(${SCENE_NAME}_test)
endfunction()

function(create_scene_benchmark)
    get_filename_component(SCENE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

    add_executable(${SCENE_NAME}_bench ${ARGN})
    target_link_libraries(${SCENE_NAME}_bench PRIVATE
            logic
            benchmark::benchmark
            OpenMP::OpenMP_CXX)
    target_include_directories(${SCENE_NAME}_bench PRIVATE
            ${LOGIC_DIR}
            ${GLM_DIR})
    set_target_properties(
            ${SCENE_NAME}_bench PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
    )
endfunction()

function(add_scene_resource_directory RESOURCE_DIR)
    get_filename_component(SCENE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

    add_custom_target(${SCENE_NAME}_copy_resources
            COMMAND cd ${CMAKE_SOURCE_DIR} && python -m scripts copy-resources ${CMAKE_CURRENT_SOURCE_DIR} ${RESOURCE_DIR} ${CMAKE_BINARY_DIR}/bin/${SCENE_NAME}_resources)
    add_dependencies(${SCENE_NAME} ${SCENE_NAME}_copy_resources)
endfunction()
