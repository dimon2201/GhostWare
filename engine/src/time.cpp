// time.cpp

#include "time.hpp"

namespace arachne
{
	cTime::cTime(cContext* context) : cObject(context) {}

	void cTime::BeginFrame()
	{
		_timepointLast = std::chrono::high_resolution_clock::now();
	}

	void cTime::Update()
	{
		const auto currentTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<f32> elapsed = currentTime - _timepointLast;
		_deltaTime = elapsed.count();
		_timepointLast = currentTime;
	}

	void cTime::EndFrame()
	{
	}
}