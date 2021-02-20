add_library(model
        Model.cpp
        )

set_target_properties(model PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON)

target_link_libraries(model
        core
        glad
        image
        opengl)

target_include_directories(model PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${GLM_DIR})

create_scene_benchmark(ModelBench.cpp)
target_link_libraries(model_bench PRIVATE model)
