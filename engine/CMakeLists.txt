add_library(EngineCore STATIC
    core/Application.cpp
    ecs/World.cpp
    network/NetworkManager.cpp
    render/VulkanBackend.cpp
)

target_include_directories(EngineCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(EngineCore PUBLIC 
    SDL2::SDL2main
    SDL2::SDL2
    Vulkan::Vulkan
    GameNetworkingSockets::static
    EnTT::EnTT
)
