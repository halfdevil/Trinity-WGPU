#include "EmptyWindow.h"
#include "Core/Logger.h"
#include "Core/Debugger.h"
#include "Core/Clock.h"
#include "Core/Window.h"
#include "VFS/FileSystem.h"
#include "Input/Input.h"
#include "Graphics/GraphicsDevice.h"
#include "Graphics/SwapChain.h"
#include "Scene/Scene.h"
#include <glm/glm.hpp>

namespace Trinity
{
    void EmptyWindow::render(float deltaTime)
    {
        mMainPass.begin();
        mMainPass.end();
    }
}

int main(int argc, char* argv[])
{
    static Trinity::EmptyWindow app;
    app.run({
        .title = "Trinity - EmptyWindow"
    });

    return 0;
}