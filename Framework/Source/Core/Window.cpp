#include "Core/Window.h"
#include "Core/Logger.h"
#include "Core/Debugger.h"

#ifndef __EMSCRIPTEN__
#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include <GLFW/glfw3native.h>
#endif

namespace Trinity
{
    Window::~Window()
    {
        destroy();
    }

    bool Window::create(const std::string& title, uint32_t width, uint32_t height, DisplayMode displayMode)
    {
        mWidth = width;
        mHeight = height;
        mDisplayMode = displayMode;

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* videoMode = glfwGetVideoMode(monitor);

        if (mDisplayMode == DisplayMode::Borderless)
        {
            mWidth = videoMode->width;
            mHeight = videoMode->height;
        }

        monitor = (mDisplayMode != DisplayMode::Windowed) ? monitor : nullptr;
        mHandle = glfwCreateWindow(mWidth, mHeight, title.c_str(), monitor, nullptr);

        if (!mHandle)
        {
            LogError("glfwCreateWindow() failed!!");
            return false;
        }

        createCursors();

        glfwGetFramebufferSize(mHandle, (int32_t*)&mWidth, (int32_t*)&mHeight);
        glfwSetWindowUserPointer(mHandle, this);

        glfwSetWindowCloseCallback(mHandle, [](GLFWwindow* handle) {
            Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(handle));
            if (window)
            {
                window->mCallbacks.onClose.notify();
            }
        });

        glfwSetFramebufferSizeCallback(mHandle, [](GLFWwindow* handle, int width, int height) {
            Window* window = reinterpret_cast<Window*>(glfwGetWindowUserPointer(handle));
            if (window)
            {
                window->mWidth = static_cast<uint32_t>(width);
                window->mHeight = static_cast<uint32_t>(height);
                window->mCallbacks.onResize.notify(window->mWidth, window->mHeight);
            }
        });

        glfwSetWindowPosCallback(mHandle, [](GLFWwindow* handle, int x, int y) {
            Window* window = static_cast<Window*>(glfwGetWindowUserPointer(handle));
            if (window)
            {
                window->mCallbacks.onPos.notify(x, y);
            }
        });

        glfwSetWindowFocusCallback(mHandle, [](GLFWwindow* handle, int focused) {
            Window* window = static_cast<Window*>(glfwGetWindowUserPointer(handle));
            if (window)
            {
                window->mCallbacks.onFocus.notify(focused != 0);
            }
        });

        glfwSetKeyCallback(mHandle, [](GLFWwindow* handle, int key, int scancode, int action, int mods) {
            Window* window = static_cast<Window*>(glfwGetWindowUserPointer(handle));
            if (window && action != GLFW_REPEAT)
            {
                window->mCallbacks.onKey.notify(key, action == GLFW_PRESS, mods);
            }
        });

        glfwSetCharCallback(mHandle, [](GLFWwindow* handle, unsigned int c) {
            Window* window = static_cast<Window*>(glfwGetWindowUserPointer(handle));
            if (window)
            {
                window->mCallbacks.onChar.notify(c);
            }
        });

        glfwSetCursorEnterCallback(mHandle, [](GLFWwindow* handle, int entered) {
            Window* window = static_cast<Window*>(glfwGetWindowUserPointer(handle));
            if (window)
            {
                window->mCallbacks.onMouseEnter.notify(entered != 0);
            }
        });

        glfwSetCursorPosCallback(mHandle, [](GLFWwindow* handle, double x, double y) {
            Window* window = static_cast<Window*>(glfwGetWindowUserPointer(handle));
            if (window)
            {
                window->mCallbacks.onMousePos.notify((float)x, (float)y);
            }
        });

        glfwSetMouseButtonCallback(mHandle, [](GLFWwindow* handle, int button, int action, int mods) {
            Window* window = static_cast<Window*>(glfwGetWindowUserPointer(handle));
            if (window && action != GLFW_REPEAT)
            {
                window->mCallbacks.onMouseButton.notify(button, action == GLFW_PRESS, mods);
            }
        });

        glfwSetScrollCallback(mHandle, [](GLFWwindow* handle, double xoffset, double yoffset) {
            Window* window = static_cast<Window*>(glfwGetWindowUserPointer(handle));
            if (window)
            {
                window->mCallbacks.onMouseScroll.notify((float)xoffset, (float)yoffset);
            }
        });

        return true;
    }

    void Window::destroy()
    {
        if (mHandle)
        {
            glfwDestroyWindow(mHandle);
            mHandle = nullptr;
        }
    }

    bool Window::isClosed() const
    {
        return glfwWindowShouldClose(mHandle);
    }

    void Window::showMouse(bool visible, bool lock) const
    {
        if (lock)
        {
            glfwSetInputMode(mHandle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
#ifndef __EMSCRIPTEN__			
            if (glfwRawMouseMotionSupported())
            {
                glfwSetInputMode(mHandle, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
            }
#endif
        }
        else
        {
            glfwSetInputMode(mHandle, GLFW_CURSOR, visible ? GLFW_CURSOR_NORMAL :
                GLFW_CURSOR_HIDDEN);
        }
    }

    void Window::show(bool visible) const
    {
        if (visible)
        {
            glfwShowWindow(mHandle);
        }
        else
        {
            glfwHideWindow(mHandle);
        }
    }

    void Window::close()
    {
        glfwSetWindowShouldClose(mHandle, GLFW_TRUE);
    }

    void Window::poll()
    {
        glfwPollEvents();
    }

    void Window::setCursor(Cursor cursor) const
    {
        GLFWcursor* cursorHandle = mCursors[(uint32_t)cursor] ? mCursors[(uint32_t)cursor] :
            mCursors[(uint32_t)Cursor::Arrow];

        glfwSetCursor(mHandle, cursorHandle);
    }

    glm::uvec2 Window::getSize() const
    {
        int32_t x{ 0 };
        int32_t y{ 0 };

        glfwGetWindowSize(mHandle, &x, &y);
        return { (uint32_t)x, (uint32_t)y };
    }

    glm::uvec2 Window::getFramebufferSize() const
    {
        int32_t x{ 0 };
        int32_t y{ 0 };

        glfwGetFramebufferSize(mHandle, &x, &y);
        return { (uint32_t)x, (uint32_t)y };
    }

    std::unique_ptr<wgpu::ChainedStruct> Window::getSurfaceDescriptor() const
    {
#ifdef __EMSCRIPTEN__
        static constexpr const char* selector = "#canvas";
        auto surfaceDesc = std::make_unique<wgpu::SurfaceDescriptorFromCanvasHTMLSelector>();
        surfaceDesc->selector = selector;
        return surfaceDesc;
#else
#ifdef _WIN32
        auto surfaceDesc = std::make_unique<wgpu::SurfaceDescriptorFromWindowsHWND>();
        surfaceDesc->hwnd = glfwGetWin32Window(mHandle);
        surfaceDesc->hinstance = GetModuleHandle(nullptr);
        return surfaceDesc;
#endif
#endif
    }

    void Window::createCursors()
    {
        mCursors.resize((uint32_t)Cursor::Max);

        mCursors[(uint32_t)Cursor::Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        mCursors[(uint32_t)Cursor::IBeam] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
        mCursors[(uint32_t)Cursor::VResize] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
        mCursors[(uint32_t)Cursor::HResize] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
        mCursors[(uint32_t)Cursor::Hand] = glfwCreateStandardCursor(GLFW_HAND_CURSOR);

#ifndef __EMSCRIPTEN__
        mCursors[(uint32_t)Cursor::ResizeAll] = glfwCreateStandardCursor(GLFW_RESIZE_ALL_CURSOR);
        mCursors[(uint32_t)Cursor::ResizeNESW] = glfwCreateStandardCursor(GLFW_RESIZE_NESW_CURSOR);
        mCursors[(uint32_t)Cursor::ResizeNWSE] = glfwCreateStandardCursor(GLFW_RESIZE_NWSE_CURSOR);
        mCursors[(uint32_t)Cursor::NotAllowed] = glfwCreateStandardCursor(GLFW_NOT_ALLOWED_CURSOR);
#else
        mCursors[(uint32_t)Cursor::ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        mCursors[(uint32_t)Cursor::ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        mCursors[(uint32_t)Cursor::ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
        mCursors[(uint32_t)Cursor::NotAllowed] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
#endif
    }

    bool Window::initialize()
    {
        glfwSetErrorCallback([](int code, const char* description) {
            LogError("GLFW error (%d) - %s", code, description);
        });

        if (!glfwInit())
        {
            LogError("glfwInit() failed!!");
            return false;
        }

        glfwWindowHint(GLFW_VISIBLE, false);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        return true;
    }
}