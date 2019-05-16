
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(SOURCES
        ${SRC_DIR}/RubiksCubeLogic.cpp
        ${SRC_DIR}/RubiksCube.cpp
        ${SRC_DIR}/RotationCommandStack.cpp)

add_library(logic ${SOURCES})
target_include_directories(logic PRIVATE
        ${SRC_DIR}
        ${GLM_DIR})

set(LOGIC_DIR ${SRC_DIR} PARENT_SCOPE)
