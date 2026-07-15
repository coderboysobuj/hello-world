#pragma once
#include "../core/Types.h"
#include <string>

#include <functional>

namespace mmo::network {
    class NetworkManager {
    public:
        using ReceiveCallback = std::function<void(uint64_t connectionId, const void* data, size_t size)>;
        using ConnectionCallback = std::function<void(uint64_t connectionId)>;

        virtual ~NetworkManager() = default;
        virtual bool Initialize() = 0;
        virtual void Shutdown() = 0;
        
        // Networking API
        virtual bool Host(uint16_t port) = 0;
        virtual bool Connect(const std::string& address, uint16_t port) = 0;
        virtual void SendMessage(const void* data, size_t size) = 0;
        virtual void SetReceiveCallback(ReceiveCallback callback) = 0;
        virtual void SetConnectionCallbacks(ConnectionCallback onConnect, ConnectionCallback onDisconnect) = 0;
        virtual void Update() = 0;
    };

    NetworkManager* CreateNetworkManager();
}
