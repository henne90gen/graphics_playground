
create_scene(
        RubiksCubeScene.cpp
        RotationCommandStack.cpp
        RubiksCube.cpp
        RubiksCubeData.cpp
        RubiksCubeLogic.cpp
)

create_scene_test(
        TestMain.cpp
        RubiksCubeTest.cpp
        RubiksCubeLogicTest.cpp
        RotationCommandStack.cpp
        RubiksCube.cpp
        RubiksCubeData.cpp
        RubiksCubeLogic.cpp
)
