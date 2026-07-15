#pragma once
#include <entt/entt.hpp>
#include <cstdint>

namespace mmo::ecs {
    class World {
    public:
        World();
        ~World();

        entt::entity CreateEntity();
        void DestroyEntity(entt::entity entity);

        template<typename T, typename... Args>
        T& AddComponent(entt::entity entity, Args&&... args) {
            return m_registry.emplace<T>(entity, std::forward<Args>(args)...);
        }

        template<typename T>
        T& GetComponent(entt::entity entity) {
            return m_registry.get<T>(entity);
        }

        template<typename T>
        bool HasComponent(entt::entity entity) {
            return m_registry.any_of<T>(entity);
        }

        entt::registry& GetRegistry() { return m_registry; }

    private:
        entt::registry m_registry;
    };
}
