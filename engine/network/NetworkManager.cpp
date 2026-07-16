#include "NetworkManager.h"
#include <GameNetworkingSockets/steam/steamnetworkingsockets.h>
#include <GameNetworkingSockets/steam/isteamnetworkingutils.h>
#include <iostream>
#include <cassert>
#include <cstring>
#include <vector>
#include <algorithm>

namespace mmo::network {
    class GNSNetworkManager : public NetworkManager {
    public:
        static GNSNetworkManager* s_instance;

        GNSNetworkManager() {
            s_instance = this;
        }

        ~GNSNetworkManager() override {
            s_instance = nullptr;
        }

        bool Initialize() override {
            SteamNetworkingErrMsg errMsg;
            if (!GameNetworkingSockets_Init(nullptr, errMsg)) {
                std::cerr << "GameNetworkingSockets_Init failed: " << errMsg << "\n";
                return false;
            }
            m_interface = SteamNetworkingSockets();
            std::cout << "GameNetworkingSockets initialized.\n";
            return true;
        }

        void Shutdown() override {
            if (m_listenSocket != k_HSteamListenSocket_Invalid) {
                m_interface->CloseListenSocket(m_listenSocket);
                m_listenSocket = k_HSteamListenSocket_Invalid;
            }
            if (m_connection != k_HSteamNetConnection_Invalid) {
                m_interface->CloseConnection(m_connection, 0, "Application quitting", false);
                m_connection = k_HSteamNetConnection_Invalid;
            }
            if (m_pollGroup != k_HSteamNetPollGroup_Invalid) {
                m_interface->DestroyPollGroup(m_pollGroup);
                m_pollGroup = k_HSteamNetPollGroup_Invalid;
            }

            GameNetworkingSockets_Kill();
        }

        bool Host(uint16_t port) override {
            SteamNetworkingIPAddr serverLocalAddr;
            serverLocalAddr.Clear();
            serverLocalAddr.m_port = port;

            SteamNetworkingConfigValue_t opt;
            opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)OnConnectionStatusChanged);

            m_listenSocket = m_interface->CreateListenSocketIP(serverLocalAddr, 1, &opt);
            if (m_listenSocket == k_HSteamListenSocket_Invalid) {
                std::cerr << "Failed to listen on port " << port << "\n";
                return false;
            }

            m_pollGroup = m_interface->CreatePollGroup();
            if (m_pollGroup == k_HSteamNetPollGroup_Invalid) {
                std::cerr << "Failed to create poll group\n";
                return false;
            }

            std::cout << "Server listening on port " << port << "...\n";
            m_isServer = true;
            return true;
        }

        bool Connect(const std::string& address, uint16_t port) override {
            SteamNetworkingIPAddr serverAddr;
            serverAddr.Clear();
            serverAddr.ParseString(address.c_str());
            serverAddr.m_port = port;

            SteamNetworkingConfigValue_t opt;
            opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)OnConnectionStatusChanged);

            m_connection = m_interface->ConnectByIPAddress(serverAddr, 1, &opt);
            if (m_connection == k_HSteamNetConnection_Invalid) {
                std::cerr << "Failed to connect to " << address << ":" << port << "\n";
                return false;
            }

            std::cout << "Connecting to " << address << ":" << port << "...\n";
            m_isServer = false;
            return true;
        }

        void SendMessage(const void* data, size_t size) override {
            if (!m_isServer && m_connection != k_HSteamNetConnection_Invalid) {
                m_interface->SendMessageToConnection(m_connection, data, size, k_nSteamNetworkingSend_Reliable, nullptr);
            } else if (m_isServer) {
                for (auto conn : m_clients) {
                    m_interface->SendMessageToConnection(conn, data, size, k_nSteamNetworkingSend_Reliable, nullptr);
                }
            }
        }

        void SendMessageTo(uint64_t connectionId, const void* data, size_t size) override {
            if (m_isServer && m_interface) {
                m_interface->SendMessageToConnection((HSteamNetConnection)connectionId, data, size, k_nSteamNetworkingSend_Reliable, nullptr);
            }
        }

        void SetReceiveCallback(ReceiveCallback callback) override {
            m_receiveCallback = callback;
        }

        void SetConnectionCallbacks(ConnectionCallback onConnect, ConnectionCallback onDisconnect) override {
            m_onConnect = onConnect;
            m_onDisconnect = onDisconnect;
        }

        void Update() override {
            m_interface->RunCallbacks();

            if (m_isServer && m_pollGroup != k_HSteamNetPollGroup_Invalid) {
                PollMessages(m_pollGroup);
            } else if (!m_isServer && m_connection != k_HSteamNetConnection_Invalid) {
                PollConnection(m_connection);
            }
        }

    private:
        ISteamNetworkingSockets* m_interface = nullptr;
        HSteamListenSocket m_listenSocket = k_HSteamListenSocket_Invalid;
        HSteamNetPollGroup m_pollGroup = k_HSteamNetPollGroup_Invalid;
        HSteamNetConnection m_connection = k_HSteamNetConnection_Invalid;
        bool m_isServer = false;
        ReceiveCallback m_receiveCallback;
        ConnectionCallback m_onConnect;
        ConnectionCallback m_onDisconnect;
        
        std::vector<HSteamNetConnection> m_clients;

        void PollMessages(HSteamNetPollGroup pollGroup) {
            ISteamNetworkingMessage* incomingMsg = nullptr;
            while (m_interface->ReceiveMessagesOnPollGroup(pollGroup, &incomingMsg, 1) > 0) {
                if (incomingMsg) {
                    if (m_receiveCallback) m_receiveCallback(incomingMsg->m_conn, incomingMsg->m_pData, incomingMsg->m_cbSize);
                    incomingMsg->Release();
                }
            }
        }

        void PollConnection(HSteamNetConnection conn) {
            ISteamNetworkingMessage* incomingMsg = nullptr;
            while (m_interface->ReceiveMessagesOnConnection(conn, &incomingMsg, 1) > 0) {
                if (incomingMsg) {
                    if (m_receiveCallback) m_receiveCallback(incomingMsg->m_conn, incomingMsg->m_pData, incomingMsg->m_cbSize);
                    incomingMsg->Release();
                }
            }
        }

        static void OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo) {
            s_instance->OnConnectionStatusChangedInternal(pInfo);
        }

        void OnConnectionStatusChangedInternal(SteamNetConnectionStatusChangedCallback_t* pInfo) {
            switch (pInfo->m_info.m_eState) {
                case k_ESteamNetworkingConnectionState_None:
                    break;
                case k_ESteamNetworkingConnectionState_Connecting:
                    if (m_isServer) {
                        if (m_interface->AcceptConnection(pInfo->m_hConn) != k_EResultOK) {
                            m_interface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
                            std::cerr << "Failed to accept connection.\n";
                            break;
                        }
                        if (!m_interface->SetConnectionPollGroup(pInfo->m_hConn, m_pollGroup)) {
                            m_interface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
                            std::cerr << "Failed to set poll group.\n";
                            break;
                        }
                        std::cout << "Incoming client connection accepted.\n";
                    }
                    break;
                case k_ESteamNetworkingConnectionState_FindingRoute:
                    break;
                case k_ESteamNetworkingConnectionState_Connected:
                    std::cout << "Connection established!\n";
                    if (m_isServer) {
                        m_clients.push_back(pInfo->m_hConn);
                        if (m_onConnect) m_onConnect(pInfo->m_hConn);
                    }
                    break;
                case k_ESteamNetworkingConnectionState_ClosedByPeer:
                case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
                    std::cout << "Connection closed (" << pInfo->m_info.m_szEndDebug << ")\n";
                    m_interface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
                    if (m_isServer) {
                        auto it = std::find(m_clients.begin(), m_clients.end(), pInfo->m_hConn);
                        if (it != m_clients.end()) m_clients.erase(it);
                        if (m_onDisconnect) m_onDisconnect(pInfo->m_hConn);
                    } else {
                        m_connection = k_HSteamNetConnection_Invalid;
                    }
                    break;
            }
        }
    };

    GNSNetworkManager* GNSNetworkManager::s_instance = nullptr;

    NetworkManager* CreateNetworkManager() {
        return new GNSNetworkManager();
    }
}
