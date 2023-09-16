#pragma once

#include "Graphics/SwapChain.h"
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

        const wgpu::CommandEncoder& getEncoder() const
        {
            return mEncoder;
        }

        const SwapChain& getSwapChain() const
        {
            return mSwapChain;
        }

        operator const wgpu::Device& () const
        {
            return mDevice;
        }

        void create(const Window& window);
        void destroy();

        void beginFrame();
        void endFrame();

        bool setupSwapChain(const Window& window, wgpu::PresentMode presentMode,
            wgpu::TextureFormat depthFormat);

        void setClearColor(const wgpu::Color& clearColor);
        void present();

    protected:

        void setupDevice(wgpu::Device device);
        void deviceLost(bool destroyed);

    public:

        Observer<bool> onCreated;
        Observer<bool> onDeviceLost;

    private:

        wgpu::Instance mInstance;
        wgpu::Surface mSurface;
        wgpu::Device mDevice;
        wgpu::Queue mQueue;
        wgpu::CommandEncoder mEncoder;
        SwapChain mSwapChain;
    };
}