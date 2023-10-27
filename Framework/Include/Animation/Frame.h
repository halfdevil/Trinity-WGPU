#pragma once

#include "glm/glm.hpp"

namespace Trinity
{
	template <typename T>
	struct Frame
	{
		T value;
		T in;
		T out;
		float time;
	};

	using FrameScalar = Frame<float>;
	using FrameVector = Frame<glm::vec3>;
	using FrameQuaternion = Frame<glm::quat>;
}