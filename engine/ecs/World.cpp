#include "World.h"

namespace mmo::ecs {
    World::World() {}
    World::~World() {}

    entt::entity World::CreateEntity() {
        return m_registry.create();
    }

    void World::DestroyEntity(entt::entity entity) {
        m_registry.destroy(entity);
    }
}
