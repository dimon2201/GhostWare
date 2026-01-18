// sound_context_al.cpp

#include <iostream>
#include <cstdio>
#include <string>
#include <windows.h>
#include "audio.hpp"
#include "application.hpp"
#include "sound_context.hpp"
#include "sound_manager.hpp"
#include "memory_pool.hpp"
#include "log.hpp"

using namespace types;

namespace arachne
{
    cOpenALSoundAPI::cOpenALSoundAPI(cContext* context) : iSoundAPI(context)
    {
        _device = alcOpenDevice(nullptr);
        _context = alcCreateContext(_device, nullptr);
        alcMakeContextCurrent(_context);
    }

    cOpenALSoundAPI::~cOpenALSoundAPI()
    {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(_context);
        alcCloseDevice(_device);
    }

    void cOpenALSoundAPI::Create(cSound::eFormat type, cSound* sound)
    {
        if (type == cSound::eFormat::WAV)
        {
            ALuint source = 0;
            ALuint buffer = 0;

            alGenSources(1, (ALuint*)&source);
            alSourcef(source, AL_PITCH, 1);
            alSourcef(source, AL_GAIN, 1);
            alSource3f(source, AL_POSITION, 0, 0, 0);
            alSource3f(source, AL_VELOCITY, 0, 0, 0);
            alSourcei(source, AL_LOOPING, AL_FALSE);

            alGenBuffers(1, (ALuint*)&buffer);

            ALenum wavFormat = AL_FORMAT_STEREO16;
            types::boolean stereo = sound->GetChannelCount() > 1;
            switch (sound->GetBitsPerSample())
            {
                case 16:
                    if (stereo)
                    {
                        wavFormat = AL_FORMAT_STEREO16;
                        break;
                    }
                    else
                    {
                        wavFormat = AL_FORMAT_MONO16;
                        break;
                    }
                case 8:
                    if (stereo)
                    {
                        wavFormat = AL_FORMAT_STEREO8;
                        break;
                    }
                    else
                    {
                        wavFormat = AL_FORMAT_MONO8;
                        break;
                    }
                default:
                    break;
            }

            alBufferData(buffer, wavFormat, sound->GetData(), sound->GetDataByteSize(), sound->GetSampleRate());
            alSourcei(source, AL_BUFFER, buffer);

            sound->SetSource(source);
            sound->SetBuffer(buffer);
        }
    }

    void cOpenALSoundAPI::Destroy(cSound* sound)
    {
        u32 buffer = sound->GetBuffer();
        u32 source = sound->GetSource();
        alDeleteBuffers(1, (ALuint*)&buffer);
        alDeleteSources(1, (ALuint*)&source);

        sound->~cSound();
        _app->GetMemoryPool()->Free(sound);
    }

    void cOpenALSoundAPI::Play(const cSound* sound)
    {
        alSourcePlay(sound->GetSource());
    }

    void cOpenALSoundAPI::Stop(const cSound* sound)
    {
        alSourceStop(sound->GetSource());
    }

    void cOpenALSoundAPI::SetPosition(const cSound* sound, const glm::vec3& position)
    {
        alSource3f(sound->GetSource(), AL_POSITION, position.x, position.y, position.z);
    }

    void cOpenALSoundAPI::SetVelocity(const cSound* sound, const glm::vec3& velocity)
    {
        alSource3f(sound->GetSource(), AL_VELOCITY, velocity.x, velocity.y, velocity.z);
    }

    void cOpenALSoundAPI::SetListenerPosition(const glm::vec3& position)
    {
        alListener3f(AL_POSITION, position.x, position.y, position.z);
    }

    void cOpenALSoundAPI::SetListenerVelocity(const glm::vec3& velocity)
    {
        alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);
    }

    void cOpenALSoundAPI::SetListenerOrientation(const glm::vec3& at, const glm::vec3& up)
    {
        ALfloat values[] = { at.x, at.y, at.z, up.x, up.y, up.z };
        alListenerfv(AL_ORIENTATION, &values[0]);
    }
}