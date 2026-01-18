// gameobject_manager.hpp

#pragma once

#include <vector>
#include <string>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "category.hpp"
#include "id_vec.hpp"
#include "object.hpp"
#include "types.hpp"

namespace arachne
{
    class mPhysics;
    class cPhysicsController;
    class cPhysicsActor;
    class cPhysicsSimulationScene;
    class cPhysicsSubstance;
    class cApplication;
    class cMaterial;
    struct sVertexBufferGeometry;
    struct sLight;
    struct sTransform;
    struct sText;
    
    class cGameObject : public cObject
    {
        REALWARE_OBJECT(cGameObject)

        friend class mGameObject;

    public:
        explicit cGameObject(cContext* context);
        ~cGameObject() = default;

        inline types::boolean GetVisible() const { return _isVisible; }
        inline types::boolean GetOpaque() const { return _isOpaque; }
        inline sVertexBufferGeometry* GetGeometry() const { return _geometry; }
        inline types::boolean GetIs2D() const { return _is2D; }
        inline const glm::mat4& GetWorldMatrix() const { return _world; }
        inline const glm::mat4& GetViewProjectionMatrix() const { return _viewProjection; }
        inline sTransform* GetTransform() const { return _transform; }
        inline cMaterial* GetMaterial() const { return _material; }
        inline sText* GetText() const { return _text; }
        inline sLight* GetLight() const { return _light; }
        inline cPhysicsActor* GetPhysicsActor() const { return _actor; }
        inline cPhysicsController* GetPhysicsController() const { return _controller; }

        inline void SetVisible(types::boolean isVisible) { _isVisible = isVisible; }
        inline void SetOpaque(types::boolean isOpaque) { _isOpaque = isOpaque; }
        inline void SetGeometry(sVertexBufferGeometry* geometry) { _geometry = geometry; }
        inline void SetIs2D(types::boolean is2D) { _is2D = is2D; }
        inline void SetWorldMatrix(const glm::mat4& world) { _world = world; }
        inline void SetViewProjectionMatrix(const glm::mat4& viewProjection) { _viewProjection = viewProjection; }
        inline void SetMaterial(cMaterial* material) { _material = material; }
        inline void SetText(sText* text) { _text = text; }
        inline void SetLight(sLight* light) { _light = light; }
        void SetPhysicsActor(eCategory staticOrDynamic, eCategory shapeType, cPhysicsSimulationScene* scene, cPhysicsSubstance* substance, types::f32 mass);
        void SetPhysicsController(types::f32 eyeHeight, types::f32 height, types::f32 radius, const glm::vec3& up, cPhysicsSimulationScene* scene, cPhysicsSubstance* substance);

    private:
        types::boolean _isVisible = types::K_TRUE;
        types::boolean _isOpaque = types::K_TRUE;
        sVertexBufferGeometry* _geometry = nullptr;
        types::boolean _is2D = types::K_FALSE;
        glm::mat4 _world = glm::mat4(1.0f);
        glm::mat4 _viewProjection = glm::mat4(1.0f);
        sTransform* _transform = nullptr;
        cMaterial* _material = nullptr;
        sText* _text = nullptr;
        sLight* _light = nullptr;
        cPhysicsActor* _actor = nullptr;
        cPhysicsController* _controller = nullptr;
    };

    class mGameObject : public cObject
    {
    public:
        explicit mGameObject(cContext* context);
        ~mGameObject() = default;

        cGameObject* CreateGameObject(const std::string& id);
        cGameObject* FindGameObject(const std::string& id);
        void DestroyGameObject(const std::string& id);

        inline cIdVector<cGameObject>& GetObjects() const { return _gameObjects; }

    private:
        types::usize _maxGameObjectCount = 0;
        types::usize _gameObjectCount = 0;
        mutable cIdVector<cGameObject> _gameObjects;
    };
}