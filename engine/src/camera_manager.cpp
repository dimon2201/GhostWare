// camera_manager.cpp

#include "camera_manager.hpp"
#include "physics_manager.hpp"
#include "graphics.hpp"
#include "render_context.hpp"
#include "application.hpp"
#include "gameobject_manager.hpp"
#include "context.hpp"
#include "time.hpp"
#include "input.hpp"
#include "math.hpp"
#include "engine.hpp"

using namespace types;

namespace triton
{
    cCamera::cCamera(cContext* context) : cComponent(context) {}

    void cCamera::Update()
    {
        const cInput* input = _context->GetSubsystem<cInput>();
        const cTime* time = _context->GetSubsystem<cTime>();
        const cMath* math = _context->GetSubsystem<cMath>();
        const iApplication* app = _context->GetSubsystem<cEngine>()->GetApplication();

        const f32 deltaTime = time->GetDeltaTime();
        const cWindow* window = app->GetWindow();

        if (_euler.GetX() > math->DegreesToRadians(65.0f))
            _euler.SetX(math->DegreesToRadians(65.0f));
        else if (_euler.GetX() < math->DegreesToRadians(-65.0f))
            _euler.SetX(math->DegreesToRadians(-65.0f));

        const cQuaternion quatX = cQuaternion(_euler.GetX(), cVector3(1.0f, 0.0f, 0.0f));
        const cQuaternion quatY = cQuaternion(_euler.GetY(), cVector3(0.0f, 1.0f, 0.0f));
        const cQuaternion quatZ = cQuaternion(_euler.GetZ(), cVector3(0.0f, 0.0f, 1.0f));
        _direction = quatZ * quatY * quatX * cVector3(0.0f, 0.0f, -1.0f);

        _view = cMatrix4(_transform.GetPosition(), _transform.GetPosition() + _direction, cVector3(0.0f, 1.0f, 0.0f));
        _projection = cMatrix4(cMath::DegreesToRadians(_fov), (f32)window->GetWidth() / window->GetHeight(), _zNear, _zFar);
        _viewProjection = _projection * _view;

        _prevCursorPosition = _cursorPosition;
        _cursorPosition = window->GetCursorPosition();

        const cVector2 mouseDelta = _prevCursorPosition - _cursorPosition;
        AddEuler(eCategory::CAMERA_ANGLE_PITCH, mouseDelta.GetY() * _mouseSensitivity * deltaTime);
        AddEuler(eCategory::CAMERA_ANGLE_YAW, mouseDelta.GetX() * _mouseSensitivity * deltaTime);
        
        const f32 forward = input->GetKey('W') * _moveSpeed * deltaTime;
        const f32 backward = input->GetKey('S') * _moveSpeed * deltaTime;
        const f32 left = input->GetKey('A') * _moveSpeed * deltaTime;
        const f32 right = input->GetKey('D') * _moveSpeed * deltaTime;
        if (forward > 0.0f || backward > 0.0f || left > 0.0f || right > 0.0f)
        {
            Move(forward);
            Move(-backward);
            Strafe(-left);
            Strafe(right);

            _isMoving = K_TRUE;
        }
        else
        {
            _isMoving = K_FALSE;
        }
    }

    void cCamera::AddEuler(eCategory angle, f32 value)
    {
        if (angle == eCategory::CAMERA_ANGLE_PITCH)
            _euler.AddX(value);
        else if (angle == eCategory::CAMERA_ANGLE_YAW)
            _euler.AddX(value);
        else if (angle == eCategory::CAMERA_ANGLE_ROLL)
            _euler.AddX(value);
    }

    void cCamera::Move(f32 value)
    {
        cPhysics* physics = _context->GetSubsystem<cPhysics>();
        const cPhysicsController* controller = _cameraGameObject->GetPhysicsController();
        const cTransform* transform = _cameraGameObject->GetTransform();
        const cVector3& transformPosition = transform->GetPosition();
        const cVector3 position = transformPosition;
        const cVector3 newPosition = transformPosition + _direction * value;
        
        physics->MoveController(
            controller,
            newPosition - position
        );
        const cVector3 cameraPosition = physics->GetControllerPosition(controller);

        _cameraGameObject->GetTransform()->_position = cameraPosition;
    }

    void cCamera::Strafe(f32 value)
    {
        cPhysics* physics = _context->GetSubsystem<cPhysics>();
        const cPhysicsController* controller = _cameraGameObject->GetPhysicsController();
        const cTransform* transform = _cameraGameObject->GetTransform();
        const cVector3& transformPosition = transform->GetPosition();
        const cVector3 right = _direction.Cross(cVector3(0.0f, 1.0f, 0.0f));
        const cVector3 position = transformPosition;
        const cVector3 newPosition = transformPosition + right * value;

        physics->MoveController(
            controller,
            newPosition - position
        );
        const cVector3 cameraPosition = physics->GetControllerPosition(controller);

        _cameraGameObject->GetTransform()->_position = cameraPosition;
    }

    void cCamera::Lift(f32 value)
    {
        cPhysics* physics = _context->GetSubsystem<cPhysics>();
        const cPhysicsController* controller = _cameraGameObject->GetPhysicsController();
        const cTransform* transform = _cameraGameObject->GetTransform();
        const cVector3& transformPosition = transform->GetPosition();
        const cVector3 position = transformPosition;
        const cVector3 newPosition = transformPosition + cVector3(0.0f, 1.0f, 0.0f) * value;

        physics->MoveController(
            controller,
            newPosition - position
        );
        const cVector3 cameraPosition = physics->GetControllerPosition(controller);

        _cameraGameObject->GetTransform()->_position = cameraPosition;
    }
}