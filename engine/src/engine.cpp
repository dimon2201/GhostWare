// engine.cpp

#include "engine.hpp"
#include "application.hpp"
#include "context.hpp"
#include "graphics.hpp"
#include "input.hpp"
#include "camera_manager.hpp"
#include "texture_manager.hpp"
#include "filesystem_manager.hpp"
#include "font_manager.hpp"
#include "physics_manager.hpp"
#include "time.hpp"
#include "event_manager.hpp"
#include "gameobject_manager.hpp"
#include "thread_manager.hpp"
#include "render_context.hpp"
#include "audio.hpp"

using namespace types;

namespace harpy
{
	cEngine::cEngine(cContext* context, iApplication* app) : iObject(context), _app(app) {}

	void cEngine::Initialize()
	{
		// Create memory allocator
		_context->CreateMemoryAllocator();

		// Register factories
		_context->RegisterFactory<cWindow>();
		_context->RegisterFactory<cBuffer>();
		_context->RegisterFactory<cShader>();
		_context->RegisterFactory<cTexture>();
		_context->RegisterFactory<cRenderTarget>();
		_context->RegisterFactory<cRenderPass>();

		// Register subsystems
		_context->RegisterSubsystem(this);
		_context->RegisterSubsystem(new cGraphics(_context, cGraphics::eAPI::OGL));
		_context->RegisterSubsystem(new cInput(_context));
		_context->RegisterSubsystem(new cCamera(_context));
		_context->RegisterSubsystem(new cTextureAtlas(_context));
		_context->RegisterSubsystem(new cFileSystem(_context));
		_context->RegisterSubsystem(new cFont(_context));
		_context->RegisterSubsystem(new cPhysics(_context));
		_context->RegisterSubsystem(new cGameObject(_context));
		_context->RegisterSubsystem(new cThread(_context));
		_context->RegisterSubsystem(new cTime(_context));
		_context->RegisterSubsystem(new cEventDispatcher(_context));
		_context->RegisterSubsystem(new cAudio(_context));

		// Create texture manager
		cTextureAtlas* texture = _context->GetSubsystem<cTextureAtlas>();
		texture->SetAtlas(glm::vec3(2048, 2048, 16));

		// Create sound context
		cAudio* audio = _context->GetSubsystem<cAudio>();
		audio->SetAPI(cAudio::API::OAL);
	}

	void cEngine::Run()
	{
		if (_app == nullptr)
			return;

		_app->Setup();

		auto gfx = _context->GetSubsystem<cGraphics>();
		auto input = _context->GetSubsystem<cInput>();
		auto camera = _context->GetSubsystem<cCamera>();
		auto time = _context->GetSubsystem<cTime>();
		auto physics = _context->GetSubsystem<cPhysics>();

		cWindow* window = _app->GetWindow();

		time->BeginFrame();

		while (window->GetRunState() == K_FALSE)
		{
			time->Update();
			physics->Simulate();
			camera->Update();
			gfx->CompositeFinal();
			input->SwapBuffers();
			input->PollEvents();
		}

		time->EndFrame();

		_app->Stop();
	}
}