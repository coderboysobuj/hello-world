#include <iostream>
#include <SDL2/SDL.h>
#include "../engine/core/Application.h"
#include "../engine/network/NetworkManager.h"
#include "../engine/network/Packet.h"
#include "../engine/render/VulkanBackend.h"
#include "../engine/ecs/World.h"
#include "../engine/ecs/Components.h"

int main(int argc, char* argv[]) {
    std::cout << "Starting MMO Game Client...\n";

    mmo::core::Application app("MMO Client - Vice City Inspired", 1280, 720);
    if (!app.Initialize()) {
        std::cerr << "Failed to initialize application.\n";
        return -1;
    }

    // Initialize Network
    mmo::network::NetworkManager* netManager = mmo::network::CreateNetworkManager();
    if (!netManager->Initialize()) {
        std::cerr << "Failed to initialize network manager.\n";
    }

    mmo::ecs::World ecsWorld;
    entt::entity localPlayerProxy = entt::null;

    netManager->SetReceiveCallback([&](uint64_t connectionId, const void* data, size_t size) {
        if (size < sizeof(mmo::network::PacketHeader)) return;
        const auto* header = static_cast<const mmo::network::PacketHeader*>(data);
        
        if (header->opcode == mmo::network::OpCode::UpdateTransform) {
            const auto* packet = static_cast<const mmo::network::UpdateTransformPacket*>(data);
            
            if (localPlayerProxy == entt::null) {
                localPlayerProxy = ecsWorld.CreateEntity();
                ecsWorld.AddComponent<mmo::ecs::TransformComponent>(localPlayerProxy, packet->x, packet->y, packet->z);
                ecsWorld.AddComponent<mmo::ecs::NetworkComponent>(localPlayerProxy, packet->networkId);
                std::cout << "Spawned Proxy Entity ID " << packet->networkId << " at X: " << packet->x << "\n";
            } else {
                auto& transform = ecsWorld.GetComponent<mmo::ecs::TransformComponent>(localPlayerProxy);
                transform.x = packet->x;
                transform.y = packet->y;
                transform.z = packet->z;
                
                static int ticks = 0;
                if (++ticks >= 60) {
                    std::cout << "Proxy Position: [X: " << transform.x << ", Y: " << transform.y << "]\n";
                    ticks = 0;
                }
            }
        }
    });

    netManager->Connect("127.0.0.1", 27015);

    // Initialize Vulkan RHI
    mmo::render::VulkanBackend vulkanBackend;
    if (!vulkanBackend.Initialize(app.GetWindowHandle())) {
        std::cerr << "Failed to initialize Vulkan backend.\n";
    }

    // Run main loop
    app.Run(&vulkanBackend, netManager, [&]() {
        float inX = 0.0f;
        float inY = 0.0f;
        if (app.IsKeyPressed(SDL_SCANCODE_W)) inY = -1.0f;
        if (app.IsKeyPressed(SDL_SCANCODE_S)) inY = 1.0f;
        if (app.IsKeyPressed(SDL_SCANCODE_A)) inX = -1.0f;
        if (app.IsKeyPressed(SDL_SCANCODE_D)) inX = 1.0f;

        if (inX != 0.0f || inY != 0.0f) {
            mmo::network::PlayerInputPacket inputPacket;
            inputPacket.header.opcode = mmo::network::OpCode::PlayerInput;
            inputPacket.header.size = sizeof(mmo::network::PlayerInputPacket);
            inputPacket.inputX = inX;
            inputPacket.inputY = inY;
            netManager->SendMessage(&inputPacket, sizeof(inputPacket));
        }

        vulkanBackend.RenderEntities(ecsWorld);
    });

    // Cleanup
    vulkanBackend.Shutdown();
    netManager->Shutdown();
    delete netManager;

    return 0;
}
