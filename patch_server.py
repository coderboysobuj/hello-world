import sys

with open("server/world/main.cpp", "r") as f:
    content = f.read()

# 1. Add AssetLoader include
content = content.replace('#include <reactphysics3d/reactphysics3d.h>', '#include <reactphysics3d/reactphysics3d.h>\n#include "../engine/render/AssetLoader.h"')

# 2. Replace Floor creation with Level loading
floor_code = """    // Create Floor
    rp3d::Vector3 floorPosition(0, -2.0, 0);
    rp3d::Quaternion floorOrientation = rp3d::Quaternion::identity();
    rp3d::Transform floorTransform(floorPosition, floorOrientation);
    rp3d::RigidBody* floorBody = physicsWorld->createRigidBody(floorTransform);
    floorBody->setType(rp3d::BodyType::STATIC);
    // Extents are half-sizes! 10x0.1x10 floor -> 5x0.05x5
    rp3d::BoxShape* floorShape = physicsCommon.createBoxShape(rp3d::Vector3(5.0, 0.05, 5.0));
    floorBody->addCollider(floorShape, rp3d::Transform::identity());"""

level_code = """    // Load Level Geometry
    mmo::render::Mesh levelMesh;
    if (!mmo::render::AssetLoader::LoadModel("assets/level.obj", levelMesh)) {
        std::cerr << "Failed to load level.obj\\n";
        return -1;
    }

    rp3d::TriangleVertexArray* triangleArray = new rp3d::TriangleVertexArray(
        static_cast<uint32_t>(levelMesh.vertices.size()),
        &levelMesh.vertices[0].pos.x,
        sizeof(mmo::render::Vertex),
        static_cast<uint32_t>(levelMesh.indices.size() / 3),
        &levelMesh.indices[0],
        3 * sizeof(uint32_t),
        rp3d::TriangleVertexArray::VertexDataType::VERTEX_FLOAT_TYPE,
        rp3d::TriangleVertexArray::IndexDataType::INDEX_INTEGER_TYPE
    );

    rp3d::TriangleMesh* triangleMesh = physicsCommon.createTriangleMesh();
    triangleMesh->addSubpart(triangleArray);

    rp3d::ConcaveMeshShape* levelShape = physicsCommon.createConcaveMeshShape(triangleMesh);

    rp3d::Transform levelTransform(rp3d::Vector3(0, 0, 0), rp3d::Quaternion::identity());
    rp3d::RigidBody* levelBody = physicsWorld->createRigidBody(levelTransform);
    levelBody->setType(rp3d::BodyType::STATIC);
    levelBody->addCollider(levelShape, rp3d::Transform::identity());"""

content = content.replace(floor_code, level_code)

# 3. Replace Player BoxShape with CapsuleShape
player_code = """                // Add collision shape for player (0.5 half extents for 1x1x1 cube)
                rp3d::BoxShape* playerShape = physicsCommon.createBoxShape(rp3d::Vector3(0.5, 0.5, 0.5));
                playerBody->addCollider(playerShape, rp3d::Transform::identity());"""

new_player_code = """                // Add collision shape for player (Capsule: radius 0.4, height 1.0 -> total height 1.8)
                rp3d::CapsuleShape* playerShape = physicsCommon.createCapsuleShape(0.4, 1.0);
                // Shift the capsule up so its bottom is at 0
                rp3d::Transform playerShapeOffset(rp3d::Vector3(0, 0.9, 0), rp3d::Quaternion::identity());
                playerBody->addCollider(playerShape, playerShapeOffset);"""

content = content.replace(player_code, new_player_code)

with open("server/world/main.cpp", "w") as f:
    f.write(content)
