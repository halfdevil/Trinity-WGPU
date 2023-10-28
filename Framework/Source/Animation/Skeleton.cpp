#include "Animation/Skeleton.h"
#include "Animation/AnimationPose.h"
#include "VFS/FileSystem.h"
#include "Core/ResourceCache.h"
#include "Core/Logger.h"

namespace Trinity
{
	bool Skeleton::create(const std::string& fileName, ResourceCache& cache, bool loadContent)
	{
		return Resource::create(fileName, cache, loadContent);
	}

	void Skeleton::destroy()
	{
		mRestPose = nullptr;
		mBindPose = nullptr;
		mJointNames.clear();
		mInvBindPose.clear();
	}

	bool Skeleton::write()
	{
		return Resource::write();
	}

	std::type_index Skeleton::getType() const
	{
		return typeid(Skeleton);
	}

	void Skeleton::setRestPose(std::unique_ptr<AnimationPose>&& restPose)
	{
		mRestPose = std::move(restPose);
	}

	void Skeleton::setBindPose(std::unique_ptr<AnimationPose>&& bindPose)
	{
		mBindPose = std::move(bindPose);
	}

	void Skeleton::setInvBindPose(std::vector<glm::mat4>&& invBindPose)
	{
		mInvBindPose = std::move(invBindPose);
	}

	void Skeleton::setJointNames(std::vector<std::string>&& jointNames)
	{
		mJointNames = std::move(jointNames);
	}

	void Skeleton::updateInvBindPose()
	{
		uint32_t numJoints = mBindPose->getNumJoints();
		
		mInvBindPose.resize(numJoints);
		mBindPose->getMatrixPalette(mInvBindPose);

		for (uint32_t idx = 0; idx < numJoints; idx++)
		{
			mInvBindPose[idx] = glm::inverse(mInvBindPose[idx]);
		}
	}

	bool Skeleton::read(FileReader& reader, ResourceCache& cache)
	{
		uint32_t numBones{ 0 };
		reader.read(&numBones);
		mJointNames.resize(numBones);

		for (uint32_t idx = 0; idx < numBones; idx++)
		{
			mJointNames[idx] = reader.readString();
		}

		mRestPose = std::make_unique<AnimationPose>();
		mRestPose->read(reader);

		mBindPose = std::make_unique<AnimationPose>();
		mBindPose->read(reader);

		updateInvBindPose();

		return true;
	}

	bool Skeleton::write(FileWriter& writer)
	{
		const uint32_t numBones = (uint32_t)mJointNames.size();
		writer.write(&numBones);

		for (const auto& name : mJointNames)
		{
			writer.writeString(name);
		}

		if (mRestPose != nullptr)
		{
			mRestPose->write(writer);
		}

		if (mBindPose != nullptr)
		{
			mBindPose->write(writer);
		}

		return true;
	}
}