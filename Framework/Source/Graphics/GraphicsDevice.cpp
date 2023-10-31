#include "Graphics/GraphicsDevice.h"
#include "Core/Logger.h"
#include "Core/Debugger.h"

namespace Trinity
{
    GraphicsDevice::~GraphicsDevice()
    {
        destroy();
    }

    void GraphicsDevice::create(const Window& window)
    {
        mInstance = wgpu::CreateInstance();
        if (!mInstance)
        {
            LogError("wgpu::CreateInstance() failed!!");
            onCreated.notify(false);
            return;
        }

        auto windowSurfaceDesc = window.getSurfaceDescriptor();
        wgpu::SurfaceDescriptor surfaceDesc;
        surfaceDesc.nextInChain = windowSurfaceDesc.get();

        mSurface = mInstance.CreateSurface(&surfaceDesc);
        if (!mSurface)
        {
            LogError("wgpu::Instance::CreateSurface() failed!!");
            onCreated.notify(false);
            return;
        }

        wgpuInstanceRequestAdapter(mInstance.Get(), nullptr,
            [](WGPURequestAdapterStatus status, WGPUAdapter adapter, char const* message, void* userdata) {
                if (status != WGPURequestAdapterStatus_Success)
                {
                    LogError("wgpu::Instance::RequestAdapter() failed!!");
                    reinterpret_cast<GraphicsDevice*>(userdata)->onCreated.notify(false);
                    return;
                }

                wgpuAdapterRequestDevice(adapter, nullptr,
                    [](WGPURequestDeviceStatus status, WGPUDevice device, char const* message, void* userdata) {
                        if (status != WGPURequestDeviceStatus_Success)
                        {
                            LogError("wgpu::Adapter::RequestDevice() failed!!");
                            reinterpret_cast<GraphicsDevice*>(userdata)->onCreated.notify(false);
                            return;
                        }

                        GraphicsDevice* graphics = reinterpret_cast<GraphicsDevice*>(userdata);
                        graphics->setupDevice(wgpu::Device::Acquire(device));
                        graphics->onCreated.notify(true);
                    },
                    userdata);
            },
        this);
    }

    void GraphicsDevice::destroy()
    {
        mQueue = nullptr;
        mDevice = nullptr;
        mSurface = nullptr;
        mInstance = nullptr;
    }

    bool GraphicsDevice::setupSwapChain(const Window& window, wgpu::PresentMode presentMode,
        wgpu::TextureFormat depthFormat)
    {
        if (!mSwapChain.create(window.getWidth(), window.getHeight(),
            mSurface, presentMode, depthFormat))
        {
            LogError("SwapChain::create() failed!!");
            return false;
        }

        return true;
    }

    void GraphicsDevice::setClearColor(const wgpu::Color& clearColor)
    {
        mSwapChain.setClearColor(clearColor);
    }

    void GraphicsDevice::clearScreen()
    {
        wgpu::RenderPassColorAttachment colorAttachment = {
            .view = mSwapChain.getCurrentView(),
            .loadOp = wgpu::LoadOp::Clear,
            .storeOp = wgpu::StoreOp::Store,
            .clearValue = mSwapChain.getClearColor()
        };

        wgpu::RenderPassDescriptor renderPassDesc = {
            .colorAttachmentCount = 1,
            .colorAttachments = &colorAttachment
        };

        if (mSwapChain.hasDepthStencilAttachment())
        {
            wgpu::RenderPassDepthStencilAttachment depthStencilAttachment = {
                .view = mSwapChain.getDepthStencilView(),
                .depthLoadOp = wgpu::LoadOp::Clear,
                .depthStoreOp = wgpu::StoreOp::Store,
                .depthClearValue = 1.0f
            };

            renderPassDesc.depthStencilAttachment = &depthStencilAttachment;
        }

        auto& graphicsDevice = GraphicsDevice::get();
        auto commandEncoder = graphicsDevice.getDevice().CreateCommandEncoder();

        if (!commandEncoder)
        {
            LogError("Device::CreateCommandEncoder() failed");
            return;
        }

        auto renderPassEncoder = commandEncoder.BeginRenderPass(&renderPassDesc);
        if (!renderPassEncoder)
        {
            LogError("wgpu::CommandEncoder::BeginRenderPass() failed!!");
            return;
        }

        renderPassEncoder.End();

        auto commands = commandEncoder.Finish();
        graphicsDevice.getQueue().Submit(1, &commands);
    }

    void GraphicsDevice::present()
    {
        mSwapChain.present();
    }

    void GraphicsDevice::setupDevice(wgpu::Device device)
    {
        mDevice = device;
        mQueue = mDevice.GetQueue();

        mDevice.SetUncapturedErrorCallback(
            [](WGPUErrorType type, char const* message, void* userdata) {
                LogError("WGPU error (%d): %s", type, message);
            },
        this);

#ifndef __EMSCRIPTEN__
        mDevice.SetDeviceLostCallback(
            [](WGPUDeviceLostReason reason, char const* message, void* userdata) {
                const bool destroyed = reason & WGPUDeviceLostReason_Destroyed;
                if (!destroyed)
                {
                    LogError("WGPU error (%d): %s", reason, message);
                }

                GraphicsDevice* graphics = reinterpret_cast<GraphicsDevice*>(userdata);
                graphics->deviceLost(destroyed);
            },
        this);
#endif
    }

    void GraphicsDevice::deviceLost(bool destroyed)
    {
        onDeviceLost.notify(destroyed);
    }
}