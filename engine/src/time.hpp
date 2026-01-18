// time.hpp

#pragma once

#include <string>
#include <typeinfo>
#include "log.hpp"
#include "object.hpp"
#include "types.hpp"

namespace arachne
{
	class cTime : public cObject
	{
		REALWARE_OBJECT(cTime)

	public:
		explicit cTime(cContext* context);
		virtual ~cTime() override final = default;

		void BeginFrame();
		void Update();
		void EndFrame();

		inline types::f32 GetDeltaTime() const { return _deltaTime; }

	private:
		std::chrono::steady_clock::time_point _timepointLast;
		types::f32 _deltaTime = 0.0f;
	};
}