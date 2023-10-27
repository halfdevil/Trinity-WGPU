#pragma once

#include "Core/Resource.h"
#include "Graphics/Shader.h"
#include "Graphics/VertexBuffer.h"
#include "Graphics/BindGroupLayout.h"
#include <optional>

namespace Trinity
{
    static constexpr const char* kDefaultVSEntry = "vs_main";
    static constexpr const char* kDefaultFSEntry = "fs_main";

    struct ColorTargetState
    {
        wgpu::TextureFormat format{ wgpu::TextureFormat::Undefined };
        wgpu::ColorWriteMask colorWriteMask{ wgpu::ColorWriteMask::All };
        std::optional<wgpu::BlendState> blendState{ std::nullopt };
    };

    struct PrimitiveState
    {
        wgpu::PrimitiveTopology topology{ wgpu::PrimitiveTopology::TriangleList };
        wgpu::FrontFace frontFace{ wgpu::FrontFace::CCW };
        wgpu::CullMode cullMode{ wgpu::CullMode::None };
    };

    struct DepthStencilState
    {
        wgpu::TextureFormat format{ wgpu::TextureFormat::Undefined };
        bool depthWriteEnabled{ false };
        wgpu::CompareFunction depthCompare{ wgpu::CompareFunction::Always };
        wgpu::StencilFaceState stencilFront;
        wgpu::StencilFaceState stencilBack;
        uint32_t stencilReadMask{ 0xFFFFFFFF };
        uint32_t stencilWriteMask{ 0xFFFFFFFF };
        int32_t depthBias{ 0 };
        float depthBiasSlopeScale{ 0.0f };
        float depthBiasClamp{ 0.0f };
    };

    struct MultisampleState
    {
        uint32_t count{ 1 };
        uint32_t mask{ 0xFFFFFFFF };
        bool alphaToCoverageEnabled{ false };
    };

    struct RenderPipelineProperties
    {
        Shader* shader{ nullptr };
        std::string vsEntry{ kDefaultVSEntry };
        std::string fsEntry{ kDefaultFSEntry };
        std::vector<const BindGroupLayout*> bindGroupLayouts;
        std::vector<const VertexBuffer*> vertexBuffers;
        std::vector<ColorTargetState> colorTargets;
        PrimitiveState primitive;
        MultisampleState multisample;
        std::optional<DepthStencilState> depthStencil{ std::nullopt };
    };

    class RenderPipeline : public Resource
    {
    public:

        RenderPipeline() = default;
        ~RenderPipeline();

        RenderPipeline(const RenderPipeline&) = delete;
        RenderPipeline& operator = (const RenderPipeline&) = delete;

        RenderPipeline(RenderPipeline&&) noexcept = default;
        RenderPipeline& operator = (RenderPipeline&&) noexcept = default;

        const wgpu::PipelineLayout& getLayout() const
        {
            return mLayout;
        }

        const wgpu::RenderPipeline& getHandle() const
        {
            return mHandle;
        }

        bool create(const RenderPipelineProperties& renderProps);

		virtual std::type_index getType() const override;
		virtual void destroy() override;

    private:

        wgpu::PipelineLayout mLayout;
        wgpu::RenderPipeline mHandle;
    };
}