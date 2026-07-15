find_package(cpr CONFIG REQUIRED)

add_executable(GameClient main.cpp)
target_link_libraries(GameClient PRIVATE EngineCore cpr::cpr)
