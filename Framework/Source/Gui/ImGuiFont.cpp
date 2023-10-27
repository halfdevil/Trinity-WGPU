#include "Gui/ImGuiFont.h"
#include "Graphics/Texture2D.h"
#include "VFS/FileSystem.h"
#include "Core/Logger.h"
#include "Core/Image.h"

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

	void ImGuiFont::addCustomGlyph(ImWchar glyph, const std::string& imgFilePath)
	{
		mCustomGlyphs.insert(std::make_pair(glyph, imgFilePath));
	}

	bool ImGuiFont::addIconsFont(const std::string& filePath, float fontSize, const ImWchar* glyphRanges)
	{
		auto file = FileSystem::get().openFile(filePath, FileOpenMode::OpenRead);
		if (!file)
		{
			LogError("Error opening icons font file: %s", filePath.c_str());
			return false;
		}

		FileReader reader(*file);
		std::vector<uint8_t> buffer(reader.getSize());
		reader.read(buffer.data(), reader.getSize());

		ImGuiIO& io = ImGui::GetIO();
		ImFontConfig cfg;
		cfg.FontDataOwnedByAtlas = false;
		cfg.RasterizerMultiply = 1.5f;
		cfg.SizePixels = fontSize;
		cfg.PixelSnapH = true;
		cfg.OversampleH = 4;
		cfg.OversampleV = 4;

		ImFont* iconsFont = io.Fonts->AddFontFromMemoryTTF(buffer.data(), (int)reader.getSize(),
			cfg.SizePixels, &cfg, glyphRanges);

		if (!iconsFont)
		{
			LogError("ImGui::FontAtlas::AddFontFromMemoryTTF() failed for: %s!!", filePath.c_str());
			return false;
		}

		return true;
	}

	bool ImGuiFont::build()
	{
		ImGuiIO& io = ImGui::GetIO();
		std::unordered_map<ImWchar, int32_t> rectIds;
		std::unordered_map<ImWchar, std::unique_ptr<Image>> images;

		for (const auto& it : mCustomGlyphs)
		{
			auto image = std::make_unique<Image>();
			if (!image->load(it.second))
			{
				LogError("Image::load() failed!!");
				return false;
			}

			int32_t id = io.Fonts->AddCustomRectFontGlyph(mHandle, it.first, image->getWidth(),
				image->getHeight(), (float)image->getWidth() + 1);

			rectIds.insert(std::make_pair(it.first, id));
			images.insert(std::make_pair(it.first, std::move(image)));
		}

		if (!io.Fonts->Build())
		{
			LogError("ImFontAtlas::Build() failed!!");
			return false;
		}

		uint8_t* pixels{ nullptr };
		int32_t width{ 0 }, height{ 0 };
		int32_t sizePerPixel{ 0 };
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &sizePerPixel);

		for (const auto& it : rectIds)
		{
			if (const auto* rect = io.Fonts->GetCustomRectByIndex(it.second))
			{
				auto imgIt = images.find(it.first);
				auto& image = imgIt->second;

				for (int32_t y = 0; y < rect->Height; y++)
				{
					ImU32* ptr = (ImU32*)pixels + (rect->Y + y) * width + rect->X;
					for (int32_t x = 0; x < rect->Width; x++)
					{
						*ptr++ = (ImU32)image->getPixelAsRGBA(x, y);
					}
				}
			}
		}

		const wgpu::TextureUsage usage = wgpu::TextureUsage::TextureBinding |
			wgpu::TextureUsage::CopyDst;

		mTexture = std::make_unique<Texture2D>();
		if (!mTexture->create(width, height, wgpu::TextureFormat::RGBA8Unorm, usage))
		{
			LogError("Texture2D::create() failed!!");
			return false;
		}

		mTexture->upload(sizePerPixel, pixels, sizePerPixel * width * height);
		io.Fonts->SetTexID((ImTextureID)mTexture.get());

		return true;
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