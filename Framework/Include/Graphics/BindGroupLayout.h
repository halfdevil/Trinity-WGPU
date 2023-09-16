#pragma once

#include <webgpu/webgpu_cpp.h>
#include <variant>
#include <vector>

namespace Trinity
{
    struct BufferBindingLayout
    {
        wgpu::BufferBindingType type{ wgpu::BufferBindingType::Undefined };
        bool hasDynamicOffset{ false };
        uint32_t minBindingSize{ 0 };
    };

    struct TextureBindingLayout
    {
        wgpu::TextureSampleType sampleType{ wgpu::TextureSampleType::Undefined };
        wgpu::TextureViewDimension viewDimension{ wgpu::TextureViewDimension::Undefined };
    };

    struct SamplerBindingLayout
    {
        wgpu::SamplerBindingType type{ wgpu::SamplerBindingType::Undefined };
    };

    struct BindGroupLayoutItem
    {
        uint32_t binding{ 0 };
        wgpu::ShaderStage shaderStages{ wgpu::ShaderStage::None };
        std::variant<
            BufferBindingLayout,
            TextureBindingLayout,
            SamplerBindingLayout> bindingLayout;
    };

    class BindGroupLayout
    {
    public:

        BindGroupLayout() = default;
        ~BindGroupLayout();

        BindGroupLayout(const BindGroupLayout&) = delete;
        BindGroupLayout& operator = (const BindGroupLayout&) = delete;

        BindGroupLayout(BindGroupLayout&&) = default;
        BindGroupLayout& operator = (BindGroupLayout&&) = default;

        const wgpu::BindGroupLayout& getHandle() const
        {
            return mHandle;
        }

        bool create(const std::vector<BindGroupLayoutItem>& items);
        void destroy();

        bool isValid() const;

    private:

        wgpu::BindGroupLayout mHandle;
    };
}