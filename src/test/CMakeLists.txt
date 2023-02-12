
include(GoogleTest)

function(create_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} GTest::gtest_main core)
    gtest_discover_tests(${name})
    set_target_properties(${name} PROPERTIES
            CXX_STANDARD 20
            CXX_STANDARD_REQUIRED ON)
endfunction()

create_test(FourierTest)

create_test(ModelLoaderTest)
target_link_libraries(ModelLoaderTest model)

#create_test(QuadTreeTest)
