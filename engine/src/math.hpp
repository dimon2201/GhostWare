// math.hpp

#pragma once

#include "../../thirdparty/glm/glm/glm.hpp"
#include "../../thirdparty/glm/glm/gtc/matrix_transform.hpp"
#include "../../thirdparty/glm/glm/gtc/quaternion.hpp"
#include "../../thirdparty/glm/glm/gtx/quaternion.hpp"
#include "object.hpp"
#include "types.hpp"

namespace triton
{
	class cVector2
	{
	public:
		explicit cVector2(const glm::vec2& vec);
		explicit cVector2(types::f32 value);
		explicit cVector2(types::f32 x, types::f32 y);
		~cVector2() = default;

		cVector2 operator+(const cVector2& vec) const;
		cVector2 operator-(const cVector2& vec) const;
		cVector2 operator*(const cVector2& vec) const;
		cVector2 operator/(const cVector2& vec) const;

		inline types::f32 GetX() const { return _vec.x; }
		inline types::f32 GetY() const { return _vec.y; }
		inline void SetX(types::f32 value) { _vec.x = value; }
		inline void SetY(types::f32 value) { _vec.y = value; }
		inline void AddX(types::f32 value) { _vec.x += value; }
		inline void AddY(types::f32 value) { _vec.y += value; }

	private:
		glm::vec2 _vec = glm::vec2(0.0f);
	};

	class cVector3
	{
	public:
		explicit cVector3(const glm::vec3& vec);
		explicit cVector3(types::f32 value);
		explicit cVector3(types::f32 x, types::f32 y, types::f32 z);
		~cVector3() = default;

		cVector3 operator+(const cVector3& vec) const;
		cVector3 operator-(const cVector3& vec) const;
		cVector3 operator*(const cVector3& vec) const;
		cVector3 operator/(const cVector3& vec) const;

		inline types::f32 GetX() const { return _vec.x; }
		inline types::f32 GetY() const { return _vec.y; }
		inline types::f32 GetZ() const { return _vec.z; }
		inline void SetX(types::f32 value) { _vec.x = value; }
		inline void SetY(types::f32 value) { _vec.y = value; }
		inline void SetZ(types::f32 value) { _vec.z = value; }
		inline void AddX(types::f32 value) { _vec.x += value; }
		inline void AddY(types::f32 value) { _vec.y += value; }
		inline void AddZ(types::f32 value) { _vec.z += value; }

	private:
		glm::vec3 _vec = glm::vec3(0.0f);
	};

	class cQuaternion
	{
	public:
		explicit cQuaternion(const glm::quat& quat);
		explicit cQuaternion(types::f32 angle, const cVector3& axis);
		~cQuaternion() = default;

		cVector3 operator*(const cVector3& vec) const;
		cQuaternion operator*(const cQuaternion& quat) const;

	private:
		glm::quat _quat = {};
	};

	class cMatrix4
	{
	public:
		explicit cMatrix4(const glm::mat4& mat);
		explicit cMatrix4(types::f32 value);
		explicit cMatrix4(const cVector3& position, const cVector3& direction, const cVector3& up);
		explicit cMatrix4(types::f32 fov, types::f32 aspect, types::f32 zNear, types::f32 zFar);
		~cMatrix4() = default;

		cMatrix4 operator*(const cMatrix4& mat) const;

	private:
		glm::mat4 _mat = {};
	};

	class cMath : public iObject
	{
		TRITON_OBJECT(cMath)

	public:
		explicit cMath(cContext* context);
		virtual ~cMath() override final = default;

		static types::f32 DegreesToRadians(types::f32 degrees);
		static types::cpuword Hash(const types::u8* data, types::usize dataByteSize);
	};
}