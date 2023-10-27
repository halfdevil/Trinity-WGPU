#pragma once

#include "Scene/Components/Script.h"
#include "Animation/CrossfadeController.h"

namespace Trinity
{
	class Mesh;
	class Model;

	class Animator : public Script
	{
	public:

		Animator() = default;
		virtual ~Animator() = default;

		Animator(const Animator&) = delete;
		Animator& operator = (const Animator&) = delete;

		Animator(Animator&&) = default;
		Animator& operator = (Animator&&) = default;

		Mesh* getMesh() const
		{
			return mMesh;
		}

		Model* getModel() const
		{
			return mModel;
		}

		bool isLooping() const
		{
			return mLooping;
		}

		AnimationClip* getCurrentClip() const
		{
			return mCurrentClip;
		}

		virtual void init() override;
		virtual void update(float deltaTime) override;
		virtual std::string getTypeStr() const override;

		virtual void setMesh(Mesh& mesh);
		virtual void setLooping(bool looping);
		virtual void setCurrentClip(uint32_t clipIndex);

	public:

		static std::string getStaticType();

	protected:

		Mesh* mMesh{ nullptr };
		Model* mModel{ nullptr };
		bool mLooping{ false };
		float mFadeTime{ 0.0f };
		AnimationClip* mCurrentClip{ nullptr };
		CrossfadeController mCrossfadeController;
	};
}