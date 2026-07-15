#include <iostream>
#include <cstring>
#include <cpr/cpr.h>
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

    // 1. Authenticate with Go Login Service
    std::cout << "Authenticating...\n";
    cpr::Response r = cpr::Post(cpr::Url{"http://localhost:8080/login"},
                                cpr::Body{"{\"username\":\"testuser\",\"password\":\"password123\"}"},
                                cpr::Header{{"Content-Type", "application/json"}});

    if (r.status_code != 200) {
        std::cerr << "Authentication failed! Status: " << r.status_code << "\n";
        return -1;
    }

    // Very simple token extraction (In a real app, use a JSON parser like nlohmann-json)
    std::string token = "";
    size_t tokenPos = r.text.find("\"token\":\"");
    if (tokenPos != std::string::npos) {
        tokenPos += 9;
        size_t endPos = r.text.find("\"", tokenPos);
        token = r.text.substr(tokenPos, endPos - tokenPos);
    }

    if (token.empty()) {
        std::cerr << "Failed to parse token from response.\n";
        return -1;
    }
    std::cout << "Authenticated! Token: " << token << "\n";

    // 2. Connect to World Server
    netManager->Connect("127.0.0.1", 27015);

    // 3. Send AuthPacket
    mmo::network::AuthPacket authPacket;
    authPacket.header.opcode = mmo::network::OpCode::Auth;
    authPacket.header.size = sizeof(mmo::network::AuthPacket);
    std::strncpy(authPacket.token, token.c_str(), sizeof(authPacket.token) - 1);
    authPacket.token[sizeof(authPacket.token) - 1] = '\0';
    netManager->SendMessage(&authPacket, sizeof(authPacket));

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
