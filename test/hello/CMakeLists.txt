hunter_add_package(GTest)
find_package(GMock CONFIG REQUIRED)

add_executable(hello main.cpp)
target_link_libraries(hello PRIVATE aglet GMock::main)

gauze_add_test(NAME hello COMMAND hello)
