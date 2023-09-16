#pragma once

#include "Graphics/Texture.h"

namespace Trinity
{
    class FrameBuffer
    {
    public:

        FrameBuffer() = default;
        ~FrameBuffer();

        FrameBuffer(const FrameBuffer&) = delete;
        FrameBuffer& operator = (const FrameBuffer&) = delete;

        FrameBuffer(FrameBuffer&&) = default;
        FrameBuffer& operator = (FrameBuffer&&) = default;

        const std::vector<wgpu::RenderPassColorAttachment>& getColorAttachments() const
        {
            return mColorAttachments;
        }

        const wgpu::RenderPassDepthStencilAttachment& getDepthAttachment() const
        {
            return mDepthStencilAttachment;
        }

        bool hasDepthStencilAttachment() const
        {
            return mHasDepthStencilAttachment;
        }

        bool addColorAttachment(const Texture& texture, wgpu::Color clearColor = { 0, 0, 0, 1 });
        bool setDepthAttachment(const Texture& texture, float depthValue = 0.0f);
        void destroy();

    private:

        std::vector<wgpu::RenderPassColorAttachment> mColorAttachments;
        wgpu::RenderPassDepthStencilAttachment mDepthStencilAttachment;
        bool mHasDepthStencilAttachment{ false };
    };
}