// physics_manager.hpp

#pragma once

#include <iostream>
#include <vector>
#include <mutex>
#include <PxPhysics.h>
#include <PxPhysicsAPI.h>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "category.hpp"
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

namespace realware
{
    namespace app
    {
        class cApplication;
    }

    namespace render
    {
        struct sVertexBufferGeometry;
        struct sTransform;
    }

    namespace game
    {
        class cGameObject;
    }

    namespace physics
    {
        class cAllocator : public physx::PxAllocatorCallback
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

        class cError : public physx::PxErrorCallback
        {
            virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override final
            {
                log::Print(message);
            }
        };

        class cCPUDispatcher : public physx::PxCpuDispatcher
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

        class cSimulationEvent : public physx::PxSimulationEventCallback
        {
			virtual void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override final {}
			virtual void onWake(physx::PxActor** actors, physx::PxU32 count) override final {}
			virtual void onSleep(physx::PxActor** actors, physx::PxU32 count) override final {}
			virtual void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override final {}
			virtual void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override final {}
			virtual void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) override final {}
        };

        class cSimulationScene : public cIdVecObject
        {
        public:
            explicit cSimulationScene(const std::string& id, const app::cApplication* const app, const physx::PxScene* const scene, const physx::PxControllerManager* const controllerManager) : cIdVecObject(id, app), _scene((physx::PxScene*)scene), _controllerManager((physx::PxControllerManager*)controllerManager) {}
            ~cSimulationScene() = default;

            inline physx::PxScene* GetScene() const { return _scene; }
            inline physx::PxControllerManager* GetControllerManager() const { return _controllerManager; }

        private:
            physx::PxScene* _scene = nullptr;
            physx::PxControllerManager* _controllerManager = nullptr;
        };

        class cSubstance : public cIdVecObject
        {
        public:
            explicit cSubstance(const std::string& id, const app::cApplication* const app, const physx::PxMaterial* const substance) : cIdVecObject(id, app), _substance((physx::PxMaterial*)substance) {}
            ~cSubstance() = default;

            inline physx::PxMaterial* GetSubstance() const { return _substance; }

        private:
            physx::PxMaterial* _substance = nullptr;
        };

        class cController : public cIdVecObject
        {
        public:
            explicit cController(const std::string& id, const app::cApplication* const app, const physx::PxController* const controller, const types::f32 eyeHeight) : cIdVecObject(id, app), _controller((physx::PxController*)controller), _eyeHeight(eyeHeight) {}
            ~cController() = default;

            inline physx::PxController* GetController() const { return _controller; }
            inline types::f32 GetEyeHeight() const { return _eyeHeight; }

        private:
            physx::PxController* _controller = nullptr;
            types::f32 _eyeHeight = 0.0f;
        };

        class cActor : public cIdVecObject
        {
        public:
            explicit cActor(const std::string& id, const app::cApplication* const app, const game::cGameObject* const gameObject, const physx::PxActor* const actor, const game::Category& actorType) : cIdVecObject(id, app), _gameObject((game::cGameObject*)gameObject), _actor((physx::PxActor*)actor), _type(actorType) {}
            ~cActor() = default;

            inline game::cGameObject* GetGameObject() const { return _gameObject; }
            inline physx::PxActor* GetActor() const { return _actor; }
            inline game::Category GetType() const { return _type; }

        private:
            game::cGameObject* _gameObject = nullptr;
            physx::PxActor* _actor = nullptr;
            game::Category _type = game::Category::PHYSICS_ACTOR_DYNAMIC;
        };

        class mPhysics
        {
        public:
            explicit mPhysics(const app::cApplication* const app);
            ~mPhysics();

            cSimulationScene* CreateScene(const std::string& id, const glm::vec3& gravity = glm::vec3(0.0f, -9.81f, 0.0f));
            cSubstance* CreateSubstance(const std::string& id, const glm::vec3& params = glm::vec3(0.5f, 0.5f, 0.6f));
            cActor* CreateActor(const std::string& id, const game::Category& staticOrDynamic, const game::Category& shapeType, const cSimulationScene* const scene, const cSubstance* const substance, const types::f32 mass, const render::sTransform* const transform, const game::cGameObject* const gameObject);
            cController* CreateController(const std::string& id, const types::f32 eyeHeight, const types::f32 height, const types::f32 radius, const render::sTransform* const transform, const glm::vec3& up, const cSimulationScene* const scene, const cSubstance* const substance);
            cSimulationScene* FindScene(const std::string&);
            cSubstance* FindSubstance(const std::string&);
            cActor* FindActor(const std::string&);
            cController* FindController(const std::string&);
            void DestroyScene(const std::string& id);
            void DestroySubstance(const std::string& id);
            void DestroyActor(const std::string& id);
            void DestroyController(const std::string& id);

            void MoveController(const cController* const controller, const glm::vec3& position, const types::f32 minStep = 0.001f);
            glm::vec3 GetControllerPosition(const cController* const controller);

            void Simulate();

        private:
            app::cApplication* _app = nullptr;
            cAllocator* _allocator = nullptr;
            cError* _error = nullptr;
            cCPUDispatcher* _cpuDispatcher = nullptr;
            cSimulationEvent* _simulationEvent = nullptr;
            physx::PxFoundation* _foundation = nullptr;
            physx::PxPhysics* _physics = nullptr;
            std::mutex _mutex;
            cIdVec<cSimulationScene> _scenes;
            cIdVec<cSubstance> _substances;
            cIdVec<cActor> _actors;
            cIdVec<cController> _controllers;
        };
    }
}