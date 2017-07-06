hunter_add_package(GTest)
find_package(GTest CONFIG REQUIRED)

add_executable(gpu2cpu main.cpp)
target_link_libraries(gpu2cpu PRIVATE aglet GTest::main)

gauze_add_test(NAME gpu2cpu COMMAND gpu2cpu)
