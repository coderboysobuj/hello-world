add_library(EngineCore STATIC
    core/Application.cpp
    ecs/World.cpp
    network/NetworkManager.cpp
    render/VulkanBackend.cpp
    render/AssetLoader.cpp
)

target_include_directories(EngineCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

find_package(glm CONFIG REQUIRED)
find_package(ReactPhysics3D CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

target_link_libraries(EngineCore PUBLIC 
    SDL2::SDL2main
    SDL2::SDL2
    Vulkan::Vulkan
    GPUOpen::VulkanMemoryAllocator
    GameNetworkingSockets::static
    EnTT::EnTT
    glm::glm
    ReactPhysics3D::ReactPhysics3D
    assimp::assimp
)
