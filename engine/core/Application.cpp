#include "Application.h"
#include "../render/RHI.h"
#include "../network/NetworkManager.h"
#include <SDL2/SDL.h>
#include <iostream>

namespace mmo::core {
    Application::Application(const std::string& name, int width, int height)
        : m_name(name), m_width(width), m_height(height), m_isRunning(false) {}

    Application::~Application() {
        Shutdown();
    }

    bool Application::Initialize() {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
            std::cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
            return false;
        }

        m_windowHandle = SDL_CreateWindow(m_name.c_str(), 100, 100, m_width, m_height, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);
        if (!m_windowHandle) {
            std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << "\n";
            SDL_Quit();
            return false;
        }

        m_isRunning = true;
        std::cout << "Application initialized successfully.\n";
        return true;
    }

    void Application::Run(mmo::render::RHI* rhi, mmo::network::NetworkManager* net, std::function<void()> onUpdate) {
        SDL_Event event;
        while (m_isRunning) {
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    m_isRunning = false;
                }
            }
            if (net) {
                net->Update();
            }
            if (onUpdate) {
                onUpdate();
            }
            if (rhi) {
                rhi->BeginFrame();
                rhi->EndFrame();
            }
        }
    }

    void Application::Shutdown() {
        if (m_windowHandle) {
            SDL_DestroyWindow(static_cast<SDL_Window*>(m_windowHandle));
            m_windowHandle = nullptr;
        }
        SDL_Quit();
    }

    bool Application::IsKeyPressed(int scancode) const {
        const Uint8* state = SDL_GetKeyboardState(nullptr);
        return state[scancode] != 0;
    }
}
