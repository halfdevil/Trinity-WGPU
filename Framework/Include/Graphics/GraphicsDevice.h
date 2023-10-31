#pragma once

#include "Graphics/SwapChain.h"
#include "Graphics/RenderPass.h"
#include "Graphics/ComputePass.h"
#include "Core/Singleton.h"
#include "Core/Observer.h"
#include "Core/Window.h"
#include <cstdint>
#include <string>

namespace Trinity
{
    class GraphicsDevice : public Singleton<GraphicsDevice>
    {
    public:

        GraphicsDevice() = default;
        ~GraphicsDevice();

        GraphicsDevice(const GraphicsDevice&) = delete;
        GraphicsDevice& operator = (const GraphicsDevice&) = delete;

        GraphicsDevice(GraphicsDevice&&) = delete;
        GraphicsDevice& operator = (GraphicsDevice&&) = delete;

        const wgpu::Surface& getSurface() const
        {
            return mSurface;
        }

        const wgpu::Device& getDevice() const
        {
            return mDevice;
        }

        const wgpu::Queue& getQueue() const
        {
            return mQueue;
        }

        const SwapChain& getSwapChain() const
        {
            return mSwapChain;
        }

        operator const wgpu::Device& () const
        {
            return mDevice;
        }

        virtual void create(const Window& window);
        virtual void destroy();

        virtual bool setupSwapChain(const Window& window, wgpu::PresentMode presentMode,
            wgpu::TextureFormat depthFormat);

        virtual void setClearColor(const wgpu::Color& clearColor);
        virtual void clearScreen();
        virtual void present();

    protected:

        virtual void setupDevice(wgpu::Device device);
        virtual void deviceLost(bool destroyed);

    public:

        Observer<bool> onCreated;
        Observer<bool> onDeviceLost;

    private:

        wgpu::Instance mInstance;
        wgpu::Surface mSurface;
        wgpu::Device mDevice;
        wgpu::Queue mQueue;
        SwapChain mSwapChain;
    };
}