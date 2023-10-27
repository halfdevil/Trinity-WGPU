#pragma once

#include "SampleApplication.h"
#include "Graphics/RenderPass.h"

namespace Trinity
{
	class FreeCamera;
	class Animator;

    class ModelViewerSample : public SampleApplication
    {
    public:

		ModelViewerSample() = default;
		virtual ~ModelViewerSample() = default;

		ModelViewerSample(const ModelViewerSample&) = delete;
		ModelViewerSample& operator = (const ModelViewerSample&) = delete;

		ModelViewerSample(ModelViewerSample&&) = default;
		ModelViewerSample& operator = (ModelViewerSample&&) = default;

    protected:

		virtual bool init() override;
        virtual void render(float deltaTime) override;
		virtual void onSceneLoaded() override;
		virtual void setupInput() override;

		virtual void moveForward(float scale);
		virtual void moveRight(float scale);
		virtual void moveUp(float scale);
		virtual void turn(float scale);
		virtual void lookUp(float scale);

	protected:

		FreeCamera* mCamera{ nullptr };
		Animator* mAnimator{ nullptr };
    };
}