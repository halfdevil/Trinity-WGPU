#include "Graphics/RenderPass.h"
#include "Graphics/GraphicsDevice.h"
#include "Core/Debugger.h"
#include "Core/Logger.h"

namespace Trinity
{
    RenderPass::~RenderPass()
    {
        destroy();
    }

    void RenderPass::destroy()
    {
        mEncoder = nullptr;
    }

    bool RenderPass::begin(const FrameBuffer& frameBuffer)
    {
        const wgpu::CommandEncoder& commandEncoder = GraphicsDevice::get().getEncoder();
        const auto& colorAttachments = frameBuffer.getColorAttachments();
        const auto& depthStencilAttachment = frameBuffer.getDepthAttachment();

        wgpu::RenderPassDescriptor renderPassDesc = {
            .colorAttachmentCount = static_cast<uint32_t>(colorAttachments.size()),
            .colorAttachments = colorAttachments.data()
        };

        if (frameBuffer.hasDepthStencilAttachment())
        {
            renderPassDesc.depthStencilAttachment = &depthStencilAttachment;
        }

        mEncoder = commandEncoder.BeginRenderPass(&renderPassDesc);
        if (!mEncoder)
        {
            LogError("wgpu::CommandEncoder::BeginRenderPass() failed!!");
            return false;
        }

        return true;
    }

    bool RenderPass::begin()
    {
        const wgpu::CommandEncoder& commandEncoder = GraphicsDevice::get().getEncoder();
        const SwapChain& swapChain = GraphicsDevice::get().getSwapChain();

        wgpu::RenderPassColorAttachment colorAttachment = {
            .view = swapChain.getCurrentView(),
            .loadOp = wgpu::LoadOp::Clear,
            .storeOp = wgpu::StoreOp::Store,
            .clearValue = swapChain.getClearColor()
        };

        wgpu::RenderPassDescriptor renderPassDesc = {
            .colorAttachmentCount = 1,
            .colorAttachments = &colorAttachment
        };

        if (swapChain.hasDepthStencilAttachment())
        {
            wgpu::RenderPassDepthStencilAttachment depthStencilAttachment = {
                .view = swapChain.getDepthStencilView(),
                .depthLoadOp = wgpu::LoadOp::Clear,
                .depthStoreOp = wgpu::StoreOp::Store,
                .depthClearValue = 1.0f
            };

            renderPassDesc.depthStencilAttachment = &depthStencilAttachment;
        }

        mEncoder = commandEncoder.BeginRenderPass(&renderPassDesc);
        if (!mEncoder)
        {
            LogError("wgpu::CommandEncoder::BeginRenderPass() failed!!");
            return false;
        }

        return true;
    }

    void RenderPass::end()
    {
        Assert(mEncoder != nullptr, "RenderPass::begin() not called!!");
        mEncoder.End();
    }

    void RenderPass::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) const
    {
        Assert(mEncoder != nullptr, "RenderPass::begin() not called!!");
        mEncoder.Draw(vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void RenderPass::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
        int32_t baseVertex, uint32_t firstInstance) const
    {
        Assert(mEncoder != nullptr, "RenderPass::begin() not called!!");
        mEncoder.DrawIndexed(indexCount, instanceCount, firstIndex, baseVertex, firstInstance);
    }

    void RenderPass::setBindGroup(uint32_t groupIndex, const BindGroup& bindGroup) const
    {
        Assert(mEncoder != nullptr, "RenderPass::begin() not called!!");
        mEncoder.SetBindGroup(groupIndex, bindGroup.getHandle());
    }

    void RenderPass::setPipeline(const RenderPipeline& pipeline) const
    {
        Assert(mEncoder != nullptr, "RenderPass::begin() not called!!");
        mEncoder.SetPipeline(pipeline.getHandle());
    }

    void RenderPass::setVertexBuffer(uint32_t slot, const VertexBuffer& vertexBuffer) const
    {
        Assert(mEncoder != nullptr, "RenderPass::begin() not called!!");
        mEncoder.SetVertexBuffer(slot, vertexBuffer.getHandle());
    }

    void RenderPass::setIndexBuffer(const IndexBuffer& indexBuffer) const
    {
        Assert(mEncoder != nullptr, "RenderPass::begin() not called!!");
        mEncoder.SetIndexBuffer(indexBuffer.getHandle(), indexBuffer.getIndexFormat());
    }

    void RenderPass::setViewport(float x, float y, float width, float height, float minDepth, float maxDepth) const
    {
        Assert(mEncoder != nullptr, "RenderPass::begin() not called!!");
        mEncoder.SetViewport(x, y, width, height, minDepth, maxDepth);
    }

    void RenderPass::setScissor(uint32_t x, uint32_t y, uint32_t width, uint32_t height) const
    {
        Assert(mEncoder != nullptr, "RenderPass::begin() not called!!");
        mEncoder.SetScissorRect(x, y, width, height);
    }

    void RenderPass::setBlendConstant(float r, float g, float b, float a) const
    {
        Assert(mEncoder != nullptr, "RenderPass::begin() not called!!");

        wgpu::Color c = {
            .r = r,
            .g = g,
            .b = b,
            .a = a
        };

        mEncoder.SetBlendConstant(&c);
    }
}