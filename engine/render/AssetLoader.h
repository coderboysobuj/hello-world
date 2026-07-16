#pragma once
#include "Mesh.h"
#include <string>

namespace mmo::render {
    class AssetLoader {
    public:
        static bool LoadModel(const std::string& filepath, Mesh& outMesh);
    };
}
