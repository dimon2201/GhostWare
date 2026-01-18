// sound_context.hpp

#pragma once

#include <vector>
#include <AL/al.h>
#include <AL/alc.h>
#include "../../thirdparty/glm/glm/glm.hpp"
#include "audio.hpp"
#include "category.hpp"
#include "object.hpp"
#include "id_vec.hpp"
#include "types.hpp"

namespace arachne
{
    class cApplication;
    class cSound;
    struct sWAVStructure;

    class iSoundAPI : public cObject
    {
        REALWARE_OBJECT(iSoundAPI)

    public:
        explicit iSoundAPI(cContext* context) : cObject(context) {}
        virtual ~iSoundAPI() = default;

        virtual void Create(cSound::eFormat type, cSound* sound) = 0;
        virtual void Destroy(cSound* sound) = 0;
        virtual void Play(const cSound* sound) = 0;
        virtual void Stop(const cSound* sound) = 0;
        virtual void SetPosition(const cSound* sound, const glm::vec3& position) = 0;
        virtual void SetVelocity(const cSound* sound, const glm::vec3& velocity) = 0;
        virtual void SetListenerPosition(const glm::vec3& position) = 0;
        virtual void SetListenerVelocity(const glm::vec3& velocity) = 0;
        virtual void SetListenerOrientation(const glm::vec3& at, const glm::vec3& up) = 0;
    };

    class cOpenALSoundAPI : public iSoundAPI
    {
        REALWARE_OBJECT(cOpenALSoundAPI)

    public:
        cOpenALSoundAPI(cContext* context);
        virtual ~cOpenALSoundAPI() override final;

        virtual void Create(cSound::eFormat type, cSound* sound) override final;
        virtual void Destroy(cSound* sound) override final;
        virtual void Play(const cSound* sound) override final;
        virtual void Stop(const cSound* sound) override final;
        virtual void SetPosition(const cSound* sound, const glm::vec3& position) override final;
        virtual void SetVelocity(const cSound* sound, const glm::vec3& velocity) override final;
        virtual void SetListenerPosition(const glm::vec3& position) override final;
        virtual void SetListenerVelocity(const glm::vec3& velocity) override final;
        virtual void SetListenerOrientation(const glm::vec3& at, const glm::vec3& up) override final;

    private:
        ALCdevice* _device = nullptr;
        ALCcontext* _context = nullptr;
    };
}