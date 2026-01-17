// input.hpp

#pragma once

#include <string>
#include <windows.h>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "object.hpp"
#include "types.hpp"

struct GLFWwindow;

namespace realware
{
	class cContext;

    class cWindow : public iObject
    {
        REALWARE_CLASS(cWindow)

        friend void WindowSizeCallback(GLFWwindow* window, int width, int height);

    public:
        explicit cWindow(cContext* context, const std::string& title, types::usize width, types::usize height);
        virtual ~cWindow() override final = default;

        types::boolean GetRunState() const;
        HWND GetWin32Window() const;
        inline GLFWwindow* GetWindow() const { return _window; }
        inline const std::string& GetTitle() const { return _title; }
        inline glm::vec2 GetSize() const { return glm::vec2(_width, _height); }
        inline types::usize GetWidth() const { return _width; }
        inline types::usize GetHeight() const { return _height; }

    private:
        GLFWwindow* _window = nullptr;
        std::string _title = "";
        types::usize _width = 0;
        types::usize _height = 0;
    };

	class cInput : public iObject
	{
        REALWARE_CLASS(cInput)

        friend class cWindow;
        friend void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        friend void WindowFocusCallback(GLFWwindow* window, int focused);
        friend void WindowSizeCallback(GLFWwindow* window, int width, int height);
        friend void CursorCallback(GLFWwindow* window, double xpos, double ypos);
        friend void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

	public:
		explicit cInput(cContext* context);
		virtual ~cInput() override final = default;

        void SwapBuffers();
		void PollEvents();

        glm::vec2 GetMonitorSize() const;
        inline types::boolean GetKey(int key) const { return _keys[key]; }
        inline types::boolean GetMouseKey(int key) const { return _mouseKeys[key]; }
        inline types::boolean GetWindowFocus() const { return _isFocused; }

    private:
        static constexpr types::usize K_MAX_KEY_COUNT = 256;

        inline void SetKey(const int key, types::boolean value) { _keys[key] = value; }
        inline void SetMouseKey(const int key, types::boolean value) { _mouseKeys[key] = value; }
        inline void SetWindowFocus(types::boolean value) { _isFocused = value; }
        inline void SetCursorPosition(const glm::vec2& cursorPosition) { _cursorPosition = cursorPosition; }

    private:
        types::boolean _initialized = types::K_FALSE;
        types::s32 _keys[K_MAX_KEY_COUNT] = {};
        types::s32 _mouseKeys[3] = {};
        types::boolean _isFocused = types::K_FALSE;
        glm::vec2 _cursorPosition = glm::vec2(0.0f);
	};
}