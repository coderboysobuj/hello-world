#include <iostream>
#include <thread>
#include <chrono>
#include <unordered_map>
#include <cmath>
#include "../engine/network/NetworkManager.h"
#include "../engine/network/Packet.h"
#include "../engine/ecs/World.h"
#include "../engine/ecs/Components.h"
#include <reactphysics3d/reactphysics3d.h>

namespace rp3d = reactphysics3d;

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

    // Initialize Physics
    rp3d::PhysicsCommon physicsCommon;
    rp3d::PhysicsWorld::WorldSettings settings;
    settings.gravity = rp3d::Vector3(0, -9.81, 0);
    rp3d::PhysicsWorld* physicsWorld = physicsCommon.createPhysicsWorld(settings);

    // Create Floor
    rp3d::Vector3 floorPosition(0, -2.0, 0);
    rp3d::Quaternion floorOrientation = rp3d::Quaternion::identity();
    rp3d::Transform floorTransform(floorPosition, floorOrientation);
    rp3d::RigidBody* floorBody = physicsWorld->createRigidBody(floorTransform);
    floorBody->setType(rp3d::BodyType::STATIC);
    // Extents are half-sizes! 10x0.1x10 floor -> 5x0.05x5
    rp3d::BoxShape* floorShape = physicsCommon.createBoxShape(rp3d::Vector3(5.0, 0.05, 5.0));
    floorBody->addCollider(floorShape, rp3d::Transform::identity());

    netManager->SetConnectionCallbacks(
        [&](uint64_t connectionId) {
            std::cout << "Client " << connectionId << " connected. Waiting for authentication...\n";
        },
        [&](uint64_t connectionId) {
            auto it = clientMap.find(connectionId);
            if (it != clientMap.end()) {
                if (ecsWorld.GetRegistry().all_of<mmo::ecs::PhysicsComponent>(it->second)) {
                    auto& physComp = ecsWorld.GetComponent<mmo::ecs::PhysicsComponent>(it->second);
                    if (physComp.body) {
                        physicsWorld->destroyRigidBody(physComp.body);
                    }
                }
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
                ecsWorld.AddComponent<mmo::ecs::TransformComponent>(playerEntity, 0.0f, 2.0f, 0.0f);
                
                // Add Physics Body
                rp3d::Vector3 startPos(0.0, 2.0, 0.0);
                rp3d::Transform playerTransform(startPos, rp3d::Quaternion::identity());
                rp3d::RigidBody* playerBody = physicsWorld->createRigidBody(playerTransform);
                playerBody->setType(rp3d::BodyType::DYNAMIC);
                
                // Prevent character from tumbling like dice
                playerBody->setAngularLockAxisFactor(rp3d::Vector3(0, 0, 0));
                
                // Add collision shape for player (0.5 half extents for 1x1x1 cube)
                rp3d::BoxShape* playerShape = physicsCommon.createBoxShape(rp3d::Vector3(0.5, 0.5, 0.5));
                playerBody->addCollider(playerShape, rp3d::Transform::identity());

                ecsWorld.AddComponent<mmo::ecs::PhysicsComponent>(playerEntity, playerBody);

                uint32_t netId = ++nextNetworkId;
                ecsWorld.AddComponent<mmo::ecs::NetworkComponent>(playerEntity, netId);
                
                clientMap[connectionId] = playerEntity;

                mmo::network::SpawnEntityPacket packet;
                packet.header.opcode = mmo::network::OpCode::SpawnEntity;
                packet.header.size = sizeof(mmo::network::SpawnEntityPacket);
                packet.networkId = netId;
                packet.x = 0.0f;
                packet.y = 2.0f;
                packet.z = 0.0f;

                netManager->SendMessage(&packet, sizeof(packet));

                mmo::network::SetLocalPlayerPacket localPlayerPacket;
                localPlayerPacket.header.opcode = mmo::network::OpCode::SetLocalPlayer;
                localPlayerPacket.header.size = sizeof(mmo::network::SetLocalPlayerPacket);
                localPlayerPacket.networkId = netId;
                netManager->SendMessageTo(connectionId, &localPlayerPacket, sizeof(localPlayerPacket));
            } else {
                std::cerr << "Client " << connectionId << " failed authentication.\n";
            }
        }
        else if (header->opcode == mmo::network::OpCode::PlayerInput) {
            const auto* packet = static_cast<const mmo::network::PlayerInputPacket*>(data);
            auto it = clientMap.find(connectionId);
            if (it != clientMap.end()) {
                auto& physComp = ecsWorld.GetComponent<mmo::ecs::PhysicsComponent>(it->second);
                if (physComp.body) {
                    rp3d::Vector3 vel = physComp.body->getLinearVelocity();
                    const float moveSpeed = 5.0f;
                    
                    float forwardX = std::sin(packet->yaw);
                    float forwardZ = -std::cos(packet->yaw);
                    
                    float rightX = std::cos(packet->yaw);
                    float rightZ = std::sin(packet->yaw);
                    
                    vel.x = (rightX * packet->inputX + forwardX * packet->inputY) * moveSpeed;
                    vel.z = (rightZ * packet->inputX + forwardZ * packet->inputY) * moveSpeed;

                    if (packet->jump) {
                        // Apply a jump impulse if not already moving much upwards
                        if (vel.y < 0.1f) {
                            vel.y = 5.0f;
                        }
                    }

                    physComp.body->setLinearVelocity(vel);
                }
            }
        }
    });

    bool isRunning = true;
    std::cout << "World Server ticking at 60Hz...\n";

    const int TPS = 60;
    const std::chrono::milliseconds tickDuration(1000 / TPS);

    while (isRunning) {
        auto tickStart = std::chrono::steady_clock::now();

        // Step Physics
        physicsWorld->update(1.0f / 60.0f);

        // Sync Physics to Transform
        auto physView = ecsWorld.GetRegistry().view<mmo::ecs::TransformComponent, mmo::ecs::PhysicsComponent>();
        for (auto entity : physView) {
            auto& transform = physView.get<mmo::ecs::TransformComponent>(entity);
            auto& physComp = physView.get<mmo::ecs::PhysicsComponent>(entity);
            if (physComp.body) {
                const rp3d::Transform& pt = physComp.body->getTransform();
                transform.x = pt.getPosition().x;
                transform.y = pt.getPosition().y;
                transform.z = pt.getPosition().z;
            }
        }

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
