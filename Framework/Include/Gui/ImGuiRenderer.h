#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include "imgui.h"
#include "imgui_internal.h"

namespace Trinity
{
	class ImGuiFont;
	class RenderPass;
	class Texture;
	class Sampler;
	class ResourceCache;
	class Shader;
	class RenderPipeline;
	class UniformBuffer;
	class BindGroup;
	class BindGroupLayout;
	class VertexLayout;
	class VertexBuffer;
	class IndexBuffer;
	class SwapChain;
	class Window;

	class ImGuiRenderer
	{
	public:

		static constexpr const char* kDefaultShader = "/Assets/Framework/Shaders/Gui.wgsl";
		static constexpr uint32_t kCommonBindGroupIndex = 0;
		static constexpr uint32_t kTextureBindGroupIndex = 1;

		struct PerFrameData
		{
			glm::mat4 viewProj;
		};

		struct RenderContext
		{
			Shader* shader{ nullptr };
			RenderPipeline* pipeline{ nullptr };
			VertexLayout* vertexLayout{ nullptr };
			VertexBuffer* vertexBuffer{ nullptr };
			IndexBuffer* indexBuffer{ nullptr };
			UniformBuffer* perFrameBuffer{ nullptr };
			BindGroup* bindGroup{ nullptr };
			BindGroupLayout* bindGroupLayout{ nullptr };
		};

		struct ImageContext
		{
			ImGuiFont* font{ nullptr };
			Sampler* sampler{ nullptr };
			BindGroupLayout* bindGroupLayout{ nullptr };
			std::unordered_map<ImGuiID, BindGroup*> bindGroups;
		};

		struct StagingContext
		{
			uint32_t numVertices{ 0 };
			uint32_t numIndices{ 0 };
			std::vector<ImDrawVert> vertices;
			std::vector<ImDrawIdx> indices;
		};

		ImGuiRenderer() = default;
		~ImGuiRenderer();

		ImGuiRenderer(const ImGuiRenderer&) = delete;
		ImGuiRenderer& operator = (const ImGuiRenderer&) = delete;

		ImGuiRenderer(ImGuiRenderer&&) = default;
		ImGuiRenderer& operator = (ImGuiRenderer&&) = default;

		bool create(Window& window, const std::string& defaultFontPath);
		void destroy();

		void newFrame(Window& window, float deltaTime);
		void draw();

	protected:

		void setupCallbacks(Window& window);
		void updateMouseCursor(Window& window);
		bool createDeviceObjects();
		void setupRenderStates(ImDrawData* drawData);

		bool createCommonBindGroup();
		bool createImageBindGroup(const Texture& texture);
		bool createBufferData();

	protected:

		RenderContext mRenderContext;
		ImageContext mImageContext;
		StagingContext mStagingContext;
		std::unique_ptr<ResourceCache> mResourceCache{ nullptr };
		std::unique_ptr<RenderPass> mRenderPass{ nullptr };
		glm::vec2 mLastValidMousePos{ 0.0f };
	};
}