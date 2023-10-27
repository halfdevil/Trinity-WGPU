#pragma once

#include "Animation/AnimationTransform.h"
#include <vector>

namespace Trinity
{
	class Skeleton;
	class FileReader;
	class FileWriter;

	class AnimationPose
	{
	public:

		AnimationPose() = default;
		AnimationPose(uint32_t numJoints);

		AnimationPose(const AnimationPose&) = default;
		AnimationPose& operator = (const AnimationPose&) = default;

		uint32_t getNumJoints() const
		{
			return (uint32_t)mJoints.size();
		}

		AnimationTransform getLocalTransform(uint32_t idx) const;
		AnimationTransform getGlobalTransform(uint32_t idx) const;

		int32_t getParent(uint32_t idx) const;
		glm::dualquat getGlobalDualQuat(uint32_t idx) const;

		virtual void getMatrixPalette(std::vector<glm::mat4>& out) const;
		virtual void getDualQuatPalette(std::vector<glm::dualquat>& out) const;
		virtual void setParent(uint32_t idx, int32_t parent);
		virtual void setLocalTransform(uint32_t idx, const AnimationTransform& transform);

		virtual AnimationTransform operator[](uint32_t idx) const;
		virtual bool operator == (const AnimationPose& p) const;
		virtual bool operator != (const AnimationPose& p) const;

		virtual bool read(FileReader& reader);
		virtual bool write(FileWriter& writer);

	public:

		static bool isInHierarchy(const AnimationPose& pose, int32_t parent, int32_t search);
		static void blend(AnimationPose& outPose, const AnimationPose& poseA, const AnimationPose& poseB,
			float t, int32_t blendRoot);

	private:

		std::vector<AnimationTransform> mJoints;
		std::vector<int32_t> mParents;
	};
}