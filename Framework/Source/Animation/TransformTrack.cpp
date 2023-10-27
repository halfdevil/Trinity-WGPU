#include "Animation/TransformTrack.h"
#include "VFS/FileSystem.h"

namespace Trinity
{
	float TransformTrack::getStartTime() const
	{
		float result = 0.0f;
		bool isSet = false;

		if (mPosition.getNumFrames() > 1)
		{
			result = mPosition.getStartTime();
			isSet = true;
		}

		if (mRotation.getNumFrames() > 1)
		{
			const float startTime = mRotation.getStartTime();
			if (startTime < result || !isSet)
			{
				result = startTime;
				isSet = true;
			}
		}

		if (mScale.getNumFrames() > 1)
		{
			const float startTime = mScale.getStartTime();
			if (startTime < result || !isSet)
			{
				result = startTime;
				isSet = true;
			}
		}

		return result;
	}

	float TransformTrack::getEndTime() const
	{
		float result = 0.0f;
		bool isSet = false;

		if (mPosition.getNumFrames() > 1)
		{
			result = mPosition.getEndTime();
			isSet = true;
		}

		if (mRotation.getNumFrames() > 1)
		{
			const float endTime = mRotation.getEndTime();
			if (endTime > result || !isSet)
			{
				result = endTime;
				isSet = true;
			}
		}

		if (mScale.getNumFrames() > 1)
		{
			const float endTime = mScale.getEndTime();
			if (endTime > result || !isSet)
			{
				result = endTime;
				isSet = true;
			}
		}

		return result;
	}

	bool TransformTrack::isValid() const
	{
		return mPosition.getNumFrames() > 1 || mRotation.getNumFrames() > 1 ||
			mScale.getNumFrames() > 1;
	}

	void TransformTrack::setId(uint32_t id)
	{
		mId = id;
	}

	AnimationTransform TransformTrack::sample(const AnimationTransform& ref, float time, bool looping) const
	{
		AnimationTransform result = ref;

		if (mPosition.getNumFrames() > 1)
		{
			result.translation = mPosition.sample(time, looping);
		}

		if (mRotation.getNumFrames() > 1)
		{
			result.rotation = mRotation.sample(time, looping);
		}

		if (mScale.getNumFrames() > 1)
		{
			result.scale = mScale.sample(time, looping);
		}

		return result;
	}

	bool TransformTrack::read(FileReader& reader)
	{
		std::vector<FrameVector> positions;
		std::vector<FrameQuaternion> rotations;
		std::vector<FrameVector> scales;

		reader.read(&mId);
		reader.readVector(positions);
		reader.readVector(rotations);
		reader.readVector(scales);

		mPosition.setFrames(std::move(positions));
		mRotation.setFrames(std::move(rotations));
		mScale.setFrames(std::move(scales));

		return true;
	}

	bool TransformTrack::write(FileWriter& writer)
	{
		writer.write(&mId);
		writer.writeVector(mPosition.getFrames());
		writer.writeVector(mRotation.getFrames());
		writer.writeVector(mScale.getFrames());

		return true;
	}
}