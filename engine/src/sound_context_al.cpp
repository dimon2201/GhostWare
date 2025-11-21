// sound_context_al.cpp

#include <iostream>
#include <cstdio>
#include <string>
#include <windows.h>
#include "application.hpp"
#include "sound_context.hpp"
#include "sound_manager.hpp"
#include "memory_pool.hpp"
#include "log.hpp"

namespace realware
{
    using namespace app;
    using namespace game;
    using namespace log;
    using namespace types;
    using namespace utils;

    namespace sound
    {
        sWAVStructure* LoadWAVFile(cMemoryPool* const memoryPool, const std::string& filename)
        {
            sWAVStructure* pWav = (sWAVStructure*)memoryPool->Allocate(sizeof(sWAVStructure));
            sWAVStructure* wav = new (pWav) sWAVStructure();

            FILE* fp = nullptr;
            errno_t err = fopen_s(&fp, &filename.c_str()[0], "rb");
            if (err != 0)
                Print("Error: can't open WAV file at '" + filename + "'!");

            // Chunk
            fread(&wav->Type[0], sizeof(char), 4, fp);
            if (std::string((const char*)&wav->Type[0]) != std::string("RIFF"))
                Print("Error: not a RIFF file!");

            fread(&wav->ChunkSize, sizeof(int), 1, fp);
            fread(&wav->Format[0], sizeof(char), 4, fp);
            if (std::string((const char*)&wav->Format[0]) != std::string("WAVE"))
                Print("Error: not a WAVE file!");

            // 1st Subchunk
            fread(&wav->Subchunk1ID[0], sizeof(char), 4, fp);
            if (std::string((const char*)&wav->Subchunk1ID[0]) != std::string("fmt "))
                Print("Error: missing fmt header!");
            fread(&wav->Subchunk1Size, sizeof(int), 1, fp);
            fread(&wav->AudioFormat, sizeof(short), 1, fp);
            fread(&wav->NumChannels, sizeof(short), 1, fp);
            fread(&wav->SampleRate, sizeof(int), 1, fp);
            fread(&wav->ByteRate, sizeof(int), 1, fp);
            fread(&wav->BlockAlign, sizeof(short), 1, fp);
            fread(&wav->BitsPerSample, sizeof(short), 1, fp);

            // 2nd Subchunk
            fread(&wav->Subchunk2ID[0], sizeof(char), 4, fp);
            if (std::string((const char*)&wav->Subchunk2ID[0]) != std::string("data"))
                Print("Error: missing data header!");
            fread(&wav->Subchunk2Size, sizeof(int), 1, fp);

            // Data
            int NumSamples = wav->Subchunk2Size / (wav->NumChannels * (wav->BitsPerSample / 8));
            wav->DataByteSize = NumSamples * (wav->BitsPerSample / 8) * wav->NumChannels;
            wav->Data = (unsigned short*)memoryPool->Allocate(wav->DataByteSize);
            if (wav->BitsPerSample == 16 && wav->NumChannels == 2)
            {
                for (int i = 0; i < NumSamples; i++)
                {
                    int idx = i * 2;
                    fread(&wav->Data[idx], sizeof(short), 1, fp);
                    fread(&wav->Data[idx + 1], sizeof(short), 1, fp);
                }
            }
            fclose(fp);

            return wav;
        }

        cOpenALSoundContext::cOpenALSoundContext(const cApplication* const app)
        {
            _app = (cApplication*)app;
            _device = alcOpenDevice(nullptr);
            _context = alcCreateContext(_device, nullptr);
            alcMakeContextCurrent(_context);
        }

        cOpenALSoundContext::~cOpenALSoundContext()
        {
            alcMakeContextCurrent(nullptr);
            alcDestroyContext(_context);
            alcCloseDevice(_device);
        }

        void cOpenALSoundContext::Create(const std::string& filename, const Category& format, const sWAVStructure** const file, types::u32& source, types::u32& buffer)
        {
            if (format == Category::SOUND_FORMAT_WAV)
            {
                sWAVStructure* wavFile = LoadWAVFile(_app->GetMemoryPool(), filename);
                *file = wavFile;

                alGenSources(1, (ALuint*)&source);
                alSourcef(source, AL_PITCH, 1);
                alSourcef(source, AL_GAIN, 1);
                alSource3f(source, AL_POSITION, 0, 0, 0);
                alSource3f(source, AL_VELOCITY, 0, 0, 0);
                alSourcei(source, AL_LOOPING, AL_FALSE);

                alGenBuffers(1, (ALuint*)&buffer);

                ALenum wavFormat = AL_FORMAT_STEREO16;
                bool stereo = (wavFile->NumChannels > 1);
                switch (wavFile->BitsPerSample) {
                    case 16:
                        if (stereo) {
                            wavFormat = AL_FORMAT_STEREO16;
                            break;
                        } else {
                            wavFormat = AL_FORMAT_MONO16;
                            break;
                        }
                    case 8:
                        if (stereo) {
                            wavFormat = AL_FORMAT_STEREO8;
                            break;
                        } else {
                            wavFormat = AL_FORMAT_MONO8;
                            break;
                        }
                    default:
                        break;
                }

                alBufferData(buffer, wavFormat, wavFile->Data, wavFile->DataByteSize, wavFile->SampleRate);
                alSourcei(source, AL_BUFFER, buffer);
            }
        }

        void cOpenALSoundContext::Destroy(cSound* sound)
        {
            u32 buffer = sound->GetBuffer();
            u32 source = sound->GetSource();
            alDeleteBuffers(1, (ALuint*)&buffer);
            alDeleteSources(1, (ALuint*)&source);

            sound->~cSound();
            _app->GetMemoryPool()->Free(sound);
        }

        void cOpenALSoundContext::Play(const cSound* const sound)
        {
            alSourcePlay(sound->GetSource());
        }

        void cOpenALSoundContext::Stop(const cSound* const sound)
        {
            alSourceStop(sound->GetSource());
        }

        void cOpenALSoundContext::SetPosition(const cSound* const sound, const glm::vec3& position)
        {
            alSource3f(sound->GetSource(), AL_POSITION, position.x, position.y, position.z);
        }

        void cOpenALSoundContext::SetVelocity(const cSound* const sound, const glm::vec3& velocity)
        {
            alSource3f(sound->GetSource(), AL_VELOCITY, velocity.x, velocity.y, velocity.z);
        }

        void cOpenALSoundContext::SetListenerPosition(const glm::vec3& position)
        {
            alListener3f(AL_POSITION, position.x, position.y, position.z);
        }

        void cOpenALSoundContext::SetListenerVelocity(const glm::vec3& velocity)
        {
            alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);
        }

        void cOpenALSoundContext::SetListenerOrientation(const glm::vec3& at, const glm::vec3& up)
        {
            ALfloat values[] = { at.x, at.y, at.z, up.x, up.y, up.z };
            alListenerfv(AL_ORIENTATION, &values[0]);
        }
    }
}