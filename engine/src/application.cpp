// application.cpp

#include <iostream>
#include <GLFW/glfw3.h>
#include "application.hpp"
#include "engine.hpp"
#include "graphics.hpp"
#include "context.hpp"
#include "camera_manager.hpp"
#include "render_manager.hpp"
#include "render_context.hpp"
#include "sound_context.hpp"
#include "font_manager.hpp"
#include "sound_manager.hpp"
#include "filesystem_manager.hpp"
#include "physics_manager.hpp"
#include "gameobject_manager.hpp"
#include "texture_manager.hpp"
#include "memory_pool.hpp"
#include "event_manager.hpp"
#include "thread_manager.hpp"
#include "input.hpp"
#include "time.hpp"
#include "log.hpp"

using namespace types;

namespace arachne
{
    cWindow::cWindow(cContext* context, const std::string& title, types::usize width, types::usize height, types::boolean fullscreen) : cObject(context), _title(title), _fullscreen(fullscreen)
    {
        cInput* input = context->GetSubsystem<cInput>();

        if (input->_initialized == K_FALSE)
            return;

        glm::vec2 windowSize = glm::vec2(width, height);

        if (fullscreen == K_FALSE)
        {
            _window = glfwCreateWindow(windowSize.x, windowSize.y, _title.c_str(), nullptr, nullptr);
        }
        else
        {
            glfwWindowHint(GLFW_DECORATED, 0);

            windowSize = input->GetMonitorSize();
            _window = glfwCreateWindow(windowSize.x, windowSize.y, _title.c_str(), glfwGetPrimaryMonitor(), nullptr);
        }

        if (!_window)
        {
            Print("Error: incompatible GL version!");
            return;
        }

        _width = windowSize.x;
        _height = windowSize.y;
        _fullscreen = fullscreen;

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

    iApplication::iApplication(cContext* context, const sEngineCapabilities* capabilities) : cObject(context)
    {
        _engine = _context->Create<cEngine>(_context, capabilities, this);
        _window = _context->Create<cWindow>(_context, capabilities->windowTitle, capabilities->windowWidth, capabilities->windowHeight, capabilities->fullscreen);
    }

    iApplication::~iApplication()
    {
        _context->Destroy<cWindow>(_window);
        _context->Destroy<cEngine>(_engine);
    }
}