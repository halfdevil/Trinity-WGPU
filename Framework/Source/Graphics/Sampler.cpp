#include "Graphics/Sampler.h"
#include "Graphics/GraphicsDevice.h"
#include "Core/Debugger.h"
#include "Core/Logger.h"

namespace Trinity
{
    Sampler::~Sampler()
    {
        destroy();
    }

    bool Sampler::create(const SamplerProperties& samplerProps)
    {
        const wgpu::Device& device = GraphicsDevice::get();

        wgpu::SamplerDescriptor samplerDesc = {
            .addressModeU = samplerProps.addressModeU,
            .addressModeV = samplerProps.addressModeV,
            .addressModeW = samplerProps.addressModeW,
            .magFilter = samplerProps.magFilter,
            .minFilter = samplerProps.minFilter,
            .mipmapFilter = samplerProps.mipmapFilter,
            .compare = samplerProps.compare
        };

        mHandle = device.CreateSampler(&samplerDesc);
        if (!mHandle)
        {
            LogError("wgpu::Device::CreateSampler() failed!!");
            return false;
        }

        return true;
    }

    void Sampler::destroy()
    {
        mHandle = nullptr;
    }

    std::type_index Sampler::getType() const
    {
        return typeid(Sampler);
    }
}