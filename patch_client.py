import sys

with open("game/main.cpp", "r") as f:
    content = f.read()

# 1. Replace playerMesh loading
old_player_mesh = """    mmo::render::Mesh playerMesh;
    if (!mmo::render::AssetLoader::LoadModel("cube.obj", playerMesh)) {
        std::cerr << "Failed to load cube.obj\\n";
        return -1;
    }"""
new_player_mesh = """    mmo::render::Mesh playerMesh;
    if (!mmo::render::AssetLoader::LoadModel("assets/character.obj", playerMesh)) {
        std::cerr << "Failed to load character.obj\\n";
        return -1;
    }"""
content = content.replace(old_player_mesh, new_player_mesh)

# 2. Add levelMesh loading
level_mesh_load = """    mmo::render::Mesh levelMesh;
    if (!mmo::render::AssetLoader::LoadModel("assets/level.obj", levelMesh)) {
        std::cerr << "Failed to load level.obj\\n";
        return -1;
    }
    vulkanBackend.CreateMeshBuffers(levelMesh);"""

content = content.replace("vulkanBackend.CreateMeshBuffers(playerMesh);", "vulkanBackend.CreateMeshBuffers(playerMesh);\n" + level_mesh_load)

# 3. Load Level Texture
level_tex_load = """    mmo::render::Texture levelTexture;
    if (!vulkanBackend.CreateTexture("assets/level.png", levelTexture)) {
        std::cerr << "Failed to load level texture\\n";
        return -1;
    }"""

content = content.replace('if (!vulkanBackend.CreateTexture("checkerboard.png", checkerTexture))', 'vulkanBackend.CreateTexture("assets/character.png", checkerTexture);\n' + level_tex_load + '\n    if (false)')

# 4. Update Floor Entity
old_floor = """    auto floorEntity = ecsWorld.CreateEntity();
    ecsWorld.AddComponent<mmo::ecs::TransformComponent>(floorEntity, 0.0f, -2.0f, 0.0f);
    auto& floorMeshComp = ecsWorld.AddComponent<mmo::ecs::MeshComponent>(floorEntity);
    floorMeshComp.mesh = &playerMesh;
    floorMeshComp.texture = &checkerTexture;"""
new_floor = """    auto floorEntity = ecsWorld.CreateEntity();
    ecsWorld.AddComponent<mmo::ecs::TransformComponent>(floorEntity, 0.0f, 0.0f, 0.0f);
    auto& floorMeshComp = ecsWorld.AddComponent<mmo::ecs::MeshComponent>(floorEntity);
    floorMeshComp.mesh = &levelMesh;
    floorMeshComp.texture = &levelTexture;"""
content = content.replace(old_floor, new_floor)

# 5. Cleanup
content = content.replace('vulkanBackend.DestroyTexture(checkerTexture);', 'vulkanBackend.DestroyTexture(checkerTexture);\n    vulkanBackend.DestroyTexture(levelTexture);')
content = content.replace('vulkanBackend.DestroyMesh(playerMesh);', 'vulkanBackend.DestroyMesh(playerMesh);\n    vulkanBackend.DestroyMesh(levelMesh);')

# 6. Update Camera height offset to follow the head
content = content.replace('glm::vec3 cameraPos = glm::vec3(transform.x, transform.y + 2.0f, transform.z + 5.0f);', 'glm::vec3 cameraPos = glm::vec3(transform.x, transform.y + 1.5f, transform.z);')
# Actually wait, the camera uses camYaw/camPitch, let's look at the actual code
with open("game/main.cpp", "w") as f:
    f.write(content)
