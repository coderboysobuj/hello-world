#pragma once
#include <cstdint>

namespace mmo::ecs {
    struct TransformComponent {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
    };

    struct NetworkComponent {
        uint32_t networkId = 0;
    };
}
