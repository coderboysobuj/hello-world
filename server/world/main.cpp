#include <iostream>
#include <thread>
#include <chrono>
#include <unordered_map>
#include "../engine/network/NetworkManager.h"
#include "../engine/network/Packet.h"
#include "../engine/ecs/World.h"
#include "../engine/ecs/Components.h"

int main(int argc, char* argv[]) {
    std::cout << "Starting MMO World Server...\n";

    mmo::network::NetworkManager* netManager = mmo::network::CreateNetworkManager();
    if (!netManager->Initialize()) {
        std::cerr << "Failed to initialize network manager.\n";
        return -1;
    }
    netManager->Host(27015);

    mmo::ecs::World ecsWorld;
    std::unordered_map<uint64_t, entt::entity> clientMap;
    uint32_t nextNetworkId = 1000;

    netManager->SetConnectionCallbacks(
        [&](uint64_t connectionId) {
            std::cout << "Client " << connectionId << " connected. Waiting for authentication...\n";
        },
        [&](uint64_t connectionId) {
            auto it = clientMap.find(connectionId);
            if (it != clientMap.end()) {
                ecsWorld.DestroyEntity(it->second);
                clientMap.erase(it);
                std::cout << "Client " << connectionId << " disconnected and entity destroyed.\n";
            }
        }
    );

    netManager->SetReceiveCallback([&](uint64_t connectionId, const void* data, size_t size) {
        if (size < sizeof(mmo::network::PacketHeader)) return;
        const auto* header = static_cast<const mmo::network::PacketHeader*>(data);
        
        if (header->opcode == mmo::network::OpCode::Auth) {
            const auto* authPacket = static_cast<const mmo::network::AuthPacket*>(data);
            std::string token(authPacket->token);
            
            if (token.find("TOKEN_") == 0) {
                std::cout << "Client " << connectionId << " authenticated successfully with token: " << token << "\n";
                
                auto playerEntity = ecsWorld.CreateEntity();
                ecsWorld.AddComponent<mmo::ecs::TransformComponent>(playerEntity, 0.0f, 0.0f, 0.0f);
                uint32_t netId = ++nextNetworkId;
                ecsWorld.AddComponent<mmo::ecs::NetworkComponent>(playerEntity, netId);
                
                clientMap[connectionId] = playerEntity;

                mmo::network::SpawnEntityPacket packet;
                packet.header.opcode = mmo::network::OpCode::SpawnEntity;
                packet.header.size = sizeof(mmo::network::SpawnEntityPacket);
                packet.networkId = netId;
                packet.x = 0.0f;
                packet.y = 0.0f;
                packet.z = 0.0f;

                netManager->SendMessage(&packet, sizeof(packet));
            } else {
                std::cerr << "Client " << connectionId << " failed authentication.\n";
            }
        }
        else if (header->opcode == mmo::network::OpCode::PlayerInput) {
            const auto* packet = static_cast<const mmo::network::PlayerInputPacket*>(data);
            auto it = clientMap.find(connectionId);
            if (it != clientMap.end()) {
                auto& transform = ecsWorld.GetComponent<mmo::ecs::TransformComponent>(it->second);
                const float speed = 0.1f;
                transform.x += packet->inputX * speed;
                transform.y += packet->inputY * speed;
            }
        }
    });

    bool isRunning = true;
    std::cout << "World Server ticking at 60Hz...\n";

    const int TPS = 60;
    const std::chrono::milliseconds tickDuration(1000 / TPS);

    while (isRunning) {
        auto tickStart = std::chrono::steady_clock::now();

        netManager->Update();

        // Broadcast State for all entities
        auto view = ecsWorld.GetRegistry().view<mmo::ecs::TransformComponent, mmo::ecs::NetworkComponent>();
        for (auto entity : view) {
            auto& transform = view.get<mmo::ecs::TransformComponent>(entity);
            auto& netComp = view.get<mmo::ecs::NetworkComponent>(entity);

            mmo::network::UpdateTransformPacket packet;
            packet.header.opcode = mmo::network::OpCode::UpdateTransform;
            packet.header.size = sizeof(mmo::network::UpdateTransformPacket);
            packet.networkId = netComp.networkId;
            packet.x = transform.x;
            packet.y = transform.y;
            packet.z = transform.z;

            netManager->SendMessage(&packet, sizeof(packet));
        }

        auto tickEnd = std::chrono::steady_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(tickEnd - tickStart);

        if (elapsedTime < tickDuration) {
            std::this_thread::sleep_for(tickDuration - elapsedTime);
        }
    }

    netManager->Shutdown();
    delete netManager;
    return 0;
}
