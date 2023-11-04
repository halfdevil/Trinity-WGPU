#include "Graphics/RenderPipeline.h"
#include "Graphics/GraphicsDevice.h"
#include "Core/Debugger.h"
#include "Core/Logger.h"

namespace Trinity
{
    RenderPipeline::~RenderPipeline()
    {
        destroy();
    }

    bool RenderPipeline::create(const RenderPipelineProperties& renderProps)
    {
        const wgpu::Device& device = GraphicsDevice::get();
        std::vector<wgpu::BindGroupLayout> bindGroupLayouts;

        for (const BindGroupLayout* bindGroupLayout : renderProps.bindGroupLayouts)
        {
            if (bindGroupLayout)
            {
                bindGroupLayouts.push_back(bindGroupLayout->getHandle());
            }
        }

        wgpu::PipelineLayoutDescriptor layoutDesc = {
            .bindGroupLayoutCount = static_cast<uint32_t>(bindGroupLayouts.size()),
            .bindGroupLayouts = bindGroupLayouts.data()
        };

        mLayout = device.CreatePipelineLayout(&layoutDesc);
        if (!mLayout)
        {
            LogError("wgpu::Device::CreatePipelineLayout() failed!!");
            return false;
        }

        std::vector<wgpu::VertexBufferLayout> buffers;
        for (const auto* vertexLayout : renderProps.vertexLayouts)
        {
            if (vertexLayout)
            {
                buffers.push_back(vertexLayout->getBufferLayout());
            }
        }

        const Shader* shader = renderProps.shader;

        std::vector<wgpu::ColorTargetState> colorTargets;
        for (const ColorTargetState& colorTarget : renderProps.colorTargets)
        {
            wgpu::ColorTargetState state = {
                .format = colorTarget.format,
                .writeMask = colorTarget.colorWriteMask
            };

            if (colorTarget.blendState)
            {
                const wgpu::BlendState& blendState = *colorTarget.blendState;
                state.blend = &blendState;
            }

            colorTargets.push_back(std::move(state));
        }

        wgpu::FragmentState fragment = {
            .module = shader->getHandle(),
            .entryPoint = renderProps.fsEntry.c_str(),
            .targetCount = static_cast<uint32_t>(colorTargets.size()),
            .targets = colorTargets.data()
        };

        wgpu::RenderPipelineDescriptor pipelineDesc = {
            .layout = mLayout,
            .vertex = {
                .module = shader->getHandle(),
                .entryPoint = renderProps.vsEntry.c_str(),
                .bufferCount = static_cast<uint32_t>(buffers.size()),
                .buffers = buffers.data()
            },
            .primitive = {
                .topology = renderProps.primitive.topology,
                .frontFace = renderProps.primitive.frontFace,
                .cullMode = renderProps.primitive.cullMode
            },
            .multisample = {
                .count = renderProps.multisample.count,
                .mask = renderProps.multisample.mask,
                .alphaToCoverageEnabled = renderProps.multisample.alphaToCoverageEnabled
            },
            .fragment = &fragment
        };

        if (renderProps.depthStencil)
        {
            const DepthStencilState& depthStencil = *renderProps.depthStencil;
            wgpu::DepthStencilState depthStencilState = {
                .format = depthStencil.format,
                .depthWriteEnabled = depthStencil.depthWriteEnabled,
                .depthCompare = depthStencil.depthCompare,
                .stencilFront = depthStencil.stencilFront,
                .stencilBack = depthStencil.stencilBack,
                .stencilReadMask = depthStencil.stencilReadMask,
                .stencilWriteMask = depthStencil.stencilWriteMask,
                .depthBias = depthStencil.depthBias,
                .depthBiasSlopeScale = depthStencil.depthBiasSlopeScale,
                .depthBiasClamp = depthStencil.depthBiasClamp
            };

            pipelineDesc.depthStencil = &depthStencilState;
        }

        mHandle = device.CreateRenderPipeline(&pipelineDesc);
        if (!mHandle)
        {
            LogError("wgpu::Device::CreateRenderPipeline() failed!!");
            return false;
        }

        return true;
    }

    void RenderPipeline::destroy()
    {
        mLayout = nullptr;
        mHandle = nullptr;
    }

    std::type_index RenderPipeline::getType() const
    {
        return typeid(RenderPipeline);
    }
}