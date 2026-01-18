// engine.hpp

#pragma once

#include <unordered_map>
#include "object.hpp"
#include "types.hpp"

namespace arachne
{
	class cContext;
	class iApplication;

	struct sEngineCapabilities
	{
		std::string windowTitle = "Test app";
		types::usize windowWidth = 640;
		types::usize windowHeight = 480;
		types::boolean fullscreen = types::K_FALSE;
		types::usize memoryAlignment = 64;
		types::usize maxPhysicsSceneCount = 16;
		types::usize maxPhysicsMaterialCount = 256;
		types::usize maxPhysicsActorCount = 8192;
		types::usize maxPhysicsControllerCount = 8;
	};

	class cEngine : public cObject
	{
		REALWARE_OBJECT(cEngine)

	public:
		explicit cEngine(cContext* context, const sEngineCapabilities* capabilities, iApplication* app);
		virtual ~cEngine() override final = default;

		void Initialize();
		void Run();

		const sEngineCapabilities* GetCapabilities() const { return _capabilities; }

	private:
		const sEngineCapabilities* _capabilities = nullptr;
		iApplication* _app = nullptr;
	};
}