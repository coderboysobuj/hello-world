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
#include "../engine/render/AssetLoader.h"
#include "../engine/render/Mesh.h"

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
    std::unordered_map<uint32_t, entt::entity> networkEntities;
    entt::entity localPlayer = entt::null;

    netManager->SetReceiveCallback([&](uint64_t connectionId, const void* data, size_t size) {
        if (size < sizeof(mmo::network::PacketHeader)) return;
        const auto* header = static_cast<const mmo::network::PacketHeader*>(data);
        
        if (header->opcode == mmo::network::OpCode::SpawnEntity) {
            const auto* packet = static_cast<const mmo::network::SpawnEntityPacket*>(data);
            if (networkEntities.find(packet->networkId) == networkEntities.end()) {
                auto entity = ecsWorld.CreateEntity();
                ecsWorld.AddComponent<mmo::ecs::TransformComponent>(entity, packet->x, packet->y, packet->z);
                ecsWorld.AddComponent<mmo::ecs::NetworkComponent>(entity, packet->networkId);
                
                // Add MeshComponent for rendering
                auto& meshComp = ecsWorld.AddComponent<mmo::ecs::MeshComponent>(entity);
                meshComp.colorR = 0.2f;
                meshComp.colorG = 0.5f;
                meshComp.colorB = 1.0f;
                
                networkEntities[packet->networkId] = entity;
                std::cout << "Spawned Entity ID " << packet->networkId << " at X: " << packet->x << "\n";
            }
        }
        else if (header->opcode == mmo::network::OpCode::SetLocalPlayer) {
            const auto* packet = static_cast<const mmo::network::SetLocalPlayerPacket*>(data);
            if (networkEntities.find(packet->networkId) != networkEntities.end()) {
                localPlayer = networkEntities[packet->networkId];
                auto& netComp = ecsWorld.GetComponent<mmo::ecs::NetworkComponent>(localPlayer);
                netComp.isLocalPlayer = true;
                std::cout << "Local Player is now Entity ID " << packet->networkId << "\n";
            }
        }
        else if (header->opcode == mmo::network::OpCode::UpdateTransform) {
            const auto* packet = static_cast<const mmo::network::UpdateTransformPacket*>(data);
            if (networkEntities.find(packet->networkId) == networkEntities.end()) {
                // If we don't have it, create it
                auto entity = ecsWorld.CreateEntity();
                ecsWorld.AddComponent<mmo::ecs::TransformComponent>(entity, packet->x, packet->y, packet->z);
                ecsWorld.AddComponent<mmo::ecs::NetworkComponent>(entity, packet->networkId);
                
                auto& meshComp = ecsWorld.AddComponent<mmo::ecs::MeshComponent>(entity);
                meshComp.colorR = 0.2f;
                meshComp.colorG = 0.5f;
                meshComp.colorB = 1.0f;
                
                networkEntities[packet->networkId] = entity;
            } else {
                auto entity = networkEntities[packet->networkId];
                auto& transform = ecsWorld.GetComponent<mmo::ecs::TransformComponent>(entity);
                transform.x = packet->x;
                transform.y = packet->y;
                transform.z = packet->z;
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

    // Load assets
    mmo::render::Mesh playerMesh;
    if (mmo::render::AssetLoader::LoadModel("cube.obj", playerMesh)) {
        vulkanBackend.CreateMeshBuffers(playerMesh);
    }

    // Setup Mesh pointers for existing entities (and future)
    // Actually, we'll just set it for any entity that gets created. 
    // We can do this in the loop, or set a global pointer.
    // For now, let's just make it a global pointer that all new entities use.
    
    // Create a Floor entity locally
    auto floorEntity = ecsWorld.CreateEntity();
    ecsWorld.AddComponent<mmo::ecs::TransformComponent>(floorEntity, 0.0f, -2.0f, 0.0f);
    auto& floorMeshComp = ecsWorld.AddComponent<mmo::ecs::MeshComponent>(floorEntity);
    floorMeshComp.mesh = &playerMesh;
    floorMeshComp.colorR = 0.8f;
    floorMeshComp.colorG = 0.8f;
    floorMeshComp.colorB = 0.8f;

    // Initialize Camera State
    float camYaw = 0.0f;
    float camPitch = 0.0f;
    SDL_SetRelativeMouseMode(SDL_TRUE);

    // Run main loop
    app.Run(&vulkanBackend, netManager, [&]() {
        // Handle Mouse Look
        int mouseDX, mouseDY;
        SDL_GetRelativeMouseState(&mouseDX, &mouseDY);
        
        const float mouseSensitivity = 0.005f;
        camYaw -= mouseDX * mouseSensitivity;
        camPitch -= mouseDY * mouseSensitivity;
        
        // Clamp pitch to avoid flipping over
        if (camPitch > 1.5f) camPitch = 1.5f;
        if (camPitch < -1.5f) camPitch = -1.5f;

        // Update Camera Position based on local player
        if (localPlayer != entt::null && ecsWorld.GetRegistry().all_of<mmo::ecs::TransformComponent>(localPlayer)) {
            auto& transform = ecsWorld.GetComponent<mmo::ecs::TransformComponent>(localPlayer);
            glm::vec3 playerPos(transform.x, transform.y, transform.z);
            
            // Calculate camera offset
            float distance = 5.0f;
            float horizontalDist = distance * cos(camPitch);
            float verticalDist = distance * sin(camPitch);
            
            float offsetX = horizontalDist * sin(camYaw);
            float offsetZ = horizontalDist * cos(camYaw);
            
            glm::vec3 cameraPos(playerPos.x - offsetX, playerPos.y - verticalDist, playerPos.z - offsetZ);
            
            // Add a little height to the target so we don't look at the player's feet
            glm::vec3 cameraTarget(playerPos.x, playerPos.y + 1.0f, playerPos.z);
            
            vulkanBackend.SetCamera(cameraPos, cameraTarget);
        }

        // Handle Input
        float inX = 0.0f;
        float inY = 0.0f;
        bool jump = false;
        if (app.IsKeyPressed(SDL_SCANCODE_W)) inY = 1.0f; // Changed to positive forward
        if (app.IsKeyPressed(SDL_SCANCODE_S)) inY = -1.0f;
        if (app.IsKeyPressed(SDL_SCANCODE_A)) inX = -1.0f;
        if (app.IsKeyPressed(SDL_SCANCODE_D)) inX = 1.0f;
        if (app.IsKeyPressed(SDL_SCANCODE_SPACE)) jump = true;

        if (inX != 0.0f || inY != 0.0f || jump) {
            mmo::network::PlayerInputPacket inputPacket;
            inputPacket.header.opcode = mmo::network::OpCode::PlayerInput;
            inputPacket.header.size = sizeof(mmo::network::PlayerInputPacket);
            inputPacket.inputX = inX;
            inputPacket.inputY = inY;
            inputPacket.jump = jump;
            inputPacket.yaw = camYaw;
            netManager->SendMessage(&inputPacket, sizeof(inputPacket));
        }

        // Assign the loaded mesh to any entities that don't have one yet (e.g. from network)
        auto view = ecsWorld.GetRegistry().view<mmo::ecs::MeshComponent>();
        for (auto entity : view) {
            auto& meshComp = view.get<mmo::ecs::MeshComponent>(entity);
            if (meshComp.mesh == nullptr && playerMesh.vertexBuffer != VK_NULL_HANDLE) {
                meshComp.mesh = &playerMesh;
            }
        }

        vulkanBackend.RenderEntities(ecsWorld);
    });

    // Cleanup
    vulkanBackend.DestroyMesh(playerMesh);
    vulkanBackend.Shutdown();
    netManager->Shutdown();
    delete netManager;

    return 0;
}
