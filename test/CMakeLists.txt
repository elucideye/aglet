hunter_add_package(GTest)
find_package(GTest CONFIG REQUIRED)

add_executable(test-aglet main.cpp)
target_link_libraries(test-aglet PRIVATE aglet GTest::main)

gauze_add_test(NAME test-aglet COMMAND test-aglet)

install(TARGETS test-aglet DESTINATION bin)
