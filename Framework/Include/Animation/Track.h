#pragma once

#include "Animation/Frame.h"
#include "Math/Math.h"
#include <vector>

namespace Trinity
{
	enum class Interpolation
	{
		Constant,
		Linear,
		Cubic
	};

	template <typename T>
	class Track
	{
	public:

		Track() = default;

		Interpolation getInterpolation() const
		{
			return mInterpolation;
		}

		uint32_t getNumFrames() const
		{
			return (uint32_t)mFrames.size();
		}

		const std::vector<Frame<T>>& getFrames() const
		{
			return mFrames;
		}

		void setInterpolation(Interpolation interpolation)
		{
			mInterpolation = interpolation;
		}

		void resize(uint32_t newSize)
		{
			mFrames.resize(newSize);
		}

		float getStartTime() const
		{
			return mFrames[0].time;
		}

		float getEndTime() const
		{
			return mFrames[mFrames.size() - 1].time;
		}

		T sample(float time, bool looping) const
		{
			switch (mInterpolation)
			{
			case Interpolation::Constant:
				return sampleConstant(time, looping);

			case Interpolation::Linear:
				return sampleLinear(time, looping);

			case Interpolation::Cubic:
				return sampleCubic(time, looping);
			}

			return T();
		}

		const Frame<T>& operator[](uint32_t idx) const
		{
			return mFrames[idx];
		}

		Frame<T>& operator[](uint32_t idx)
		{
			return mFrames[idx];
		}

		void setFrames(std::vector<Frame<T>>&& frames)
		{
			mFrames = std::move(frames);
		}

	protected:

		T sampleConstant(float time, bool looping) const
		{
			uint32_t frame = 0;
			if (!getFrameIndex(time, looping, frame))
			{
				return T();
			}

			return mFrames[frame].value;
		}

		T sampleLinear(float time, bool looping) const
		{
			uint32_t frame = 0;
			if (!getFrameIndex(time, looping, frame))
			{
				return T();
			}

			uint32_t nextFrame = frame + 1;
			float trackTime = adjustTime(time, looping);
			float frameDelta = mFrames[nextFrame].time - mFrames[frame].time;

			if (frameDelta <= 0.0f)
			{
				return T();
			}

			float t = (trackTime - mFrames[frame].time) / frameDelta;

			T start = mFrames[frame].value;
			T end = mFrames[nextFrame].value;

			return Math::interpolate(start, end, t);
		}

		T sampleCubic(float time, bool looping) const
		{
			uint32_t frame = 0;
			if (!getFrameIndex(time, looping, frame))
			{
				return T();
			}

			uint32_t nextFrame = frame + 1;
			float trackTime = adjustTime(time, looping);
			float frameDelta = mFrames[nextFrame].time - mFrames[frame].time;

			if (frameDelta <= 0.0f)
			{
				return T();
			}

			float t = (trackTime - mFrames[frame].time) / frameDelta;

			T p1 = mFrames[frame].value;
			T s1 = mFrames[frame].out;
			T p2 = mFrames[nextFrame].value;
			T s2 = mFrames[nextFrame].in;

			return Math::Hermite(t, p1, s1, p2, s2);
		}

		bool getFrameIndex(float time, bool looping, uint32_t& idx) const
		{
			uint32_t size = (uint32_t)mFrames.size();
			if (size <= 1)
			{
				return false;
			}

			if (looping)
			{
				float startTime = mFrames[0].time;
				float endTime = mFrames[size - 1].time;
				float duration = endTime - startTime;

				time = fmodf(time - startTime, endTime - startTime);
				if (time < 0.0f)
				{
					time += endTime - startTime;
				}
				time += startTime;
			}
			else
			{
				if (time <= mFrames[0].time)
				{
					idx = 0;
					return true;
				}

				if (time >= mFrames[size - 2].time)
				{
					idx = size - 2;
					return true;
				}
			}

			for (uint32_t i = size - 1; i >= 0; i--)
			{
				if (time >= mFrames[i].time)
				{
					idx = i;
					return true;
				}
			}

			return false;
		}

		float adjustTime(float time, bool looping) const
		{
			uint32_t size = (uint32_t)mFrames.size();
			if (size <= 1)
			{
				return 0.0f;
			}

			float startTime = mFrames[0].time;
			float endTime = mFrames[size - 1].time;
			float duration = endTime - startTime;

			if (duration <= 0.0f)
			{
				return 0.0f;
			}

			if (looping)
			{
				time = fmodf(time - startTime, endTime - startTime);
				if (time < 0.0f)
				{
					time += endTime - startTime;
				}
				time += startTime;
			}
			else
			{
				if (time <= startTime)
				{
					time = startTime;
				}

				if (time >= endTime)
				{
					time = endTime;
				}
			}

			return time;
		}

	protected:

		Interpolation mInterpolation{ Interpolation::Linear };
		std::vector<Frame<T>> mFrames;
	};

	using TrackScalar = Track<float>;
	using TrackVector = Track<glm::vec3>;
	using TrackQuaternion = Track<glm::quat>;
}