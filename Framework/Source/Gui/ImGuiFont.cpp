#include "Gui/ImGuiFont.h"
#include "Graphics/Texture2D.h"
#include "VFS/FileSystem.h"
#include "Core/Logger.h"

namespace Trinity
{
	ImGuiFont::~ImGuiFont()
	{
		destroy();
	}

	bool ImGuiFont::create(const std::string& name, const std::string& filePath, float size)
	{
		auto file = FileSystem::get().openFile(filePath, FileOpenMode::OpenRead);
		if (!file)
		{
			LogError("Error opening font file: %s", filePath.c_str());
			return false;
		}

		FileReader reader(*file);
		std::vector<uint8_t> buffer(reader.getSize());
		reader.read(buffer.data(), reader.getSize());

		ImGuiIO& io = ImGui::GetIO();
		ImFontConfig cfg;
		cfg.FontDataOwnedByAtlas = false;
		cfg.RasterizerMultiply = 1.5f;
		cfg.SizePixels = size;
		cfg.PixelSnapH = true;
		cfg.OversampleH = 4;
		cfg.OversampleV = 4;

		mHandle = io.Fonts->AddFontFromMemoryTTF(buffer.data(), (int)reader.getSize(),
			cfg.SizePixels, &cfg);

		if (!mHandle)
		{
			LogError("ImGui::FontAtlas::AddFontFromMemoryTTF() failed for: %s!!", filePath.c_str());
			return false;
		}

		uint8_t* pixels{ nullptr };
		int32_t width{ 0 }, height{ 0 };
		int32_t sizePerPixel{ 0 };
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &sizePerPixel);

		const wgpu::TextureUsage usage = wgpu::TextureUsage::TextureBinding |
			wgpu::TextureUsage::CopyDst;

		mTexture = std::make_unique<Texture2D>();
		if (!mTexture->create(width, height, wgpu::TextureFormat::RGBA8Unorm, usage))
		{
			LogError("Texture2D::create() failed for: %s!!", filePath.c_str());
			return false;
		}

		mTexture->upload(sizePerPixel, pixels, sizePerPixel * width * height);
		io.Fonts->SetTexID((ImTextureID)mTexture.get());

		mName = name;
		mSize = size;

		return true;
	}

	void ImGuiFont::destroy()
	{
		mTexture->destroy();
		mTexture = nullptr;
		mHandle = nullptr;
	}

	void ImGuiFont::activate()
	{
		ImGui::PushFont(mHandle);

		ImGuiIO& io = ImGui::GetIO();
		io.Fonts->SetTexID((ImTextureID)&mTexture);
	}

	void ImGuiFont::deactivate()
	{
		ImGui::PopFont();
	}

	std::type_index ImGuiFont::getType() const
	{
		return typeid(ImGuiFont);
	}
}