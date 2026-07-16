import sys

with open("server/world/main.cpp", "r") as f:
    content = f.read()

old_code = """    rp3d::TriangleMesh* triangleMesh = physicsCommon.createTriangleMesh();
    triangleMesh->addSubpart(triangleArray);"""

new_code = """    std::vector<rp3d::Message> messages;
    rp3d::TriangleMesh* triangleMesh = physicsCommon.createTriangleMesh(*triangleArray, messages);
    if (!triangleMesh) {
        std::cerr << "Failed to create TriangleMesh. Errors:\\n";
        for (const auto& msg : messages) {
            std::cerr << msg.text << "\\n";
        }
        return -1;
    }"""

content = content.replace(old_code, new_code)

with open("server/world/main.cpp", "w") as f:
    f.write(content)
