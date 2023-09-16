#include "EmptyWindow.h"
#include <glm/glm.hpp>

namespace Trinity
{
    void EmptyWindow::render()
    {
        mMainPass.begin();
        mMainPass.end();
    }
}

int main(int argc, char* argv[])
{
    static Trinity::EmptyWindow app;
    app.run("Trinity - Empty Window");

    return 0;
}