
include(GoogleTest)

function(create_test name)
    add_executable(${name} ${name}.cpp)
    target_link_libraries(${name} gtest_main core model)
    gtest_discover_tests(${name})
    set_property(TARGET ${name} PROPERTY CXX_STANDARD 20 CXX_STANDARD_REQUIRED ON)
endfunction()

# TODO make tests run again
#create_test(FourierTest)
#create_test(ModelLoaderTest)
#create_test(QuadTreeTest)
