// sound_manager.cpp

#include "application.hpp"
#include "sound_manager.hpp"
#include "sound_context.hpp"
#include "memory_pool.hpp"

using namespace types;

namespace arachne
{
    mSound::mSound(cContext* context, iSoundContext* soundContext) : cObject(context), _soundContext(soundContext), _sounds(app) {}

    cSound* mSound::CreateSound(const std::string& id, const std::string& filename, eCategory format)
    {
        u32 source = 0;
        u32 buffer = 0;
        sWAVStructure* file = nullptr;
        _soundContext->Create(filename, format, (const sWAVStructure**)&file, source, buffer);

        return _sounds.Add(id, GetApplication(), source, buffer);
    }

    cSound* mSound::FindSound(const std::string& id)
    {
        return _sounds.Find(id);
    }

    void mSound::DestroySound(const std::string& id)
    {
        _sounds.Delete(id);
    }

    /*void mSound::Play(entity object, cScene* scene)
    {
        core::sCSound* sound = scene->Get<core::sCSound>(object);
        core::sCTransform* transform = scene->Get<core::sCTransform>(object);
        _context->SetPosition(sound->Sound, transform->Position);
        _context->Play(sound->Sound);
    }

    void mSound::Stop(core::entity object, core::cScene* scene)
    {
        core::sCSound* sound = scene->Get<core::sCSound>(object);
        _context->Stop(sound->Sound);
    }*/
}