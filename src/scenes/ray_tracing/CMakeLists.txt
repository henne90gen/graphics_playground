
create_scene(RayTracing.cpp RayTracer.cpp)

create_scene_test(RayTracer3DTest.cpp RayTracer.cpp)
target_include_directories(ray_tracing_test PRIVATE ../../core)
