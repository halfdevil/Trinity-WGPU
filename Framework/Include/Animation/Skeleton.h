#pragma once

#include "Core/Resource.h"
#include <vector>
#include <string>
#include <glm/glm.hpp>

namespace Trinity
{
	class AnimationPose;

	class Skeleton : public Resource
	{
	public:

		Skeleton() = default;
		virtual ~Skeleton() = default;

		Skeleton(const Skeleton&) = delete;
		Skeleton& operator = (const Skeleton&) = delete;

		Skeleton(Skeleton&&) noexcept = default;
		Skeleton& operator = (Skeleton&&) noexcept = default;

		AnimationPose* getBindPose() const
		{
			return mBindPose.get();
		}

		AnimationPose* getRestPose() const
		{
			return mRestPose.get();
		}

		const std::vector<std::string>& getJointNames() const
		{
			return mJointNames;
		}

		const std::vector<glm::mat4>& getInvBindPose() const
		{
			return mInvBindPose;
		}

		virtual void destroy() override;
		virtual std::type_index getType() const override;

		virtual void setRestPose(std::unique_ptr<AnimationPose>&& restPose);
		virtual void setBindPose(std::unique_ptr<AnimationPose>&& bindPose);
		virtual void setInvBindPose(std::vector<glm::mat4>&& invBindPose);
		virtual void setJointNames(std::vector<std::string>&& jointNames);
		virtual void updateInvBindPose();

		using Resource::create;
		using Resource::write;

	protected:

		virtual bool read(FileReader& reader, ResourceCache& cache) override;
		virtual bool write(FileWriter& writer) override;

	protected:

		std::unique_ptr<AnimationPose> mRestPose{ nullptr };
		std::unique_ptr<AnimationPose> mBindPose{ nullptr };
		std::vector<std::string> mJointNames;
		std::vector<glm::mat4> mInvBindPose;
	};
}