// input.cpp

#include <windows.h>
#include <GLFW/glfw3.h>
#include "input.hpp"
#include "context.hpp"
#include "graphics.hpp"

using namespace types;

namespace realware
{
    void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        const cpuword keyBufferMask = 0xFF;
        key &= keyBufferMask;

        cContext* context = (cContext*)glfwGetWindowUserPointer(window);
        cInput* input = context->GetSubsystem<cInput>();

        if (action == GLFW_PRESS)
            input->SetKey(key, K_TRUE);
        else if (action == GLFW_RELEASE)
            input->SetKey(key, K_FALSE);
    }

    void WindowFocusCallback(GLFWwindow* window, int focused)
    {
        cContext* context = (cContext*)glfwGetWindowUserPointer(window);
        cInput* input = context->GetSubsystem<cInput>();

        if (focused)
        {
            if (input->GetWindowFocus() == K_FALSE)
                input->SetWindowFocus(K_TRUE);
        }
        else
        {
            input->SetWindowFocus(K_FALSE);
        }
    }

    void WindowSizeCallback(GLFWwindow* window, int width, int height)
    {
        cContext* context = (cContext*)glfwGetWindowUserPointer(window);
        cInput* input = context->GetSubsystem<cInput>();
        cGraphics* gfx = context->GetSubsystem<cGraphics>();

        input->_window->_width = width;
        input->_window->_height = height;

        gfx->ResizeRenderTargets(glm::vec2(width, height));
    }

    void CursorCallback(GLFWwindow* window, double xpos, double ypos)
    {
        cContext* context = (cContext*)glfwGetWindowUserPointer(window);
        cInput* input = context->GetSubsystem<cInput>();

        input->SetCursorPosition(glm::vec2(xpos, ypos));
    }

    void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        cContext* context = (cContext*)glfwGetWindowUserPointer(window);
        cInput* input = context->GetSubsystem<cInput>();

        if (action == GLFW_RELEASE)
            input->SetMouseKey(button, 0);
        else if (action == GLFW_PRESS)
            input->SetMouseKey(button, 1);
    }

    cWindow::cWindow(cContext* context, const std::string& title, types::usize width, types::usize height) : iObject(context), _title(title)
    {
        if (context->GetSubsystem<cInput>()->_initialized == K_FALSE)
            return;

        glm::vec2 windowSize = glm::vec2(width, height);

        if (fullscreen == K_FALSE)
        {
            _window = glfwCreateWindow(windowSize.x, windowSize.y, _title.c_str(), nullptr, nullptr);
        }
        else
        {
            glfwWindowHint(GLFW_DECORATED, 0);

            windowSize = GetMonitorSize();
            _window = glfwCreateWindow(windowSize.x, windowSize.y, _title.c_str(), glfwGetPrimaryMonitor(), nullptr);
        }

        if (!_window)
        {
            Print("Error: incompatible GL version!");
            return;
        }

        _width = windowSize.x;
        _height = windowSize.y;

        glfwSetWindowUserPointer(_window, _context);

        glfwMakeContextCurrent(_window);

        glfwSwapInterval(1);

        glfwSetKeyCallback(_window, &KeyCallback);
        glfwSetWindowFocusCallback(_window, &WindowFocusCallback);
        glfwSetWindowSizeCallback(_window, &WindowSizeCallback);
        glfwSetCursorPosCallback(_window, &CursorCallback);
        glfwSetMouseButtonCallback(_window, &MouseButtonCallback);
    }

    types::boolean cWindow::GetRunState() const
    {
        return glfwWindowShouldClose(_window);
    }

    HWND cWindow::GetWin32Window() const
    {
        return glfwGetWin32Window(_window);
    }
    
    cInput::cInput(cContext* context) : iObject(context)
    {
        if (_initialized == types::K_FALSE)
        {
            _initialized = types::K_TRUE;

            glfwInit();
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        }
    }

    void cInput::SwapBuffers()
    {
        glfwSwapBuffers(_window->GetWindow());
    }

	void cInput::PollEvents()
	{
		glfwPollEvents();
	}

    glm::vec2 cInput::GetMonitorSize() const
    {
        return glm::vec2(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
    }
}