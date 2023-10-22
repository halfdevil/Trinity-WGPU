#pragma once

#include "Core/Application.h"

namespace Trinity
{
	class Scene;
	class SceneRenderer;
	class Script;

	class SampleApplication : public Application
	{
	public:

		SampleApplication() = default;
		virtual ~SampleApplication() = default;

		SampleApplication(const SampleApplication&) = delete;
		SampleApplication& operator = (const SampleApplication&) = delete;

		SampleApplication(SampleApplication&&) = default;
		SampleApplication& operator = (SampleApplication&&) = default;

		Scene* getScene() const
		{
			return mScene.get();
		}

		SceneRenderer* getSceneRenderer() const
		{
			return mSceneRenderer.get();
		}

		bool hasScene() const
		{
			return mScene != nullptr;
		}

	protected:

		virtual bool init() override;
		virtual void update(float deltaTime) override;
		virtual void onResize() override;
		virtual void onSceneLoaded();

	protected:

		std::unique_ptr<Scene> mScene{ nullptr };
		std::unique_ptr<SceneRenderer> mSceneRenderer{ nullptr };
		std::vector<Script*> mScripts;
	};
}