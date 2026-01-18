// camera_manager.hpp

#pragma once

#include "../../thirdparty/glm/glm/glm.hpp"
#include "category.hpp"
#include "object.hpp"
#include "types.hpp"

namespace arachne
{
    struct sTransform;

    class cCamera : public cObject
    {
        REALWARE_OBJECT(cCamera)

    public:
        explicit cCamera(cContext* context);
        ~cCamera() = default;

        void Update();
        void AddEuler(eCategory angle, types::f32 value);
        void Move(types::f32 value);
        void Strafe(types::f32 value);
        void Lift(types::f32 value);

        inline const glm::mat4& GetViewProjectionMatrix() const { return _viewProjection; }
        inline types::f32 GetMouseSensitivity() const { return _mouseSensitivity; }
        inline types::f32 GetMoveSpeed() const { return _moveSpeed; }

        inline void SetMouseSensitivity(types::f32 value) { _mouseSensitivity = value; }
        inline void SetMoveSpeed(types::f32 value) { _moveSpeed = value; }

    private:
        std::shared_ptr<sTransform> _transform;
        glm::vec3 _euler = glm::vec3(0.0f);
        glm::vec3 _direction = glm::vec3(0.0f);
        glm::mat4 _view = glm::mat4(1.0f);
        glm::mat4 _projection = glm::mat4(1.0f);
        glm::mat4 _viewProjection = glm::mat4(1.0f);
        types::f32 _fov = 60.0f;
        types::f32 _zNear = 0.01f;
        types::f32 _zFar = 100.0f;
        types::f32 _mouseSensitivity = 1.0f;
        types::f32 _moveSpeed = 1.0f;
        types::boolean _isMoving = types::K_FALSE;
        glm::vec2 _cursorPosition = glm::vec2(0.0f);
        glm::vec2 _prevCursorPosition = _cursorPosition;
    };
}