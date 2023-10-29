#pragma once

#include "SampleApplication.h"
#include "Graphics/RenderPass.h"

namespace Trinity
{
	class FreeCamera;
	class Terrain;
	class TerrainRenderer;

    class TerrainViewerSample : public SampleApplication
    {
    public:

		TerrainViewerSample() = default;
		virtual ~TerrainViewerSample() = default;

		TerrainViewerSample(const TerrainViewerSample&) = delete;
		TerrainViewerSample& operator = (const TerrainViewerSample&) = delete;

		TerrainViewerSample(TerrainViewerSample&&) = default;
		TerrainViewerSample& operator = (TerrainViewerSample&&) = default;

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
		std::unique_ptr<Terrain> mTerrain{ nullptr };
		std::unique_ptr<TerrainRenderer> mTerrainRenderer{ nullptr };
    };
}