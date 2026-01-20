// application.hpp

#pragma once

#include <windows.h>
#include <chrono>
#include "object.hpp"
#include "../../thirdparty/glm/glm/glm.hpp"
#include "types.hpp"

struct GLFWwindow;

namespace harpy
{
    class cEngine;

    class cWindow : public iObject
    {
        HARPY_OBJECT(cWindow)

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

    struct sApplicationCapabilities
    {
        std::string windowTitle = "Test app";
        types::usize windowWidth = 640;
        types::usize windowHeight = 480;
        types::boolean fullscreen = types::K_FALSE;
        types::usize memoryAlignment = 64;
        types::usize maxPhysicsSceneCount = 16;
        types::usize maxPhysicsMaterialCount = 256;
        types::usize maxPhysicsActorCount = 8192;
        types::usize maxPhysicsControllerCount = 8;
        types::usize maxSoundCount = 65536;
        types::usize maxEventPerTypeCount = 8192;
        types::usize maxRenderOpaqueInstanceCount = 65536;
        types::usize maxRenderTransparentInstanceCount = 65536;
        types::usize maxRenderTextInstanceCount = 8192;
        types::usize maxRenderMaterialCount = 256;
        types::usize maxRenderLightCount = 256;
        types::usize maxRenderTextureAtlasTextureCount = 8192;
        types::usize vertexBufferSize = 64 * 1024 * 1024;
        types::usize indexBufferSize = 64 * 1024 * 1024;

    };

    class iApplication : public iObject
    {
        HARPY_OBJECT(iApplication)

    public:
        enum class eMouseButton
        {
            LEFT,
            RIGHT,
            MIDDLE
        };

        explicit iApplication(cContext* context, const sApplicationCapabilities* caps);
        ~iApplication();

        virtual void Setup() = 0;
        virtual void Stop() = 0;

        inline const sApplicationCapabilities* GetCapabilities() const { return _caps; }
        inline cEngine* GetEngine() const { return _engine; }
        inline cWindow* GetWindow() const { return _window; }

    protected:
        const sApplicationCapabilities* _caps = nullptr;
        cEngine* _engine = nullptr;
        cWindow* _window = nullptr;
    };
}