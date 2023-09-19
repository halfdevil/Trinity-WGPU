#include "Graphics/FrameBuffer.h"
#include "Core/Debugger.h"
#include "Core/Logger.h"

namespace Trinity
{
    FrameBuffer::~FrameBuffer()
    {
        destroy();
    }

    void FrameBuffer::destroy()
    {
        mColorAttachments.clear();
        mDepthStencilAttachment = {};
    }

    std::type_index FrameBuffer::getType() const
    {
        return typeid(Resource);
    }

    bool FrameBuffer::addColorAttachment(const Texture& texture, wgpu::Color clearColor)
    {
        wgpu::RenderPassColorAttachment colorAttachment = {
            .view = texture.getView(),
            .loadOp = wgpu::LoadOp::Clear,
            .storeOp = wgpu::StoreOp::Store,
            .clearValue = clearColor
        };

        mColorAttachments.push_back(std::move(colorAttachment));
        return true;
    }

    bool FrameBuffer::setDepthAttachment(const Texture& texture, float depthValue)
    {
        mDepthStencilAttachment = {
            .view = texture.getView(),
            .depthLoadOp = wgpu::LoadOp::Clear,
            .depthStoreOp = wgpu::StoreOp::Store,
            .depthClearValue = depthValue
        };

        return true;
    }
}