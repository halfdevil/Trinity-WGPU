#include "Graphics/BindGroupLayout.h"
#include "Graphics/GraphicsDevice.h"
#include "Core/Debugger.h"
#include "Core/Logger.h"

namespace Trinity
{
    BindGroupLayout::~BindGroupLayout()
    {
        destroy();
    }

    bool BindGroupLayout::create(const std::vector<BindGroupLayoutItem>& items)
    {
        const wgpu::Device& device = GraphicsDevice::get();
        std::vector<wgpu::BindGroupLayoutEntry> entries;

        for (const BindGroupLayoutItem& item : items)
        {
            wgpu::BindGroupLayoutEntry entry = {
                .binding = item.binding,
                .visibility = item.shaderStages
            };

            std::visit([&entry](auto&& bindingLayout) {
                using T = std::decay_t<decltype(bindingLayout)>;

                if constexpr (std::is_same_v<T, BufferBindingLayout>)
                {
                    entry.buffer = {
                        .type = bindingLayout.type,
                        .hasDynamicOffset = bindingLayout.hasDynamicOffset,
                        .minBindingSize = bindingLayout.minBindingSize
                    };
                }
                else if constexpr (std::is_same_v<T, TextureBindingLayout>)
                {
                    entry.texture = {
                        .sampleType = bindingLayout.sampleType,
                        .viewDimension = bindingLayout.viewDimension
                    };
                }
                else if constexpr (std::is_same_v<T, SamplerBindingLayout>)
                {
                    entry.sampler = {
                        .type = bindingLayout.type
                    };
                }
                }, item.bindingLayout);

            entries.push_back(std::move(entry));
        }

        wgpu::BindGroupLayoutDescriptor layoutDesc = {
            .entryCount = static_cast<uint32_t>(entries.size()),
            .entries = entries.data()
        };

        mHandle = device.CreateBindGroupLayout(&layoutDesc);
        if (!mHandle)
        {
            LogError("wgpu::Device::CreateBindGroupLayout() failed!!");
            return false;
        }

        return true;
    }

    void BindGroupLayout::destroy()
    {
        mHandle = 0;
    }

    bool BindGroupLayout::isValid() const
    {
        return mHandle != 0;
    }

    std::type_index BindGroupLayout::getType() const
    {
        return typeid(BindGroupLayout);
    }
}