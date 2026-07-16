#pragma once
#include <cstdint>

namespace reactphysics3d { class RigidBody; }
namespace mmo::render { struct Mesh; }

namespace mmo::ecs {
    struct TransformComponent {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
    };

    struct NetworkComponent {
        uint32_t networkId = 0;
        bool isLocalPlayer = false;
    };

    struct PhysicsComponent {
        reactphysics3d::RigidBody* body = nullptr;
    };

    struct MeshComponent {
        class render::Mesh* mesh = nullptr;
        // Optionally store a color override or material reference here
        float colorR = 1.0f;
        float colorG = 1.0f;
        float colorB = 1.0f;
    };
}
