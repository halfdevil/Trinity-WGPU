#pragma once

#include "SampleApplication.h"
#include "Graphics/RenderPass.h"

namespace Trinity
{
	class FreeCamera;
	class Animator;

    class RenderPassSample : public SampleApplication
    {
    public:

		RenderPassSample() = default;
		virtual ~RenderPassSample() = default;

		RenderPassSample(const RenderPassSample&) = delete;
		RenderPassSample& operator = (const RenderPassSample&) = delete;

		RenderPassSample(RenderPassSample&&) = default;
		RenderPassSample& operator = (RenderPassSample&&) = default;

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
    };
}