// application.hpp

#pragma once

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <windows.h>
#include <chrono>
#include "object.hpp"
#include "../../thirdparty/glm/glm/glm.hpp"
#include "types.hpp"

namespace arachne
{
    class iRenderContext;
    class iSoundContext;
    class cRenderer;
    class cFontManager;
    class mCamera;
    class mGameObject;
    class mRender;
    class mTexture;
    class mSound;
    class mFont;
    class mPhysics;
    class mFileSystem;
    class mEvent;
    class mThread;
    class cWindow;
    class cEngine;
    struct sEngineCapabilities;

    class cWindow : public cObject
    {
        REALWARE_OBJECT(cWindow)

        friend void WindowSizeCallback(GLFWwindow* window, int width, int height);

    public:
        explicit cWindow(cContext* context, const std::string& title, types::usize width, types::usize height, types::boolean fullscreen);
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
        types::boolean _fullscreen = types::K_FALSE;
    };

    class iApplication : public cObject
    {
        REALWARE_OBJECT(iApplication)

    public:
        enum class eMouseButton
        {
            LEFT,
            RIGHT,
            MIDDLE
        };

        explicit iApplication(cContext* context, const sEngineCapabilities* capabilities);
        ~iApplication();

        virtual void Setup() = 0;
        virtual void Stop() = 0;

        inline cEngine* GetEngine() const { return _engine; }
        inline cWindow* GetWindow() const { return _window; }
        inline iRenderContext* GetRenderContext() const { return _renderContext; }
        inline iSoundContext* GetSoundContext() const { return _soundContext; }
        inline mCamera* GetCameraManager() const { return _camera; }
        inline mTexture* GetTextureManager() const { return _texture; }
        inline mRender* GetRenderManager() const { return _render; }
        inline mFont* GetFontManager() const { return _font; }
        inline mSound* GetSoundManager() const { return _sound; }
        inline mFileSystem* GetFileSystemManager() const { return _fileSystem; }
        inline mPhysics* GetPhysicsManager() const { return _physics; }
        inline mGameObject* GetGameObjectManager() const { return _gameObject; }
        inline mEvent* GetEventManager() const { return _event; }
        inline mThread* GetThreadManager() const { return _thread; }

    protected:
        iRenderContext* _renderContext = nullptr;
        iSoundContext* _soundContext = nullptr;
        mCamera* _camera = nullptr;
        mRender* _render = nullptr;
        mTexture* _texture = nullptr;
        mFont* _font = nullptr;
        mSound* _sound = nullptr;
        mFileSystem* _fileSystem = nullptr;
        mPhysics* _physics = nullptr;
        mGameObject* _gameObject = nullptr;
        mEvent* _event = nullptr;
        mThread* _thread = nullptr;
        types::f32 _deltaTime = 0.0;
        std::chrono::steady_clock::time_point _timepointLast;
        types::boolean _isFocused = types::K_FALSE;
        glm::vec2 _cursorPosition = glm::vec2(0.0f);
        cEngine* _engine = nullptr;
        cWindow* _window = nullptr;
    };
}