#include "Animation/CrossfadeController.h"
#include "Animation/AnimationClip.h"
#include "Animation/Skeleton.h"

namespace Trinity
{
	CrossfadeController::CrossfadeController(Skeleton& skeleton)
	{
		setSkeleton(skeleton);
	}

	void CrossfadeController::play(AnimationClip& targetClip)
	{
		mTargets.clear();
		mClip = &targetClip;
		mTime = targetClip.getStartTime();
	}

	void CrossfadeController::fadeTo(AnimationClip& targetClip, float fadeTime)
	{
		if (!mClip)
		{
			play(targetClip);
			return;
		}

		if (mTargets.size() >= 1)
		{
			if (mTargets[mTargets.size() - 1].clip == &targetClip)
			{
				return;
			}
		}
		else
		{
			if (mClip == &targetClip)
			{
				return;
			}
		}

		mTargets.push_back({
			.pose = *mSkeleton->getRestPose(),
			.clip = &targetClip,
			.duration = fadeTime
		});
	}

	void CrossfadeController::setSkeleton(Skeleton& skeleton)
	{
		mSkeleton = &skeleton;
		mPose = *mSkeleton->getRestPose();
	}

	void CrossfadeController::update(bool looping, float deltaTime)
	{
		if (!mClip || !mSkeleton)
		{
			return;
		}

		uint32_t numTargets = (uint32_t)mTargets.size();
		for (uint32_t idx = 0; idx < numTargets; idx++)
		{
			const auto& target = mTargets[idx];
			if (target.elapsed >= target.duration)
			{
				mClip = target.clip;
				mTime = target.time;
				mPose = target.pose;

				mTargets.erase(mTargets.begin() + idx);
			}
		}

		mTime += (deltaTime / 1000.0f) * mClip->getTicksPerSecond();
		mTime = mClip->sample(mTime, looping, mPose);

		for (auto& target : mTargets)
		{
			target.time += deltaTime * target.clip->getTicksPerSecond();
			target.time = target.clip->sample(target.time, looping, target.pose);
			target.elapsed += deltaTime;

			float t = target.elapsed / target.duration;
			if (t > 1.0f)
			{
				t = 1.0f;
			}

			AnimationPose::blend(mPose, mPose, target.pose, t, -1);
		}
	}
}