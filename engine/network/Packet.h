#pragma once
#include <cstdint>

namespace mmo::network {
    enum class OpCode : uint16_t {
        None = 0,
        SpawnEntity = 1,
        UpdateTransform = 2,
        PlayerInput = 3,
        Auth = 4,
        SetLocalPlayer = 5
    };

    #pragma pack(push, 1)
    struct PacketHeader {
        OpCode opcode;
        uint16_t size;
    };

    struct AuthPacket {
        PacketHeader header;
        char token[64];
    };

    struct PlayerInputPacket {
        PacketHeader header;
        float inputX; // -1 to 1 (A/D)
        float inputY; // -1 to 1 (W/S)
        bool jump;    // Spacebar
        float yaw;    // Camera yaw in radians
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

    struct SetLocalPlayerPacket {
        PacketHeader header;
        uint32_t networkId;
    };
    #pragma pack(pop)
}
