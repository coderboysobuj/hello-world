#pragma once
#include "../core/Types.h"

namespace mmo::render {
    class RHI {
    public:
        virtual ~RHI() = default;
        virtual bool Initialize(void* windowHandle) = 0;
        virtual void Shutdown() = 0;
        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;
    };
}
