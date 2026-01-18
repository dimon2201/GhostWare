// audio.cpp

#include <fstream>
#include "audio.hpp"
#include "context.hpp"
#include "sound_context.hpp"
#include "engine.hpp"
#include "log.hpp"

using namespace types;

namespace arachne
{
	cSound::cSound(cContext* context) : cObject(context), _audioBackend(context->GetSubsystem<cAudio>()->GetAPI()) {}

	cSound::~cSound()
	{
		if (_file != nullptr)
		{
			if (_format == eCategory::SOUND_FORMAT_WAV)
			{
				cMemoryPool* memoryPool = GetApplication()->GetMemoryPool();
				memoryPool->Free(_file->_data);
				_file->~sWAVStructure();
				memoryPool->Free(_file);
			}
		}
	}

	void cSound::Load(eType type, const std::string& path)
	{
		const sEngineCapabilities* caps = _context->GetSubsystem<cEngine>()->GetCapabilities();
		cMemoryAllocator* memoryAllocator = _context->GetMemoryAllocator();

		sWAVStructure wav = {};

		std::ifstream inputFile(path, std::ios::binary);
		if (!inputFile)
			return Print("Error: can't open sound file at '" + path + "'!");

		if (type == eType::WAV)
		{
			// Chunk
			inputFile.read(&wav._type[0], 4);
			if (std::string((const char*)&wav._type[0]) != std::string("RIFF"))
				Print("Error: not a RIFF file!");
			inputFile.read(&wav._chunkSize, sizeof(types::u32));
			inputFile.read(&wav._format[0], 4);
			if (std::string((const char*)&wav._format[0]) != std::string("WAVE"))
				Print("Error: not a WAVE file!");

			// 1st Subchunk
			inputFile.read(&wav._subchunk1ID, 4);
			if (std::string((const char*)&wav._subchunk1ID[0]) != std::string("fmt "))
				Print("Error: missing fmt header!");
			inputFile.read(&wav._subchunk1Size, sizeof(types::u32));
			inputFile.read(&wav._audioFormat, sizeof(types::u16));
			inputFile.read(&_channelCount, sizeof(types::u16));
			inputFile.read(&_sampleRate, sizeof(types::u32));
			inputFile.read(&wav._byteRate, sizeof(types::u32));
			inputFile.read(&wav._blockAlign, sizeof(types::u16));
			inputFile.read(&_bitsPerSample, sizeof(types::u16));

			// 2nd Subchunk
			inputFile.read(&wav._subchunk2ID, 4);
			if (std::string((const char*)&wav._subchunk2ID[0]) != std::string("data"))
				Print("Error: missing data header!");
			inputFile.read(&wav._subchunk2Size, sizeof(types::u32));

			// Data
			const int numSamples = wav._subchunk2Size / (_channelCount * (_bitsPerSample / 8));
			_dataByteSize = numSamples * (_bitsPerSample / 8) * _channelCount;
			_data = (u16*)memoryAllocator->Allocate(_dataByteSize, caps->memoryAlignment);
			if (_bitsPerSample == 16 && _channelCount == 2)
			{
				for (usize i = 0; i < numSamples; i++)
				{
					const usize idx = i * 2;
					inputFile.read(&_data[idx], sizeof(u16));
					inputFile.read(&_data[idx + 1], sizeof(u16));
				}
			}
			inputFile.close();
		}
	}

	cAudio::cAudio(cContext* context) : cObject(context) {}

	cAudio::~cAudio()
	{
		if (api == API::NONE)
		{
			Print("Error: sound API not selected!");

			return;
		}
		else if (api == API::OPENAL)
		{
			if (_api)
				delete _api;
		}
	}

	cSound* cAudio::CreateSound(const std::string& id, const std::string& path, cSound::eFormat format)
	{
		cSound* sound = _context->Create<cSound>(_context);
		_audioBackend->Create(format, sound);

		return _sounds.Add(id, GetApplication(), source, buffer);
	}

	cSound* cAudio::FindSound(const std::string& id)
	{
		return _sounds.Find(id);
	}

	void cAudio::DestroySound(const std::string& id)
	{
		_sounds.Delete(id);
	}

	void cAudio::SetAPI(API api)
	{
		if (api == API::NONE)
		{
			Print("Error: sound API not selected!");

			return;
		}
		else if (api == API::OAL)
		{
			_api = new cOpenALSoundAPI(_context);
		}
	}
}