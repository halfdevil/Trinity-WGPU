#pragma once

#include "Core/Application.h"
#include "Graphics/RenderPass.h"

namespace Trinity
{
    class EmptyWindow : public Application
    {
    protected:

        virtual void render() override;

    protected:

        RenderPass mMainPass;
    };
}