#pragma once

#include "SampleApplication.h"
#include "Graphics/RenderPass.h"

namespace Trinity
{
    class EmptyWindow : public SampleApplication
    {
    protected:

        virtual void render(float deltaTime) override;

    protected:

        RenderPass mMainPass;
    };
}