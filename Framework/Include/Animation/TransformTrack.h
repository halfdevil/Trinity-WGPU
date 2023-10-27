#pragma once

#include "Animation/Track.h"
#include "Animation/AnimationTransform.h"

namespace Trinity
{
	class FileReader;
	class FileWriter;

	class TransformTrack
	{
	public:

		TransformTrack() = default;

		uint32_t getId() const
		{
			return mId;
		}

		TrackVector& getPosition()
		{
			return mPosition;
		}

		TrackQuaternion& getRotation()
		{
			return mRotation;
		}

		TrackVector& getScale()
		{
			return mScale;
		}

		float getStartTime() const;
		float getEndTime() const;

		bool isValid() const;
		void setId(uint32_t id);

		AnimationTransform sample(const AnimationTransform& ref, float time, bool looping) const;

		bool read(FileReader& reader);
		bool write(FileWriter& writer);

	private:

		uint32_t mId{ 0 };
		TrackVector mPosition;
		TrackQuaternion mRotation;
		TrackVector mScale;
	};
}