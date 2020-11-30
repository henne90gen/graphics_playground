function(create_scene SCENE_NAME ADDITIONAL_SOURCE_FILES)
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

    add_executable(${SCENE_NAME} ${ADDITIONAL_SOURCE_FILES} ${SHADERS})
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

endfunction()
