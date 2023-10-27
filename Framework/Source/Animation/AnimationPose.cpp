#include "Animation/AnimationPose.h"
#include "Animation/Skeleton.h"
#include "VFS/FileSystem.h"

namespace Trinity
{
	AnimationPose::AnimationPose(uint32_t numJoints)
	{
		mJoints.resize(numJoints);
		mParents.resize(numJoints);
	}

	AnimationTransform AnimationPose::getLocalTransform(uint32_t idx) const
	{
		return mJoints[idx];
	}

	AnimationTransform AnimationPose::getGlobalTransform(uint32_t idx) const
	{
		glm::mat4 result = mJoints[idx].toMatrix();
		int32_t parent = mParents[idx];

		while (parent >= 0)
		{
			result = mJoints[parent].toMatrix() * result;
			parent = mParents[parent];
		}

		return AnimationTransform(result);
	}

	int32_t AnimationPose::getParent(uint32_t idx) const
	{
		return mParents[idx];
	}

	glm::dualquat AnimationPose::getGlobalDualQuat(uint32_t idx) const
	{
		glm::dualquat result = mJoints[idx].toDualQuat();
		int32_t p = mParents[idx];

		while (p >= 0)
		{
			glm::dualquat parent = mJoints[p].toDualQuat();
			result = result * parent;

			p = mParents[p];
		}

		return result;
	}

	void AnimationPose::getMatrixPalette(std::vector<glm::mat4>& out) const
	{
		uint32_t numJoints = getNumJoints();
		if (numJoints != (uint32_t)out.size())
		{
			out.resize(numJoints);
		}
				
		for (uint32_t idx = 0; idx < numJoints; idx++)
		{
			int32_t parent = mParents[idx];			
			glm::mat4 result = mJoints[idx].toMatrix();

			if (parent >= 0)
			{
				result = out[parent] * result;
			}

			out[idx] = result;
		}
	}

	void AnimationPose::getDualQuatPalette(std::vector<glm::dualquat>& out) const
	{
		uint32_t numJoints = getNumJoints();
		if (numJoints != (uint32_t)out.size())
		{
			out.resize(numJoints);
		}

		for (uint32_t idx = 0; idx < numJoints; idx++)
		{
			glm::dualquat result = mJoints[idx].toDualQuat();
			int32_t parent = mParents[idx];

			if (parent >= 0)
			{
				result = result * out[parent];
			}

			out[idx] = result;
		}
	}

	void AnimationPose::setParent(uint32_t idx, int32_t parent)
	{
		mParents[idx] = parent;
	}

	void AnimationPose::setLocalTransform(uint32_t idx, const AnimationTransform& transform)
	{
		mJoints[idx] = transform;
	}

	AnimationTransform AnimationPose::operator[](uint32_t idx) const
	{
		return getGlobalTransform(idx);
	}

	bool AnimationPose::operator==(const AnimationPose& p) const
	{
		if (mJoints.size() != p.mJoints.size())
		{
			return false;
		}

		if (mParents.size() != p.mParents.size())
		{
			return false;
		}

		uint32_t numJoints = (uint32_t)mJoints.size();
		for (uint32_t idx = 0; idx < numJoints; idx++)
		{
			AnimationTransform aTransform = mJoints[idx];
			AnimationTransform bTransform = p.mJoints[idx];

			int32_t aParent = mParents[idx];
			int32_t bParent = p.mParents[idx];

			if (aParent != bParent)
			{
				return false;
			}

			if (aTransform != bTransform)
			{
				return false;
			}
		}

		return true;
	}

	bool AnimationPose::operator!=(const AnimationPose& p) const
	{
		return !(*this == p);
	}

	bool AnimationPose::read(FileReader& reader)
	{
		uint32_t numJoints{ 0 };
		reader.read(&numJoints);

		mJoints.resize(numJoints);
		mParents.resize(numJoints);

		reader.read(mJoints.data(), numJoints);
		reader.read(mParents.data(), numJoints);

		return true;
	}

	bool AnimationPose::write(FileWriter& writer)
	{
		const uint32_t numJoints = (uint32_t)mJoints.size();
		writer.write(&numJoints);

		writer.write(mJoints.data(), numJoints);
		writer.write(mParents.data(), numJoints);

		return true;
	}

	bool AnimationPose::isInHierarchy(const AnimationPose& pose, int32_t parent, int32_t search)
	{
		if (search == parent)
		{
			return true;
		}

		int32_t p = pose.getParent(search);

		while (p >= 0)
		{
			if (p == parent)
			{
				return true;
			}

			p = pose.getParent(p);
		}

		return false;
	}

	void AnimationPose::blend(AnimationPose& outPose, const AnimationPose& poseA, const AnimationPose& poseB, 
		float t, int32_t blendRoot)
	{
		uint32_t numJoints = outPose.getNumJoints();
		for (uint32_t idx = 0; idx < numJoints; idx++)
		{
			if (blendRoot >= 0)
			{
				if (!isInHierarchy(outPose, blendRoot, (int32_t)idx))
				{
					continue;
				}
			}

			outPose.setLocalTransform(idx, AnimationTransform::lerp(poseA.getLocalTransform(idx), 
				poseB.getLocalTransform(idx), t));
		}
	}
}