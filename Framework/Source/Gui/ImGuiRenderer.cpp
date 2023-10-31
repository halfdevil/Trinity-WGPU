#include "Gui/ImGuiRenderer.h"
#include "Gui/ImGuiFont.h"
#include "Graphics/RenderPass.h"
#include "Graphics/Texture2D.h"
#include "Graphics/Sampler.h"
#include "Graphics/Shader.h"
#include "Graphics/RenderPipeline.h"
#include "Graphics/UniformBuffer.h"
#include "Graphics/BindGroup.h"
#include "Graphics/BindGroupLayout.h"
#include "Graphics/VertexLayout.h"
#include "Graphics/VertexBuffer.h"
#include "Graphics/IndexBuffer.h"
#include "Graphics/UniformBuffer.h"
#include "Graphics/GraphicsDevice.h"
#include "Graphics/SwapChain.h"
#include "Input/Types.h"
#include "Core/Window.h"
#include "Core/Logger.h"
#include "Core/ResourceCache.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>

namespace Trinity
{
	static void updateKeyModifiers(int32_t mods)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddKeyEvent(ImGuiKey_ModCtrl, (mods & MOD_CONTROL) != 0);
		io.AddKeyEvent(ImGuiKey_ModShift, (mods & MOD_SHIFT) != 0);
		io.AddKeyEvent(ImGuiKey_ModAlt, (mods & MOD_ALT) != 0);
		io.AddKeyEvent(ImGuiKey_ModSuper, (mods & MOD_SUPER) != 0);
	}

	static ImGuiKey mapKey(int32_t key)
	{
		switch (key)
		{
			case KEY_TAB: return ImGuiKey_Tab;
			case KEY_LEFT: return ImGuiKey_LeftArrow;
			case KEY_RIGHT: return ImGuiKey_RightArrow;
			case KEY_UP: return ImGuiKey_UpArrow;
			case KEY_DOWN: return ImGuiKey_DownArrow;
			case KEY_PAGE_UP: return ImGuiKey_PageUp;
			case KEY_PAGE_DOWN: return ImGuiKey_PageDown;
			case KEY_HOME: return ImGuiKey_Home;
			case KEY_END: return ImGuiKey_End;
			case KEY_INSERT: return ImGuiKey_Insert;
			case KEY_DELETE: return ImGuiKey_Delete;
			case KEY_BACKSPACE: return ImGuiKey_Backspace;
			case KEY_SPACE: return ImGuiKey_Space;
			case KEY_ENTER: return ImGuiKey_Enter;
			case KEY_ESCAPE: return ImGuiKey_Escape;
			case KEY_APOSTROPHE: return ImGuiKey_Apostrophe;
			case KEY_COMMA: return ImGuiKey_Comma;
			case KEY_MINUS: return ImGuiKey_Minus;
			case KEY_PERIOD: return ImGuiKey_Period;
			case KEY_SLASH: return ImGuiKey_Slash;
			case KEY_SEMICOLON: return ImGuiKey_Semicolon;
			case KEY_EQUAL: return ImGuiKey_Equal;
			case KEY_LEFT_BRACKET: return ImGuiKey_LeftBracket;
			case KEY_BACKSLASH: return ImGuiKey_Backslash;
			case KEY_RIGHT_BRACKET: return ImGuiKey_RightBracket;
			case KEY_GRAVE_ACCENT: return ImGuiKey_GraveAccent;
			case KEY_CAPS_LOCK: return ImGuiKey_CapsLock;
			case KEY_SCROLL_LOCK: return ImGuiKey_ScrollLock;
			case KEY_NUM_LOCK: return ImGuiKey_NumLock;
			case KEY_PRINT_SCREEN: return ImGuiKey_PrintScreen;
			case KEY_PAUSE: return ImGuiKey_Pause;
			case KEY_KP_0: return ImGuiKey_Keypad0;
			case KEY_KP_1: return ImGuiKey_Keypad1;
			case KEY_KP_2: return ImGuiKey_Keypad2;
			case KEY_KP_3: return ImGuiKey_Keypad3;
			case KEY_KP_4: return ImGuiKey_Keypad4;
			case KEY_KP_5: return ImGuiKey_Keypad5;
			case KEY_KP_6: return ImGuiKey_Keypad6;
			case KEY_KP_7: return ImGuiKey_Keypad7;
			case KEY_KP_8: return ImGuiKey_Keypad8;
			case KEY_KP_9: return ImGuiKey_Keypad9;
			case KEY_KP_DECIMAL: return ImGuiKey_KeypadDecimal;
			case KEY_KP_DIVIDE: return ImGuiKey_KeypadDivide;
			case KEY_KP_MULTIPLY: return ImGuiKey_KeypadMultiply;
			case KEY_KP_SUBTRACT: return ImGuiKey_KeypadSubtract;
			case KEY_KP_ADD: return ImGuiKey_KeypadAdd;
			case KEY_KP_ENTER: return ImGuiKey_KeypadEnter;
			case KEY_KP_EQUAL: return ImGuiKey_KeypadEqual;
			case KEY_LEFT_SHIFT: return ImGuiKey_LeftShift;
			case KEY_LEFT_CONTROL: return ImGuiKey_LeftCtrl;
			case KEY_LEFT_ALT: return ImGuiKey_LeftAlt;
			case KEY_LEFT_SUPER: return ImGuiKey_LeftSuper;
			case KEY_RIGHT_SHIFT: return ImGuiKey_RightShift;
			case KEY_RIGHT_CONTROL: return ImGuiKey_RightCtrl;
			case KEY_RIGHT_ALT: return ImGuiKey_RightAlt;
			case KEY_RIGHT_SUPER: return ImGuiKey_RightSuper;
			case KEY_MENU: return ImGuiKey_Menu;
			case KEY_0: return ImGuiKey_0;
			case KEY_1: return ImGuiKey_1;
			case KEY_2: return ImGuiKey_2;
			case KEY_3: return ImGuiKey_3;
			case KEY_4: return ImGuiKey_4;
			case KEY_5: return ImGuiKey_5;
			case KEY_6: return ImGuiKey_6;
			case KEY_7: return ImGuiKey_7;
			case KEY_8: return ImGuiKey_8;
			case KEY_9: return ImGuiKey_9;
			case KEY_A: return ImGuiKey_A;
			case KEY_B: return ImGuiKey_B;
			case KEY_C: return ImGuiKey_C;
			case KEY_D: return ImGuiKey_D;
			case KEY_E: return ImGuiKey_E;
			case KEY_F: return ImGuiKey_F;
			case KEY_G: return ImGuiKey_G;
			case KEY_H: return ImGuiKey_H;
			case KEY_I: return ImGuiKey_I;
			case KEY_J: return ImGuiKey_J;
			case KEY_K: return ImGuiKey_K;
			case KEY_L: return ImGuiKey_L;
			case KEY_M: return ImGuiKey_M;
			case KEY_N: return ImGuiKey_N;
			case KEY_O: return ImGuiKey_O;
			case KEY_P: return ImGuiKey_P;
			case KEY_Q: return ImGuiKey_Q;
			case KEY_R: return ImGuiKey_R;
			case KEY_S: return ImGuiKey_S;
			case KEY_T: return ImGuiKey_T;
			case KEY_U: return ImGuiKey_U;
			case KEY_V: return ImGuiKey_V;
			case KEY_W: return ImGuiKey_W;
			case KEY_X: return ImGuiKey_X;
			case KEY_Y: return ImGuiKey_Y;
			case KEY_Z: return ImGuiKey_Z;
			case KEY_F1: return ImGuiKey_F1;
			case KEY_F2: return ImGuiKey_F2;
			case KEY_F3: return ImGuiKey_F3;
			case KEY_F4: return ImGuiKey_F4;
			case KEY_F5: return ImGuiKey_F5;
			case KEY_F6: return ImGuiKey_F6;
			case KEY_F7: return ImGuiKey_F7;
			case KEY_F8: return ImGuiKey_F8;
			case KEY_F9: return ImGuiKey_F9;
			case KEY_F10: return ImGuiKey_F10;
			case KEY_F11: return ImGuiKey_F11;
			case KEY_F12: return ImGuiKey_F12;
			default: return ImGuiKey_None;
		}
	}

	static Cursor mapCursor(ImGuiMouseCursor cursor)
	{
		switch (cursor)
		{
			case ImGuiMouseCursor_Arrow: return Cursor::Arrow;
			case ImGuiMouseCursor_TextInput: return Cursor::IBeam;
			case ImGuiMouseCursor_ResizeNS: return Cursor::VResize;
			case ImGuiMouseCursor_ResizeEW: return Cursor::HResize;
			case ImGuiMouseCursor_Hand: return Cursor::Hand;
			case ImGuiMouseCursor_ResizeAll: return Cursor::ResizeAll;
			case ImGuiMouseCursor_ResizeNESW: return Cursor::ResizeNESW;
			case ImGuiMouseCursor_ResizeNWSE: return Cursor::ResizeNWSE;
			case ImGuiMouseCursor_NotAllowed: return Cursor::NotAllowed;
			default: return Cursor::Arrow;
		}
	}

	ImGuiRenderer::~ImGuiRenderer()
	{
		destroy();
	}

	bool ImGuiRenderer::create(Window& window, const std::string& defaultFontPath)
	{
		ImGui::CreateContext();

		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowBorderSize = 0.0f;
		style.ScaleAllSizes(window.getScaleFactor());

		auto fbSize = window.getFramebufferSize();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
		io.DisplaySize.x = (float)fbSize.x;
		io.DisplaySize.y = (float)fbSize.y;
		io.FontGlobalScale = 1.0f;
		io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
		io.BackendRendererName = "TrinityGui";
		io.BackendPlatformName = "TrinityGui";

		auto font = std::make_unique<ImGuiFont>();
		if (!font->create("default_font", defaultFontPath))
		{
			LogError("Font::create() failed for '%s'", defaultFontPath.c_str());
			return false;
		}

		if (!font->build())
		{
			LogError("Font::build() failed for '%s'", defaultFontPath.c_str());
			return false;
		}

		mImageContext.font = font.get();
		mResourceCache = std::make_unique<ResourceCache>();
		mResourceCache->addResource(std::move(font));
		mRenderPass = std::make_unique<RenderPass>();

		if (!createDeviceObjects())
		{
			LogError("createDeviceObjects() failed!!");
			return false;
		}

		setupCallbacks(window);
		return true;
	}

	void ImGuiRenderer::destroy()
	{
		mResourceCache->clear();
		ImGui::DestroyContext();
	}

	void ImGuiRenderer::newFrame(Window& window, float deltaTime)
	{
		ImGuiIO& io = ImGui::GetIO();

		const glm::uvec2 size = window.getSize();
		const glm::uvec2 displaySize = window.getFramebufferSize();

		io.DeltaTime = deltaTime;
		io.DisplaySize = { (float)size.x, (float)size.y };

		if (displaySize.x > 0 && displaySize.y > 0)
		{
			io.DisplayFramebufferScale = { (float)displaySize.x / size.x ,
				(float)displaySize.y / size.y };
		}

		updateMouseCursor(window);
		ImGui::NewFrame();
	}

	void ImGuiRenderer::draw()
	{
		ImGui::Render();

		auto* drawData = ImGui::GetDrawData();
		if (drawData->DisplaySize.x <= 0.0f || drawData->DisplaySize.y <= 0.0f)
		{
			return;
		}

		mRenderPass->begin();

		if (mStagingContext.numVertices < (uint32_t)drawData->TotalVtxCount)
		{
			auto numVertices = drawData->TotalVtxCount + 5000;
			auto* vertexBuffer = mRenderContext.vertexBuffer;
			vertexBuffer->destroy();

			if (!vertexBuffer->create(*mRenderContext.vertexLayout, numVertices))
			{
				LogError("VertexBuffer::create() failed!!");
				return;
			}

			mStagingContext.vertices.resize(numVertices);
			mStagingContext.numVertices = numVertices;
		}

		if (mStagingContext.numIndices < (uint32_t)drawData->TotalIdxCount)
		{
			auto numIndices = drawData->TotalIdxCount + 10000;
			auto* indexBuffer = mRenderContext.indexBuffer;
			indexBuffer->destroy();

			if (!indexBuffer->create(wgpu::IndexFormat::Uint16, numIndices))
			{
				LogError("IndexBuffer::create() failed!!");
				return;
			}

			mStagingContext.indices.resize(numIndices);
			mStagingContext.numIndices = numIndices;
		}

		ImDrawVert* vtxDst = mStagingContext.vertices.data();
		ImDrawIdx* idxDst = mStagingContext.indices.data();

		for (int32_t idx = 0; idx < drawData->CmdListsCount; idx++)
		{
			const ImDrawList* cmdList = drawData->CmdLists[idx];

			memcpy(vtxDst, cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
			memcpy(idxDst, cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));

			vtxDst += cmdList->VtxBuffer.Size;
			idxDst += cmdList->IdxBuffer.Size;
		}

		uint32_t vbSize = ((char*)vtxDst - (char*)mStagingContext.vertices.data() + 3) & ~3;
		uint32_t ibSize = ((char*)idxDst - (char*)mStagingContext.indices.data() + 3) & ~3;

		mRenderContext.vertexBuffer->write(0, vbSize, mStagingContext.vertices.data());
		mRenderContext.indexBuffer->write(0, ibSize, mStagingContext.indices.data());

		setupRenderStates(drawData);

		const float width = drawData->DisplaySize.x * drawData->FramebufferScale.x;
		const float height = drawData->DisplaySize.y * drawData->FramebufferScale.y;
		const ImVec2 displaySize = ImVec2(width - 1, height - 1);

		uint32_t vtxOffset = 0;
		uint32_t idxOffset = 0;
		ImVec2 clipScale = drawData->FramebufferScale;
		ImVec2 clipOff = drawData->DisplayPos;

		for (uint32_t idx = 0; idx < (uint32_t)drawData->CmdListsCount; idx++)
		{
			const ImDrawList* cmdList = drawData->CmdLists[idx];
			for (uint32_t cmdIdx = 0; cmdIdx < (uint32_t)cmdList->CmdBuffer.Size; cmdIdx++)
			{
				const ImDrawCmd* cmd = &cmdList->CmdBuffer[cmdIdx];
				const auto& bindGroups = mImageContext.bindGroups;

				ImTextureID texId = cmd->GetTexID();
				ImGuiID texIdHash = ImHashData(texId, sizeof(ImTextureID));

				if (!bindGroups.contains(texIdHash))
				{
					createImageBindGroup(*(Texture*)(texId));
				}

				if (auto it = bindGroups.find(texIdHash); it != bindGroups.end())
				{
					const auto* bindGroup = it->second;
					mRenderPass->setBindGroup(kTextureBindGroupIndex, *bindGroup);
				}

				ImVec2 clipMin((cmd->ClipRect.x - clipOff.x) * clipScale.x, (cmd->ClipRect.y - clipOff.y) * clipScale.y);
				ImVec2 clipMax((cmd->ClipRect.z - clipOff.x) * clipScale.x, (cmd->ClipRect.w - clipOff.y) * clipScale.y);

				if (clipMax.x <= clipMin.x || clipMax.y <= clipMin.y)
				{
					continue;
				}

				clipMin = ImClamp(clipMin, ImVec2(), displaySize);
				clipMax = ImClamp(clipMax, ImVec2(), displaySize);

				mRenderPass->setScissor((uint32_t)clipMin.x, (uint32_t)clipMin.y, (uint32_t)(clipMax.x - clipMin.x),
					(uint32_t)(clipMax.y - clipMin.y));

				mRenderPass->drawIndexed(cmd->ElemCount, 1, cmd->IdxOffset + idxOffset,
					cmd->VtxOffset + vtxOffset, 0);
			}

			vtxOffset += cmdList->VtxBuffer.Size;
			idxOffset += cmdList->IdxBuffer.Size;
		}

		mRenderPass->end();
		mRenderPass->submit();
	}

	void ImGuiRenderer::setupCallbacks(Window& window)
	{
		auto& callbacks = window.getCallbacks();

		callbacks.onMouseButton.subscribe([](int button, bool action, int32_t mods) {
			updateKeyModifiers(mods);

			ImGuiIO& io = ImGui::GetIO();
			if (button >= 0 && button < ImGuiMouseButton_COUNT)
			{
				io.AddMouseButtonEvent(button, action);
			}
		});

		callbacks.onMouseScroll.subscribe([](float xoffset, float yoffset) {
			ImGuiIO& io = ImGui::GetIO();
			io.AddMouseWheelEvent(xoffset, yoffset);
		});

		callbacks.onKey.subscribe([](int32_t key, bool pressed, int32_t mods) {
			updateKeyModifiers(mods);

			ImGuiIO& io = ImGui::GetIO();
			ImGuiKey imKey = mapKey(key);
			io.AddKeyEvent(imKey, pressed);
		});

		callbacks.onFocus.subscribe([](bool focus) {
			ImGuiIO& io = ImGui::GetIO();
			io.AddFocusEvent(focus);
		});

		callbacks.onMousePos.subscribe([this](float x, float y) {
			ImGuiIO& io = ImGui::GetIO();
			io.AddMousePosEvent(x, y);
			mLastValidMousePos = { (float)x, (float)y };
		});

		callbacks.onMouseEnter.subscribe([this](bool entered) {
			ImGuiIO& io = ImGui::GetIO();

			if (entered)
			{
				io.AddMousePosEvent(mLastValidMousePos.x, mLastValidMousePos.y);
			}
			else
			{
				mLastValidMousePos = { io.MousePos.x, io.MousePos.y };
				io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
			}
		});

		callbacks.onChar.subscribe([](uint32_t c) {
			ImGuiIO& io = ImGui::GetIO();
			io.AddInputCharacter(c);
		});
	}

	void ImGuiRenderer::updateMouseCursor(Window& window)
	{
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
		{
			return;
		}

		ImGuiMouseCursor imguiCursor = ImGui::GetMouseCursor();
		if (imguiCursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
		{
			window.showMouse(false, false);
		}
		else
		{
			window.setCursor(mapCursor(imguiCursor));
			window.showMouse(true, false);
		}
	}

	bool ImGuiRenderer::createDeviceObjects()
	{
		if (!createBufferData())
		{
			LogError("createBufferData() failed!!");
			return false;
		}

		if (!createCommonBindGroup())
		{
			LogError("createCommonBindGroup() failed!!");
			return false;
		}

		if (mImageContext.font != nullptr)
		{
			createImageBindGroup(*mImageContext.font->getTexture());
		}

		ShaderPreProcessor processor;
		auto shader = std::make_unique<Shader>();
		if (!shader->load(kDefaultShader, processor))
		{
			LogError("Shader::create() failed for: %s!!", kDefaultShader);
			return false;
		}

		const SwapChain& swapChain = GraphicsDevice::get().getSwapChain();
		RenderPipelineProperties renderProps = {
			.shader = shader.get(),
			.bindGroupLayouts = {
				mRenderContext.bindGroupLayout,
				mImageContext.bindGroupLayout
			},
			.vertexBuffers = { mRenderContext.vertexBuffer },
			.colorTargets = {{
				.format = swapChain.getColorFormat(),
				.blendState = wgpu::BlendState {
					.color = {
						.operation = wgpu::BlendOperation::Add,
						.srcFactor = wgpu::BlendFactor::SrcAlpha,
						.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha
					},
					.alpha = {
						.operation = wgpu::BlendOperation::Add,
						.srcFactor = wgpu::BlendFactor::One,
						.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha
					}
				}
			}},
			.primitive = {
				.topology = wgpu::PrimitiveTopology::TriangleList,
				.frontFace = wgpu::FrontFace::CW,
				.cullMode = wgpu::CullMode::None
			}
		};

		wgpu::TextureFormat depthFormat = swapChain.getDepthFormat();
		if (depthFormat != wgpu::TextureFormat::Undefined)
		{
			renderProps.depthStencil = DepthStencilState{
				.format = depthFormat,
				.depthWriteEnabled = false,
				.depthCompare = wgpu::CompareFunction::Less
			};
		}

		auto pipeline = std::make_unique<RenderPipeline>();
		if (!pipeline->create(renderProps))
		{
			LogError("RenderPipeline::create() failed!!");
			return false;
		}

		mRenderContext.shader = shader.get();
		mRenderContext.pipeline = pipeline.get();
		mResourceCache->addResource(std::move(shader));
		mResourceCache->addResource(std::move(pipeline));

		return true;
	}

	void ImGuiRenderer::setupRenderStates(ImDrawData* drawData)
	{
		float l = drawData->DisplayPos.x;
		float r = drawData->DisplayPos.x + drawData->DisplaySize.x;
		float t = drawData->DisplayPos.y;
		float b = drawData->DisplayPos.y + drawData->DisplaySize.y;

		PerFrameData perFrameData;
		perFrameData.viewProj = glm::ortho(l, r, b, t);
		mRenderContext.perFrameBuffer->write(0, sizeof(PerFrameData), &perFrameData);

		mRenderPass->setViewport(0, 0, drawData->FramebufferScale.x * drawData->DisplaySize.x,
			drawData->FramebufferScale.y * drawData->DisplaySize.y, 0.0f, 1.0f);

		mRenderPass->setVertexBuffer(0, *mRenderContext.vertexBuffer);
		mRenderPass->setIndexBuffer(*mRenderContext.indexBuffer);
		mRenderPass->setPipeline(*mRenderContext.pipeline);
		mRenderPass->setBindGroup(kCommonBindGroupIndex, *mRenderContext.bindGroup);
		mRenderPass->setBlendConstant(0.0f, 0.0f, 0.0f, 0.0f);
	}

	bool ImGuiRenderer::createCommonBindGroup()
	{
		auto perFrameBuffer = std::make_unique<UniformBuffer>();
		if (!perFrameBuffer->create(sizeof(PerFrameData)))
		{
			LogError("UniformBuffer::create() failed!!");
			return false;
		}

		const std::vector<BindGroupLayoutItem> layoutItems =
		{
			{
				.binding = 0,
				.shaderStages = wgpu::ShaderStage::Vertex,
				.bindingLayout = BufferBindingLayout {
					.type = wgpu::BufferBindingType::Uniform,
					.minBindingSize = sizeof(PerFrameData)
				}
			}
		};

		const std::vector<BindGroupItem> bindGroupItems =
		{
			{
				.binding = 0,
				.size = sizeof(PerFrameData),
				.resource = BufferBindingResource(*perFrameBuffer)
			}
		};

		auto bindGroupLayout = std::make_unique<BindGroupLayout>();
		if (!bindGroupLayout->create(layoutItems))
		{
			LogError("BindGroupLayout::create() failed!!");
			return false;
		}

		auto bindGroup = std::make_unique<BindGroup>();
		if (!bindGroup->create(*bindGroupLayout, bindGroupItems))
		{
			LogError("BindGroup::create() failed!!");
			return false;
		}

		mRenderContext.perFrameBuffer = perFrameBuffer.get();
		mRenderContext.bindGroupLayout = bindGroupLayout.get();
		mRenderContext.bindGroup = bindGroup.get();
		mResourceCache->addResource(std::move(perFrameBuffer));
		mResourceCache->addResource(std::move(bindGroupLayout));
		mResourceCache->addResource(std::move(bindGroup));

		return true;
	}

	bool ImGuiRenderer::createImageBindGroup(const Texture& texture)
	{
		if (mImageContext.bindGroupLayout == nullptr)
		{
			const std::vector<BindGroupLayoutItem> layoutItems =
			{
				{
					.binding = 0,
					.shaderStages = wgpu::ShaderStage::Fragment,
					.bindingLayout = SamplerBindingLayout {
						.type = wgpu::SamplerBindingType::Filtering
					}
				},
				{
					.binding = 1,
					.shaderStages = wgpu::ShaderStage::Fragment,
					.bindingLayout = TextureBindingLayout {
						.sampleType = wgpu::TextureSampleType::Float,
						.viewDimension = wgpu::TextureViewDimension::e2D
					}
				}
			};

			auto bindGroupLayout = std::make_unique<BindGroupLayout>();
			if (!bindGroupLayout->create(layoutItems))
			{
				LogError("BindGroupLayout::create() failed!!");
				return false;
			}

			mImageContext.bindGroupLayout = bindGroupLayout.get();
			mResourceCache->addResource(std::move(bindGroupLayout));
		}

		if (mImageContext.sampler == nullptr)
		{
			auto sampler = std::make_unique<Sampler>();
			if (!sampler->load({
				.addressModeU = wgpu::AddressMode::Repeat,
				.addressModeV = wgpu::AddressMode::Repeat,
				.addressModeW = wgpu::AddressMode::Repeat,
				.magFilter = wgpu::FilterMode::Nearest,
				.minFilter = wgpu::FilterMode::Nearest,
				.mipmapFilter = wgpu::MipmapFilterMode::Nearest
			}))
			{
				LogError("Sampler::create() failed!!");
				return false;
			}

			mImageContext.sampler = sampler.get();
			mResourceCache->addResource(std::move(sampler));
		}

		const std::vector<BindGroupItem> bindGroupItems =
		{
			{
				.binding = 0,
				.resource = SamplerBindingResource(*mImageContext.sampler)
			},
			{
				.binding = 1,
				.resource = TextureBindingResource(texture)
			}
		};

		auto bindGroup = std::make_unique<BindGroup>();
		if (!bindGroup->create(*mImageContext.bindGroupLayout, bindGroupItems))
		{
			LogError("BindGroup::create() failed!!");
			return false;
		}

		ImGuiID texIdHash = ImHashData(&texture, sizeof(ImTextureID));
		mImageContext.bindGroups.insert(std::make_pair(texIdHash, bindGroup.get()));
		mResourceCache->addResource(std::move(bindGroup));

		return true;
	}

	bool ImGuiRenderer::createBufferData()
	{
		auto vertexLayout = std::make_unique<VertexLayout>();
		vertexLayout->setAttributes({
			{ wgpu::VertexFormat::Float32x2, (uint32_t)IM_OFFSETOF(ImDrawVert, pos), 0 },
			{ wgpu::VertexFormat::Float32x2, (uint32_t)IM_OFFSETOF(ImDrawVert, uv), 1 },
			{ wgpu::VertexFormat::Unorm8x4, (uint32_t)IM_OFFSETOF(ImDrawVert, col), 2 }
		});
		
		uint32_t numVertices = 128 * 1024;
		uint32_t numIndices = 256 * 1024;

		auto vertexBuffer = std::make_unique<VertexBuffer>();
		if (!vertexBuffer->create(*vertexLayout, numVertices))
		{
			LogError("VertexBuffer::create() failed!!");
			return false;
		}

		auto indexBuffer = std::make_unique<IndexBuffer>();
		if (!indexBuffer->create(wgpu::IndexFormat::Uint16, numIndices))
		{
			LogError("IndexBuffer::create() failed!!");
			return false;
		}

		mStagingContext.vertices.resize(numVertices);
		mStagingContext.indices.resize(numIndices);
		mStagingContext.numVertices = numVertices;
		mStagingContext.numIndices = numIndices;

		mRenderContext.vertexLayout = vertexLayout.get();
		mRenderContext.vertexBuffer = vertexBuffer.get();
		mRenderContext.indexBuffer = indexBuffer.get();

		mResourceCache->addResource(std::move(vertexLayout));
		mResourceCache->addResource(std::move(vertexBuffer));
		mResourceCache->addResource(std::move(indexBuffer));

		return true;
	}
}