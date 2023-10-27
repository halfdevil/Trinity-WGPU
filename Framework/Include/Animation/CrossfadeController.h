#pragma once

#include "Animation/AnimationPose.h"

namespace Trinity
{
	class AnimationClip;
	class Skeleton;

	class CrossfadeController
	{
	public:

		struct Target
		{
			AnimationPose pose;
			AnimationClip* clip{ nullptr };
			float time{ 0.0f };
			float duration{ 0.0f };
			float elapsed{ 0.0f };
		};

		CrossfadeController() = default;
		CrossfadeController(Skeleton& skeleton);

		const AnimationClip* getClip() const
		{
			return mClip;
		}

		const AnimationPose& getPose() const
		{
			return mPose;
		}

		void play(AnimationClip& targetClip);
		void fadeTo(AnimationClip& targetClip, float fadeTime);
		void setSkeleton(Skeleton& skeleton);
		void update(bool looping, float deltaTime);

	private:

		std::vector<Target> mTargets;
		AnimationClip* mClip{ nullptr };
		Skeleton* mSkeleton{ nullptr };
		AnimationPose mPose;
		float mTime{ 0.0f };
	};
}