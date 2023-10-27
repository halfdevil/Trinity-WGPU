#pragma once

#include "Core/Resource.h"
#include "Animation/TransformTrack.h"
#include "Animation/AnimationPose.h"

namespace Trinity
{
	class AnimationClip : public Resource
	{
	public:

		AnimationClip() = default;
		virtual ~AnimationClip() = default;

		AnimationClip(const AnimationClip&) = delete;
		AnimationClip& operator = (const AnimationClip&) = delete;

		AnimationClip(AnimationClip&&) noexcept = default;
		AnimationClip& operator = (AnimationClip&&) noexcept = default;

		float getTicksPerSecond() const
		{
			return mTicksPerSecond;
		}

		const std::vector<TransformTrack>& getTracks() const
		{
			return mTracks;
		}

		float getStartTime() const
		{
			return mStartTime;
		}

		float getEndTime() const
		{
			return mEndTime;
		}

		float getDuration() const
		{
			return mEndTime - mStartTime;
		}

		virtual void destroy() override;
		virtual std::type_index getType() const override;

		virtual void setTicksPerSecond(float ticksPerSecond);
		virtual void setTracks(std::vector<TransformTrack>&& tracks);
		virtual void recalculateDuration();
		virtual float sample(float time, bool looping, AnimationPose& pose) const;

		TransformTrack& operator[](uint32_t index);

		using Resource::create;
		using Resource::write;

	protected:

		virtual float adjustTime(float time, bool looping) const;
		virtual bool read(FileReader& reader, ResourceCache& cache) override;
		virtual bool write(FileWriter& writer) override;

	protected:

		float mTicksPerSecond{ 1.0f };
		std::vector<TransformTrack> mTracks;
		float mStartTime{ 0.0f };
		float mEndTime{ 0.0f };
	};
}