#pragma once
#include "Mesh.h"
#include <string>

namespace mmo::render {
    class VulkanBackend;

    class AssetLoader {
    public:
        static bool LoadModel(const std::string& filepath, Mesh& outMesh);
        // We pass VulkanBackend because creating a texture requires staging buffers, image layout transitions, etc.
        // Actually, it might be cleaner to just load raw pixels here and let VulkanBackend create the texture.
        static bool LoadTextureData(const std::string& filepath, unsigned char*& outPixels, int& outWidth, int& outHeight, int& outChannels);
        static void FreeTextureData(unsigned char* pixels);
    };
}
