// physics_manager.hpp

#pragma once

#include <iostream>
#include <vector>
#include <mutex>
#include <PxPhysics.h>
#include <PxPhysicsAPI.h>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "category.hpp"
#include "object.hpp"
#include "id_vec.hpp"
#include "log.hpp"

namespace physx
{
    class PxActor;
    class PxRigidDynamic;
    class PxShape;
    class PxMaterial;
    class PxJoint;
    class PxScene;
    class PxFoundation;
    class PxPhysics;
}

namespace arachne
{
    class cApplication;
    class cGameObject;
    struct sVertexBufferGeometry;
    struct sTransform;

    class cPhysicsAllocator : public physx::PxAllocatorCallback
    {
        virtual void* allocate(size_t size, const char* typeName, const char* filename, int line) override final
        {
            return malloc(size);
        }

        virtual void deallocate(void* ptr) override final
        {
            free(ptr);
        }
    };

    class cPhysicsError : public physx::PxErrorCallback
    {
        virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override final
        {
            Print(message);
        }
    };

    class cPhysicsCPUDispatcher : public physx::PxCpuDispatcher
    {
        virtual void submitTask(physx::PxBaseTask& task) override final
        {
            task.run();
            task.release();
        }

        virtual uint32_t getWorkerCount() const override final
        {
            return 0;
        }
    };

    class cPhysicsSimulationEvent : public physx::PxSimulationEventCallback
    {
		virtual void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override final {}
		virtual void onWake(physx::PxActor** actors, physx::PxU32 count) override final {}
		virtual void onSleep(physx::PxActor** actors, physx::PxU32 count) override final {}
		virtual void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override final {}
		virtual void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override final {}
		virtual void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) override final {}
    };

    class cPhysicsScene : public cObject
    {
        REALWARE_OBJECT(cPhysicsScene)

    public:
        explicit cPhysicsScene(cContext* context, physx::PxScene* scene, physx::PxControllerManager* controllerManager) : cObject(context), _scene(scene), _controllerManager(controllerManager) {}
        ~cPhysicsScene() = default;

        inline physx::PxScene* GetScene() const { return _scene; }
        inline physx::PxControllerManager* GetControllerManager() const { return _controllerManager; }

    private:
        physx::PxScene* _scene = nullptr;
        physx::PxControllerManager* _controllerManager = nullptr;
    };

    class cPhysicsMaterial : public cObject
    {
        REALWARE_OBJECT(cPhysicsMaterial)

    public:
        explicit cPhysicsMaterial(cContext* context, physx::PxMaterial* material) : cObject(context), _material(material) {}
        ~cPhysicsMaterial() = default;

        inline physx::PxMaterial* GetMaterial() const { return _material; }

    private:
        physx::PxMaterial* _material = nullptr;
    };

    class cPhysicsController : public cObject
    {
        REALWARE_OBJECT(cPhysicsController)

    public:
        explicit cPhysicsController(cContext* context, physx::PxController* controller, types::f32 eyeHeight) : cObject(context), _controller(controller), _eyeHeight(eyeHeight) {}
        ~cPhysicsController() = default;

        inline physx::PxController* GetController() const { return _controller; }
        inline types::f32 GetEyeHeight() const { return _eyeHeight; }

    private:
        physx::PxController* _controller = nullptr;
        types::f32 _eyeHeight = 0.0f;
    };

    class cPhysicsActor : public cObject
    {
        REALWARE_OBJECT(cPhysicsActor)

    public:
        explicit cPhysicsActor(cContext* context, cGameObject* gameObject, physx::PxActor* actor, eCategory actorType) : cObject(context), _gameObject(gameObject), _actor(actor), _type(actorType) {}
        ~cPhysicsActor() = default;

        inline cGameObject* GetGameObject() const { return _gameObject; }
        inline physx::PxActor* GetActor() const { return _actor; }
        inline eCategory GetActorType() const { return _type; }

    private:
        cGameObject* _gameObject = nullptr;
        physx::PxActor* _actor = nullptr;
        eCategory _type = eCategory::PHYSICS_ACTOR_DYNAMIC;
    };

    class cPhysics : public cObject
    {
        REALWARE_OBJECT(cPhysics)

    public:
        explicit cPhysics(cContext* context);
        ~cPhysics();

        cPhysicsScene* CreateScene(const std::string& id, const glm::vec3& gravity = glm::vec3(0.0f, -9.81f, 0.0f));
        cPhysicsMaterial* CreateMaterial(const std::string& id, const glm::vec3& params = glm::vec3(0.5f, 0.5f, 0.6f));
        cPhysicsActor* CreateActor(const std::string& id, eCategory staticOrDynamic, eCategory shapeType, const cPhysicsScene* scene, const cPhysicsMaterial* material, types::f32 mass, const sTransform* transform, cGameObject* gameObject);
        cPhysicsController* CreateController(const std::string& id, types::f32 eyeHeight, types::f32 height, types::f32 radius, const sTransform* transform, const glm::vec3& up, const cPhysicsScene* scene, const cPhysicsMaterial* material);
        cPhysicsScene* FindScene(const std::string&);
        cPhysicsMaterial* FindMaterial(const std::string&);
        cPhysicsActor* FindActor(const std::string&);
        cPhysicsController* FindController(const std::string&);
        void DestroyScene(const std::string& id);
        void DestroyMaterial(const std::string& id);
        void DestroyActor(const std::string& id);
        void DestroyController(const std::string& id);

        void MoveController(const cPhysicsController* controller, const glm::vec3& position, types::f32 minStep = 0.001f);
        glm::vec3 GetControllerPosition(const cPhysicsController* controller);

        void Simulate();

    private:
        cPhysicsAllocator* _allocator = nullptr;
        cPhysicsError* _error = nullptr;
        cPhysicsCPUDispatcher* _cpuDispatcher = nullptr;
        cPhysicsSimulationEvent* _simulationEvent = nullptr;
        physx::PxFoundation* _foundation = nullptr;
        physx::PxPhysics* _physics = nullptr;
        std::mutex _mutex;
        cIdVector<cPhysicsScene>* _scenes;
        cIdVector<cPhysicsMaterial>* _materials;
        cIdVector<cPhysicsActor>* _actors;
        cIdVector<cPhysicsController>* _controllers;
    };
}