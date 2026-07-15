#pragma once
#include <cstdint>

namespace mmo::network {
    enum class OpCode : uint16_t {
        None = 0,
        SpawnEntity = 1,
        UpdateTransform = 2,
        PlayerInput = 3
    };

    #pragma pack(push, 1)
    struct PacketHeader {
        OpCode opcode;
        uint16_t size;
    };

    struct PlayerInputPacket {
        PacketHeader header;
        float inputX; // -1 to 1 (A/D)
        float inputY; // -1 to 1 (W/S)
    };

    struct SpawnEntityPacket {
        PacketHeader header;
        uint32_t networkId;
        float x, y, z;
    };

    struct UpdateTransformPacket {
        PacketHeader header;
        uint32_t networkId;
        float x, y, z;
    };
    #pragma pack(pop)
}
