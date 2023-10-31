#pragma once

#include "Core/Resource.h"
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
        virtual ~RenderPass() = default;

        RenderPass(const RenderPass&) = delete;
        RenderPass& operator = (const RenderPass&) = delete;

        RenderPass(RenderPass&&) = default;
        RenderPass& operator = (RenderPass&&) = default;

		virtual void destroy() override;
        virtual std::type_index getType() const override;

        virtual bool begin(const FrameBuffer& frameBuffer);
        virtual bool begin();
        
        virtual void end();
        virtual void submit();

        virtual void draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0,
            uint32_t firstInstance = 0) const;

        virtual void drawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0,
            int32_t baseVertex = 0, uint32_t firstInstance = 0) const;

        virtual void setBindGroup(uint32_t groupIndex, const BindGroup& bindGroup) const;
        virtual void setPipeline(const RenderPipeline& pipeline) const;
        virtual void setVertexBuffer(uint32_t slot, const VertexBuffer& vertexBuffer) const;
        virtual void setIndexBuffer(const IndexBuffer& indexBuffer) const;
        virtual void setViewport(float x, float y, float width, float height, float minDepth, float maxDepth) const;
        virtual void setScissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height) const;
        virtual void setBlendConstant(float r, float g, float b, float a = 1.0f) const;

    protected:

        wgpu::CommandEncoder mCommandEncoder{ nullptr };
        wgpu::RenderPassEncoder mRenderPassEncoder{ nullptr };
    };
}