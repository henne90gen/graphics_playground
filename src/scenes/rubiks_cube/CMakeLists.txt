
create_scene(
        RubiksCubeScene.cpp
        RotationCommandStack.cpp
        RubiksCube.cpp
        RubiksCubeData.cpp
        RubiksCubeLogic.cpp
        CoreRubiksCube.cpp
        AnimationRubiksCube.cpp
)

create_scene_test(
        RubiksCubeTest.cpp
        RubiksCubeLogicTest.cpp
        RotationCommandStack.cpp
        RubiksCube.cpp
        RubiksCubeData.cpp
        RubiksCubeLogic.cpp
        CoreRubiksCube.cpp
        CoreRubiksCubeTest.cpp
)
