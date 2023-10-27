#include "Scene/Components/Scripts/Animator.h"
#include "Scene/Components/Mesh.h"
#include "Scene/Model.h"
#include "Animation/AnimationClip.h"
#include "Animation/Skeleton.h"

namespace Trinity
{
	void Animator::init()
	{
		Script::init();
	}

	void Animator::update(float deltaTime)
	{
		Script::update(deltaTime);

		mCrossfadeController.update(mLooping, deltaTime);

		const auto& pose = mCrossfadeController.getPose();
		auto& bindPose = mMesh->getBindPose();

		pose.getMatrixPalette(bindPose);
	}

	std::string Animator::getTypeStr() const
	{
		return getStaticType();
	}

	void Animator::setMesh(Mesh& mesh)
	{
		if (!mesh.isAnimated())
		{
			return;
		}

		mMesh = &mesh;
		mModel = mesh.getModel();
		mCrossfadeController.setSkeleton(*mModel->getSkeleton());

		const auto& pose = mCrossfadeController.getPose();
		auto& bindPose = mMesh->getBindPose();

		pose.getMatrixPalette(bindPose);
	}

	void Animator::setLooping(bool looping)
	{
		mLooping = looping;
	}

	void Animator::setCurrentClip(uint32_t clipIndex)
	{
		auto& clips = mModel->getClips();
		if (clipIndex < (uint32_t)clips.size())
		{
			if (mCurrentClip != nullptr)
			{
				mCurrentClip = clips[clipIndex];
				mCrossfadeController.fadeTo(*mCurrentClip, mFadeTime);
			}
			else
			{
				mCurrentClip = clips[clipIndex];
				mCrossfadeController.play(*mCurrentClip);
			}
		}
	}

	std::string Animator::getStaticType()
	{
		return "Animator";
	}
}