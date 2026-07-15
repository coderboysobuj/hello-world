#pragma once
#include <string>

#include <functional>

namespace mmo::render { class RHI; }
namespace mmo::network { class NetworkManager; }

namespace mmo::core {
    class Application {
    public:
        Application(const std::string& name, int width, int height);
        ~Application();

        bool Initialize();
        void Run(mmo::render::RHI* rhi, mmo::network::NetworkManager* net, std::function<void()> onUpdate = nullptr);
        void Shutdown();

        bool IsKeyPressed(int scancode) const;
        void* GetWindowHandle() const { return m_windowHandle; }

    private:
        std::string m_name;
        int m_width;
        int m_height;
        bool m_isRunning;
        
        void* m_windowHandle = nullptr;
    };
}
