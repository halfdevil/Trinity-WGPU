#include "Graphics/BindGroup.h"
#include "Graphics/GraphicsDevice.h"
#include "Core/Debugger.h"
#include "Core/Logger.h"
#include <type_traits>

namespace Trinity
{
    BindGroup::~BindGroup()
    {
        destroy();
    }

    bool BindGroup::create(const BindGroupLayout& layout, const std::vector<BindGroupItem>& items)
    {
        const wgpu::Device& device = GraphicsDevice::get();
        std::vector<wgpu::BindGroupEntry> entries;

        for (const BindGroupItem& item : items)
        {
            wgpu::BindGroupEntry entry = {
                .binding = item.binding,
                .offset = item.offset,
                .size = item.size
            };

            std::visit(
                [&entry](auto&& resource)
                {
                    using T = std::decay_t<decltype(resource)>;

                    if constexpr (
                        std::is_same_v<T, BufferBindingResource>)
                    {
                        entry.buffer = resource.buffer.getHandle();
                    }
                    else if constexpr (std::is_same_v<T, TextureBindingResource>)
                    {
                        entry.textureView = resource.texture.getView();
                    }
                    else if constexpr (std::is_same_v<T, SamplerBindingResource>)
                    {
                        entry.sampler = resource.sampler.getHandle();
                    }
                },
            item.resource);

            entries.push_back(std::move(entry));
        }

        wgpu::BindGroupDescriptor groupDesc = {
            .layout = layout.getHandle(),
            .entryCount = static_cast<uint32_t>(entries.size()),
            .entries = entries.data() };

        mHandle = device.CreateBindGroup(&groupDesc);
        if (!mHandle)
        {
            LogError("wgpu::Device::CreateBindGroup() failed!!");
            return false;
        }

        return true;
    }

    void BindGroup::destroy()
    {
        mHandle = 0;
    }
    std::type_index BindGroup::getType() const
    {
        return typeid(BindGroup);
    }
}