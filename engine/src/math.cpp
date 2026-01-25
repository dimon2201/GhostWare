// math.cpp

#include "math.hpp"

using namespace types;

namespace triton
{
	cVector2::cVector2(const glm::vec2& vec) : _vec(vec) {}

	cVector2::cVector2(f32 value) : _vec(glm::vec2(value, value)) {}

	cVector2::cVector2(f32 x, f32 y) : _vec(glm::vec2(x, y)) {}

	cVector2 cVector2::operator+(const cVector2& vec) const
	{
		return cVector2(_vec + vec);
	}

	cVector2 cVector2::operator-(const cVector2& vec) const
	{
		return cVector2(_vec - vec);
	}

	cVector2 cVector2::operator*(const cVector2& vec) const
	{
		return cVector2(_vec * vec);
	}

	cVector2 cVector2::operator/(const cVector2& vec) const
	{
		return cVector2(_vec / vec);
	}

	cVector2 cVector2::operator+(f32 val) const
	{
		return cVector2(_vec + val);
	}

	cVector2 cVector2::operator-(f32 val) const
	{
		return cVector2(_vec - val);
	}

	cVector2 cVector2::operator*(f32 val) const
	{
		return cVector2(_vec * val);
	}

	cVector2 cVector2::operator/(f32 val) const
	{
		return cVector2(_vec / val);
	}

	cVector3::cVector3(const glm::vec3& vec) : _vec(vec) {}

	cVector3::cVector3(f32 value) : _vec(glm::vec3(value, value, value)) {}

	cVector3::cVector3(f32 x, f32 y, f32 z) : _vec(glm::vec3(x, y, z)) {}

	cVector3 cVector3::operator+(const cVector3& vec) const
	{
		return cVector3(_vec + vec);
	}

	cVector3 cVector3::operator-(const cVector3& vec) const
	{
		return cVector3(_vec - vec);
	}

	cVector3 cVector3::operator*(const cVector3& vec) const
	{
		return cVector3(_vec * vec);
	}

	cVector3 cVector3::operator/(const cVector3& vec) const
	{
		return cVector3(_vec / vec);
	}

	cVector3 cVector3::operator+(f32 val) const
	{
		return cVector3(_vec + val);
	}

	cVector3 cVector3::operator-(f32 val) const
	{
		return cVector3(_vec - val);
	}

	cVector3 cVector3::operator*(f32 val) const
	{
		return cVector3(_vec * val);
	}

	cVector3 cVector3::operator/(f32 val) const
	{
		return cVector3(_vec / val);
	}

	cVector3 cVector3::Cross(const cVector3& axis)
	{
		_vec = glm::cross(_vec, glm::vec3(axis.GetX(), axis.GetY(), axis.GetZ()));

		return cVector3(_vec);
	}

	cVector4::cVector4(const glm::vec4& vec) : _vec(vec) {}

	cVector4::cVector4(f32 value) : _vec(glm::vec4(value, value, value, value)) {}

	cVector4::cVector4(f32 x, f32 y, f32 z, f32 w) : _vec(glm::vec3(x, y, z, w)) {}

	cVector4 cVector4::operator+(const cVector4& vec) const
	{
		return cVector4(_vec + vec);
	}

	cVector4 cVector4::operator-(const cVector4& vec) const
	{
		return cVector4(_vec - vec);
	}

	cVector4 cVector4::operator*(const cVector4& vec) const
	{
		return cVector4(_vec * vec);
	}

	cVector4 cVector4::operator/(const cVector4& vec) const
	{
		return cVector4(_vec / vec);
	}

	cVector4 cVector4::operator+(f32 val) const
	{
		return cVector4(_vec + val);
	}

	cVector4 cVector4::operator-(f32 val) const
	{
		return cVector4(_vec - val);
	}

	cVector4 cVector4::operator*(f32 val) const
	{
		return cVector4(_vec * val);
	}

	cVector4 cVector4::operator/(f32 val) const
	{
		return cVector4(_vec / val);
	}

	cQuaternion::cQuaternion(const glm::quat& quat) : _quat(quat) {}

	cQuaternion::cQuaternion(f32 angle, const cVector3& axis) : _quat(glm::angleAxis(angle, glm::vec3(axis.GetX(), axis.GetY(), axis.GetZ()))) {}

	cQuaternion::cQuaternion(types::f32 w, types::f32 x, types::f32 y, types::f32 z)
		: _quat(glm::quat(w, x, y, z)) {}

	cVector3 cQuaternion::operator*(const cVector3& vec) const
	{
		return cVector3(_quat * vec);
	}

	cQuaternion cQuaternion::operator*(const cQuaternion& quat) const
	{
		return cQuaternion(_quat * quat);
	}

	cVector3 cQuaternion::EulerAngles() const
	{
		return cVector3(glm::eulerAngles(q));
	}

	cMatrix4::cMatrix4(const glm::mat4& mat) : _mat(mat) {}

	cMatrix4::cMatrix4(f32 value) : _mat(glm::mat4(value)) {}

	cMatrix4::cMatrix4(const cVector3& position, const cVector3& direction, const cVector3& up)
		: _mat(glm::lookAtRH(position, position + direction, up)) {}

	cMatrix4::cMatrix4(f32 fov, f32 aspect, f32 zNear, f32 zFar) 
		: _mat(glm::perspective(cMath::DegreesToRadians(fov), aspect, zNear, zFar)) {}

	cMatrix4 cMatrix4::operator*(const cMatrix4& mat) const
	{
		return cMatrix4(_mat * mat);
	}

	void cTransform::Transform()
	{
		const glm::quat quatX = glm::angleAxis(_rotation.GetX(), glm::vec3(1.0f, 0.0f, 0.0f));
		const glm::quat quatY = glm::angleAxis(_rotation.GetY(), glm::vec3(0.0f, 1.0f, 0.0f));
		const glm::quat quatZ = glm::angleAxis(_rotation.GetZ(), glm::vec3(0.0f, 0.0f, 1.0f));
		_world = glm::translate(glm::mat4(1.0f), glm::vec3(_position.GetX(), _position.GetY(), _position.GetZ())) * glm::toMat4(quatZ * quatY * quatX) * glm::scale(glm::mat4(1.0f), glm::vec3(_scale.GetX(), _scale.GetY(), _scale.GetZ()));
	}

	cMath::cMath(cContext* context) : iObject(context) {}

	f32 cMath::DegreesToRadians(f32 degrees)
	{
		return glm::radians(degrees);
	}

	cpuword cMath::Hash(const u8* data, usize dataByteSize)
	{
		u64 hash = 0x9e3779b97f4a7c15ull;
		while (dataByteSize >= 4)
		{
			hash = (hash ^ (((u64)(*data++) * 0x9e3779b9ull) >> 32)) * 0xbf58476d1ce4e5b9ull;
			byteSize -= 4;
		}

		u32 tail = 0;
		memcpy(&tail, data, byteSize);
		hash ^= tail;

		return (cpuword)hash;
	}
}