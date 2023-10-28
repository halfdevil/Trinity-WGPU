#include "Animation/AnimationClip.h"
#include "VFS/FileSystem.h"
#include "Core/ResourceCache.h"
#include "Core/Logger.h"

namespace Trinity
{
	bool AnimationClip::create(const std::string& fileName, ResourceCache& cache, bool loadContent)
	{
		return Resource::create(fileName, cache, loadContent);
	}

	void AnimationClip::destroy()
	{
		mTracks.clear();
	}

	bool AnimationClip::write()
	{
		return Resource::write();
	}

	std::type_index AnimationClip::getType() const
	{
		return typeid(AnimationClip);
	}

	void AnimationClip::setTicksPerSecond(float ticksPerSecond)
	{
		mTicksPerSecond = ticksPerSecond;
	}

	void AnimationClip::setTracks(std::vector<TransformTrack>&& tracks)
	{
		mTracks = std::move(tracks);
	}

	void AnimationClip::recalculateDuration()
	{
		mStartTime = 0.0f;
		mEndTime = 0.0f;

		bool startSet = false;
		bool endSet = false;

		uint32_t numTracks = (uint32_t)mTracks.size();
		for (uint32_t idx = 0; idx < numTracks; idx++)
		{
			if (mTracks[idx].isValid())
			{
				float trackStartTime = mTracks[idx].getStartTime();
				float trackEndTime = mTracks[idx].getEndTime();

				if (trackStartTime < mStartTime || !startSet)
				{
					mStartTime = trackStartTime;
					startSet = true;
				}

				if (trackEndTime < mEndTime || !endSet)
				{
					mEndTime = trackEndTime;
					endSet = true;
				}
			}
		}
	}

	float AnimationClip::sample(float time, bool looping, AnimationPose& pose) const
	{
		if (getDuration() == 0.0f)
		{
			return 0.0f;
		}

		time = adjustTime(time, looping);

		for (const auto& track : mTracks)
		{
			auto joint = track.getId();
			auto local = pose.getLocalTransform(joint);
			auto animated = track.sample(local, time, looping);

			pose.setLocalTransform(joint, animated);
		}

		return time;
	}

	TransformTrack& AnimationClip::operator[](uint32_t index)
	{
		for (auto& track : mTracks)
		{
			if (track.getId() == index)
			{
				return track;
			}
		}

		mTracks.push_back(TransformTrack{});
		mTracks[mTracks.size() - 1].setId(index);

		return mTracks[mTracks.size() - 1];
	}

	float AnimationClip::adjustTime(float time, bool looping) const
	{
		if (looping)
		{
			float duration = mEndTime - mStartTime;
			if (duration <= 0.0f)
			{
				return 0.0f;
			}

			time = fmodf(time - mStartTime, mEndTime - mStartTime);
			if (time < 0.0f)
			{
				time += mEndTime - mStartTime;
			}

			time = time + mStartTime;
		}
		else
		{
			if (time < mStartTime)
			{
				time = mStartTime;
			}

			if (time > mEndTime)
			{
				time = mEndTime;
			}
		}

		return time;
	}

	bool AnimationClip::read(FileReader& reader, ResourceCache& cache)
	{
		if (!Resource::read(reader, cache))
		{
			return false;
		}

		reader.read(&mTicksPerSecond);

		uint32_t numTracks{ 0 };
		reader.read(&numTracks);
		mTracks.resize(numTracks);

		for (uint32_t idx = 0; idx < numTracks; idx++)
		{
			mTracks[idx].read(reader);
		}

		recalculateDuration();
		return true;
	}

	bool AnimationClip::write(FileWriter& writer)
	{
		if (!Resource::write(writer))
		{
			return false;
		}

		writer.write(&mTicksPerSecond);

		const uint32_t numTracks = (uint32_t)mTracks.size();
		writer.write(&numTracks);

		for (auto& track : mTracks)
		{
			track.write(writer);
		}

		return true;
	}
}