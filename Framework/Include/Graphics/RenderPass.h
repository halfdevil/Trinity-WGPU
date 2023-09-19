#pragma once

#include "Graphics/Resource.h"
#include "Graphics/FrameBuffer.h"
#include "Graphics/SwapChain.h"
#include "Graphics/RenderPipeline.h"
#include "Graphics/VertexBuffer.h"
#include "Graphics/IndexBuffer.h"
#include "Graphics/BindGroup.h"

namespace Trinity
{
    class RenderPass : public Resource
    {
    public:

        RenderPass() = default;
        virtual ~RenderPass();

        RenderPass(const RenderPass&) = delete;
        RenderPass& operator = (const RenderPass&) = delete;

        RenderPass(RenderPass&&) = default;
        RenderPass& operator = (RenderPass&&) = default;

        virtual std::type_index getType() const override;

        bool begin(const FrameBuffer& frameBuffer);
        bool begin();
        void end();
        void destroy();

        void draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0,
            uint32_t firstInstance = 0) const;

        void drawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0,
            int32_t baseVertex = 0, uint32_t firstInstance = 0) const;

        void setBindGroup(uint32_t groupIndex, const BindGroup& bindGroup) const;
        void setPipeline(const RenderPipeline& pipeline) const;
        void setVertexBuffer(uint32_t slot, const VertexBuffer& vertexBuffer) const;
        void setIndexBuffer(const IndexBuffer& indexBuffer) const;
        void setViewport(float x, float y, float width, float height, float minDepth, float maxDepth) const;
        void setScissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height) const;
        void setBlendConstant(float r, float g, float b, float a = 1.0f) const;

    private:

        wgpu::RenderPassEncoder mEncoder;
    };
}