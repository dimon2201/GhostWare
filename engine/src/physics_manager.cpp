// physics_manager.cpp

#include <windows.h>
#include <cooking/PxCooking.h>
#include "../../thirdparty/glm/glm/gtc/quaternion.hpp"
#include "application.hpp"
#include "context.hpp"
#include "time.hpp"
#include "graphics.hpp"
#include "render_manager.hpp"
#include "physics_manager.hpp"
#include "gameobject_manager.hpp"

using namespace physx;
using namespace types;

namespace arachne
{
    PxFilterFlags FilterShader(
        PxFilterObjectAttributes attributes0, PxFilterData filterData0,
        PxFilterObjectAttributes attributes1, PxFilterData filterData1,
        PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
    {
        if (PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
        {
            pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
            return PxFilterFlag::eDEFAULT;
        }
        pairFlags = PxPairFlag::eCONTACT_DEFAULT;

        if ((filterData0.word0 & filterData1.word0) && (filterData1.word1 & filterData0.word1))
            pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;

        return PxFilterFlag::eDEFAULT;
    }

    cPhysics::cPhysics(cContext* context) :
        cObject(context),
        _allocator(new cPhysicsAllocator()),
        _error(new cPhysicsError()),
        _cpuDispatcher(new cPhysicsCPUDispatcher()),
        _simulationEvent(new cPhysicsSimulationEvent())
    {
        const sEngineCapabilities* capabilities = context->GetSubsystem<cEngine>()->GetCapabilities();
        _scenes = _context->Create<cIdVector<cPhysicsScene>>(context, capabilities->_maxPhysicsSceneCount);
        _materials = _context->Create<cIdVector<cPhysicsMaterial>>(context, capabilities->_maxPhysicsMaterialCount);
        _actors = _context->Create<cIdVector<cPhysicsActor>>(context, capabilities->_maxPhysicsActorCount);
        _controllers = _context->Create<cIdVector<cPhysicsController>>(context, capabilities->_maxPhysicsControllerCount);

        _foundation = PxCreateFoundation(PX_PHYSICS_VERSION, *_allocator, *_error);
        if (_foundation == nullptr)
        {
            MessageBox(0, "Failed to initialize PhysXFoundation!", "Error", MB_ICONERROR);
            return;
        }

        _physics = PxCreatePhysics(PX_PHYSICS_VERSION, *_foundation, PxTolerancesScale(), false, nullptr);
        if (_physics == nullptr)
        {
            MessageBox(0, "Failed to initialize PhysXPhysics!", "Error", MB_ICONERROR);
            return;
        }
    }

    cPhysics::~cPhysics()
    {
        _physics->release();
        _foundation->release();
        _context->Destroy<cIdVector<cPhysicsController>>(_controllers);
        _context->Destroy<cIdVector<cPhysicsActor>>(_actors);
        _context->Destroy<cIdVector<cPhysicsMaterial>>(_materials);
        _context->Destroy<cIdVector<cPhysicsScene>>(_scenes);
        delete _simulationEvent;
        delete _cpuDispatcher;
        delete _error;
        delete _allocator;
    }

    cPhysicsScene* cPhysics::CreateScene(const std::string& id, const glm::vec3& gravity)
    {
        PxSceneDesc sceneDesc(_physics->getTolerancesScale());
        sceneDesc.gravity = PxVec3(gravity.y, gravity.x, gravity.z);
        sceneDesc.cpuDispatcher = _cpuDispatcher;
        sceneDesc.filterShader = PxDefaultSimulationFilterShader;

        PxScene* scene = _physics->createScene(sceneDesc);

        PxControllerManager* controllerManager = PxCreateControllerManager(*scene);

        return _scenes->Add(id, _context, scene, controllerManager);
    }

    cPhysicsMaterial* cPhysics::CreateMaterial(const std::string& id, const glm::vec3& params)
    {
        PxMaterial* material = _physics->createMaterial(params.x, params.y, params.z);

        return _materials->Add(id, _context, material);
    }

    cPhysicsController* cPhysics::CreateController(const std::string& id, f32 eyeHeight, f32 height, f32 radius, const sTransform* transform, const glm::vec3& up, const cPhysicsScene* scene, const cPhysicsMaterial* material)
    {
        const glm::vec3 position = transform->_position;

        PxCapsuleControllerDesc desc;
        desc.setToDefault();
        desc.height = height;
        desc.radius = radius;
        desc.position = PxExtendedVec3(position.y, position.x, position.z);
        desc.stepOffset = 0.5f;
        desc.slopeLimit = cosf(PxPi / 4.0f);
        desc.contactOffset = 0.01f;
        desc.upDirection = PxVec3(up.y, up.x, up.z);
        desc.material = material->GetMaterial();

        PxController* controller = scene->GetControllerManager()->createController(desc);

        return _controllers->Add(id, _context, controller, eyeHeight);
    }

    cPhysicsActor* cPhysics::CreateActor(const std::string& id, eCategory staticOrDynamic, eCategory shapeType, const cPhysicsScene* scene, const cPhysicsMaterial* material, f32 mass, const sTransform* transform, cGameObject* gameObject)
    {
        const glm::vec3 position = transform->_position;
        const glm::vec3 scale = transform->_scale;

        PxTransform pose(PxVec3(position.y, position.x, position.z));
            
        PxShape* shape = nullptr;
        if (shapeType == eCategory::PHYSICS_SHAPE_PLANE)
            shape = _physics->createShape(PxPlaneGeometry(), *material->GetMaterial(), false, PxShapeFlag::eSIMULATION_SHAPE | PxShapeFlag::eSCENE_QUERY_SHAPE);
        else if (shapeType == eCategory::PHYSICS_SHAPE_BOX)
            shape = _physics->createShape(PxBoxGeometry(scale.y * 0.5f, scale.x * 0.5f, scale.z * 0.5f), *material->GetMaterial(), false, PxShapeFlag::eSIMULATION_SHAPE | PxShapeFlag::eSCENE_QUERY_SHAPE);

        if (shape == nullptr)
            return nullptr;

        shape->setContactOffset(0.1f);
        shape->setRestOffset(0.05f);
            
        PxActor* actor = nullptr;
        if (staticOrDynamic == eCategory::PHYSICS_ACTOR_STATIC)
        {
            actor = _physics->createRigidStatic(pose);
            ((PxRigidStatic*)actor)->attachShape(*shape);
        }
        else if (staticOrDynamic == eCategory::PHYSICS_ACTOR_DYNAMIC)
        {
            actor = _physics->createRigidDynamic(pose);
            ((PxRigidDynamic*)actor)->attachShape(*shape);
            ((PxRigidDynamic*)actor)->setAngularDamping(0.75f);
            ((PxRigidDynamic*)actor)->setLinearVelocity(PxVec3(0.0f, 0.0f, 0.0f));
            PxRigidBodyExt::updateMassAndInertia(*((PxRigidBody*)actor), mass);
        }

        if (shape != nullptr)
            shape->release();

        if (actor != nullptr)
            scene->GetScene()->addActor(*actor);

        return _actors->Add(id, gameObject, actor, staticOrDynamic);
    }

    cPhysicsScene* cPhysics::FindScene(const std::string& id)
    {
        return _scenes->Find(id);
    }

    cPhysicsMaterial* cPhysics::FindMaterial(const std::string& id)
    {
        return _materials->Find(id);
    }

    cPhysicsActor* cPhysics::FindActor(const std::string& id)
    {
        return _actors->Find(id);
    }

    cPhysicsController* cPhysics::FindController(const std::string& id)
    {
        return _controllers->Find(id);
    }

    void cPhysics::DestroyScene(const std::string& id)
    {
        _scenes->Delete(id);
    }

    void cPhysics::DestroyMaterial(const std::string& id)
    {
        _materials->Delete(id);
    }

    void cPhysics::DestroyActor(const std::string& id)
    {
        _actors->Delete(id);
    }

    void cPhysics::DestroyController(const std::string& id)
    {
        _controllers->Delete(id);
    }

    void cPhysics::MoveController(const cPhysicsController* controller, const glm::vec3& position, f32 minStep)
    {
        cTime* time = _context->GetSubsystem<cTime>();
        const f32 deltaTime = time->GetDeltaTime();

        PxController* pxController = controller->GetController();

        PxControllerFilters filters = PxControllerFilters();
        pxController->move(
            PxVec3(position.y, position.x, position.z),
            minStep,
            deltaTime,
            filters
        );
    }

    glm::vec3 cPhysics::GetControllerPosition(const cPhysicsController* controller)
    {
        PxController* pxController = controller->GetController();
        const PxExtendedVec3 position = pxController->getPosition();

        return glm::vec3(position.y, position.x + controller->GetEyeHeight(), position.z);
    }

    void cPhysics::Simulate()
    {
        const cPhysicsActor* actorsArray = _actors->GetElements();

        for (usize i = 0; i < _actors->GetElementCount(); i++)
        {
            auto& actor = actorsArray[i];

            if (actor.GetActorType() != eCategory::PHYSICS_ACTOR_DYNAMIC)
                continue;

            sTransform* transform = actor.GetGameObject()->GetTransform();
            const PxActor* pxActor = actor.GetActor();

            const PxTransform actorTransform = ((PxRigidDynamic*)pxActor)->getGlobalPose();
            const glm::quat q = glm::quat(
                actorTransform.q.w,
                actorTransform.q.x,
                actorTransform.q.y,
                actorTransform.q.z
            );
            const glm::vec3 actorEuler = glm::eulerAngles(q);

            {
                std::lock_guard<std::mutex> lock(_mutex);
                transform->_position = glm::vec3(actorTransform.p.y, actorTransform.p.x, actorTransform.p.z);
                transform->_rotation = glm::vec3(actorEuler.y, actorEuler.x, actorEuler.z);
            }
        }

        const cPhysicsScene* scenesArray = _scenes->GetElements();

        for (usize i = 0; i < _scenes->GetElementCount(); i++)
        {
            PxScene* scene = scenesArray[i].GetScene();
            scene->simulate(1.0f / 60.0f);
            scene->fetchResults(true);
        }
    }

    /*void mPhysics::Update()
    {
        // Actors
        for (auto& pair : _actors)
        {
            sCTransform* transformComponent = pair.Scene->Get<sCTransform>(pair.Entity);
            sCPhysicsActor* actorComponent = pair.Scene->Get<sCPhysicsActor>(pair.Entity);

            PxTransform actorTransform = ((PxRigidDynamic*)actorComponent->Actor)->getGlobalPose();
            glm::vec3 actorEuler = glm::eulerAngles(
                glm::quat(
                    actorTransform.q.w,
                    actorTransform.q.x,
                    actorTransform.q.y,
                    actorTransform.q.z
                )
            );

            {
                std::lock_guard<std::mutex> lock(_mutex);
                transformComponent->Position = glm::vec3(actorTransform.p.y, actorTransform.p.x, actorTransform.p.z);
                transformComponent->Rotation = glm::vec3(actorEuler.y, actorEuler.x, actorEuler.z);
            }
        }

        // Controllers
        for (auto& pair : _controllers)
        {
            sCTransform* transformComponent = pair.Scene->Get<sCTransform>(pair.Entity);
            sCPhysicsCharacterController* controllerComponent =
                pair.Scene->Get<sCPhysicsCharacterController>(pair.Entity);

            if (controllerComponent->IsGravityEnabled == K_TRUE) {
                SetCharacterControllerMovement(
                    pair, glm::vec3(0.0f, -controllerComponent->GravitySpeed, 0.0f)
                );
            }

            PxExtendedVec3 position = controllerComponent->Controller->getPosition();
                
            {
                std::lock_guard<std::mutex> lock(_mutex);
                transformComponent->Position = glm::vec3(
                    position.y, position.x, position.z
                );
            }
        }

        // Scenes
        for (auto& pair : _scenes)
        {
            sCPhysicsScene* component = pair.Scene->Get<sCPhysicsScene>(pair.Entity);
            component->Scene->simulate(1.0f / 60.0f);
            component->Scene->fetchResults(true);
        }
    }

    sCPhysicsScene* mPhysics::AddScene(const sEntityScenePair& scene)
    {
        PxSceneDesc sceneDesc(_physics->getTolerancesScale());
        sceneDesc.gravity = PxVec3(-9.81f, 0.0f, 0.0f);
        sceneDesc.cpuDispatcher = _cpuDispatcher;
        sceneDesc.simulationEventCallback = _simulationEvent;
        sceneDesc.filterShader = FilterShader;

        sCPhysicsScene* component = scene.Scene->Add<sCPhysicsScene>(scene.Entity);
        component->Scene = _physics->createScene(sceneDesc);
        component->ControllerManager = PxCreateControllerManager(*component->Scene);

        _scenes.push_back(scene);

        return component;
    }

    sCPhysicsActor* mPhysics::AddActor(
        const sEntityScenePair& scene,
        const sEntityScenePair& actor,
        const mPhysics::eActorDescriptor& actorDesc,
        const mPhysics::eShapeDescriptor& shapeDesc,
        const glm::vec4& extents,
        const sVertexBufferGeometry* const geometry
    )
    {
        sCTransform* transformComponent = actor.Scene->Get<sCTransform>(actor.Entity);

        glm::vec3& entityPosition = transformComponent->Position;
        glm::vec3 position = glm::vec3(entityPosition.y, entityPosition.x, entityPosition.z);

        // Creating material
        PxMaterial* physicsMaterial = _physics->createMaterial(0.05f, 0.05f, 0.05f);

        // Creation of actor
        PxActor* physicsActor = nullptr;
            
        switch (actorDesc)
        {

        case mPhysics::eActorDescriptor::STATIC:
            physicsActor = _physics->createRigidStatic(PxTransform(position.x, position.y, position.z));
            break;

        case mPhysics::eActorDescriptor::DYNAMIC:
            physicsActor = _physics->createRigidDynamic(PxTransform(position.x, position.y, position.z));
            ((PxRigidDynamic*)physicsActor)->setAngularDamping(0.75f);
            ((PxRigidDynamic*)physicsActor)->setLinearVelocity(PxVec3(0.0f, 0.0f, 0.0f));
            break;

        }

        if (physicsActor == nullptr) { return nullptr; }

        sCPhysicsScene* sceneComponent = scene.Scene->Get<sCPhysicsScene>(scene.Entity);
        sceneComponent->Scene->addActor(*physicsActor);

        // Shape creation
        PxShape* physicsShape = nullptr;

        switch (shapeDesc)
        {
            case mPhysics::eShapeDescriptor::SPHERE:
            {
                physicsShape = _physics->createShape(
                    PxSphereGeometry(extents.x),
                    *physicsMaterial
                );
                break;
            }

            case mPhysics::eShapeDescriptor::CAPSULE:
            {
                physicsShape = _physics->createShape(
                    PxCapsuleGeometry(extents.x, extents.y),
                    *physicsMaterial
                );
                break;
            }

            case mPhysics::eShapeDescriptor::BOX:
            {
                physicsShape = _physics->createShape(
                    PxBoxGeometry(extents.x, extents.y, extents.z),
                    *physicsMaterial
                );
                break;
            }

            case mPhysics::eShapeDescriptor::PLANE:
            {
                physicsShape = _physics->createShape(
                    PxPlaneGeometry(),
                    *physicsMaterial
                );
                break;
            }

            case mPhysics::eShapeDescriptor::TRIANGLE_MESH:
            {
                if (geometry->Format !=
                    render::sVertexBufferGeometry::eFormat::POSITION_TEXCOORD_NORMAL_VEC3_VEC2_VEC3)
                {
                    MessageBox(0, "PhysX shape 'TRIANGLE_MESH' can only be created with geometry of 'POSITION_TEXCOORD_NORMAL_VEC3_VEC2_VEC3' format!", "Error", MB_ICONERROR);
                    return nullptr;
                }

                PxVec3* verticesPX = new PxVec3[geometry->VertexCount];
                for (usize i = 0; i < geometry->VertexCount; i++) {
                    verticesPX[i] = PxVec3(
                        ((render::sVertex*)geometry->VertexPtr)[i].Position.y,
                        ((render::sVertex*)geometry->VertexPtr)[i].Position.x,
                        ((render::sVertex*)geometry->VertexPtr)[i].Position.z
                    );
                }

                PxTriangleMeshDesc meshDesc;
                meshDesc.points.count = geometry->VertexCount;
                meshDesc.points.stride = sizeof(PxVec3);
                meshDesc.points.data = verticesPX;

                meshDesc.triangles.count = geometry->IndexCount / 3;
                meshDesc.triangles.stride = 3 * sizeof(PxU32);
                meshDesc.triangles.data = geometry->IndexPtr;

                PxTolerancesScale scale(1.0f, 10.0f);
                PxCookingParams params(scale);
                PxDefaultMemoryOutputStream writeBuffer;
                PxTriangleMeshCookingResult::Enum result;
                bool status = PxCookTriangleMesh(params, meshDesc, writeBuffer, &result);
                if (!status || result == PxTriangleMeshCookingResult::eFAILURE)
                {
                    MessageBox(0, "PhysX can't cook triangle mesh!", "Error", MB_ICONERROR);
                    return nullptr;
                }

                PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
                PxTriangleMesh* triangleMesh = _physics->createTriangleMesh(readBuffer);
                    
                ((PxRigidDynamic*)physicsActor)->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
                    
                physicsShape = _physics->createShape(
                    PxTriangleMeshGeometry(
                        triangleMesh,
                        PxMeshScale(PxVec3(1.0f, 1.0f, 1.0f))
                    ),
                    *physicsMaterial,
                    false,
                    PxShapeFlag::eSIMULATION_SHAPE
                );

                delete[] verticesPX;

                break;
            }
        }

        if (physicsShape == nullptr) { return nullptr; }

        physicsShape->setContactOffset(0.1f);
        physicsShape->setRestOffset(0.05f);

        PxFilterData FilterData;
        FilterData.word0 = 0; FilterData.word1 = 0; FilterData.word2 = 0; FilterData.word3 = 0;
        physicsShape->setSimulationFilterData(FilterData);

        switch (actorDesc)
        {

        case mPhysics::eActorDescriptor::STATIC:
            ((PxRigidStatic*)physicsActor)->attachShape(*physicsShape);
            break;

        case mPhysics::eActorDescriptor::DYNAMIC:
            ((PxRigidDynamic*)physicsActor)->attachShape(*physicsShape);
            break;

        }
            
        sCPhysicsActor* component = actor.Scene->Add<sCPhysicsActor>(actor.Entity);
        component->Actor = physicsActor;
        component->Material = physicsMaterial;

        _actors.push_back(actor);

        return component;
    }

    sCPhysicsCharacterController* mPhysics::AddCharacterController(
        const sEntityScenePair& scene,
        const sEntityScenePair& controller,
        const mPhysics::eShapeDescriptor& shapeDesc,
        const glm::vec4& extents
    )
    {
        sCPhysicsScene* sceneComponent = scene.Scene->Get<sCPhysicsScene>(scene.Entity);
        sCTransform* transformComponent = controller.Scene->Get<sCTransform>(controller.Entity);
        PxControllerManager* controllerManager = sceneComponent->ControllerManager;
            
        PxMaterial* physicsMaterial = _physics->createMaterial(0.05f, 0.05f, 0.05f);

        PxController* physicsController = nullptr;
        switch (shapeDesc)
        {
            case mPhysics::eShapeDescriptor::CAPSULE:
            {
                PxCapsuleControllerDesc desc = PxCapsuleControllerDesc();
                desc.setToDefault();
                desc.radius = extents.x;
                desc.height = extents.y;
                desc.position = PxExtendedVec3(
                    transformComponent->Position.y,
                    transformComponent->Position.x,
                    transformComponent->Position.z
                );
                desc.upDirection = PxVec3(1.0f, 0.0f, 0.0f);
                desc.contactOffset = 0.01;
                desc.stepOffset = desc.height * 0.25f;
                desc.slopeLimit = glm::cos(glm::radians(45.0f));
                desc.material = physicsMaterial;

                physicsController = controllerManager->createController(desc);
                break;
            }

            case mPhysics::eShapeDescriptor::BOX:
            {
                PxBoxControllerDesc desc = PxBoxControllerDesc();
                desc.halfForwardExtent = extents.x;
                desc.halfSideExtent = extents.y;
                desc.halfHeight = extents.z;
                desc.stepOffset = desc.halfHeight * 0.25f;
                desc.slopeLimit = glm::cos(glm::radians(45.0f));
                desc.material = physicsMaterial;

                physicsController = controllerManager->createController(desc);
                break;
            }
        }

        sCPhysicsCharacterController* controllerComponent =
            controller.Scene->Add<sCPhysicsCharacterController>(controller.Entity);
        controllerComponent->Controller = physicsController;
        controllerComponent->Material = physicsMaterial;

        _controllers.push_back(controller);
            
        return controllerComponent;
    }

    void mPhysics::SetForce(const sEntityScenePair& actor, const glm::vec3& force)
    {
        sCPhysicsActor* component = actor.Scene->Get<sCPhysicsActor>(actor.Entity);
        ((PxRigidDynamic*)component->Actor)->setForceAndTorque(
            PxVec3(force.y, force.x, force.z),
            PxVec3(0.0f, 0.0f, 0.0f)
        );
    }

    void mPhysics::SetCharacterControllerMovement(const sEntityScenePair& controller, const glm::vec3& direction)
    {
        sCPhysicsCharacterController* controllerComponent =
            controller.Scene->Get<sCPhysicsCharacterController>(controller.Entity);

        PxControllerFilters filters = PxControllerFilters();
        controllerComponent->Controller->move(
            PxVec3(direction.y, direction.x, direction.z),
            0.01f,
            1.0f,
            filters
        );
    }*/
}