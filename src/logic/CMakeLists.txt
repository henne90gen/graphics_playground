
add_library(logic
        marching_cubes/MarchingCubes.cpp
        fourier_transform/Fourier.cpp
        )

target_link_libraries(logic PRIVATE FastNoise OpenMP::OpenMP_CXX)

set_target_properties(
        logic PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
elseif (APPLE)
else ()
    target_link_libraries(logic PRIVATE stdc++fs)
endif ()

target_include_directories(logic PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${GLM_DIR}
        ${FAST_NOISE_DIR})

if (CLANG_TIDY_EXE AND RUN_CLANG_TIDY)
    set_target_properties(
            logic PROPERTIES
            CXX_CLANG_TIDY "${DO_CLANG_TIDY}"
    )
endif ()

set(LOGIC_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
