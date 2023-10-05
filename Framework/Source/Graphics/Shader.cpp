#include "Graphics/Shader.h"
#include "Graphics/GraphicsDevice.h"
#include "Core/Debugger.h"
#include "Core/Logger.h"
#include "VFS/FileSystem.h"

namespace Trinity
{
	void ShaderPreProcessor::addDefine(const std::string& define)
	{
		std::string newDefine;
		std::transform(define.begin(), define.end(), newDefine.begin(), ::toupper);

		mDefines.push_back(newDefine);
	}

	void ShaderPreProcessor::addDefines(const std::vector<std::string>& defines)
	{
		for (const std::string& define : defines)
		{
			addDefine(define);
		}
	}

	std::string ShaderPreProcessor::process(const std::string& fileName)
	{
		return processFile(fileName);
	}

	std::string ShaderPreProcessor::processFile(const std::string& fileName)
	{
		auto file = FileSystem::get().openFile(fileName, FileOpenMode::OpenRead);
		if (!file)
		{
			LogError("Error opening shader file: %s", fileName.c_str());
			return {};
		}

		fs::path dir{ fileName };
		dir.remove_filename();

		FileReader reader{ *file };
		std::string source = reader.readAsString();

		std::istringstream input(source);
		std::string output;

		for (std::string line; std::getline(input, line);)
		{
			output.append(processLine(input, dir.string(), line));
		}

		return output;
	}

	std::string ShaderPreProcessor::processLine(std::istringstream& input, const std::string& dir,
		const std::string& line)
	{
		std::string output;
		if (line.find("#include ") != line.npos)
		{
			const auto pos = line.find("#include ");
			const auto p1 = line.find('<', pos);
			const auto p2 = line.find('>', pos);

			if (p1 == line.npos || p2 == line.npos || p2 <= p1)
			{
				LogError("Invalid include directive");
				return std::string();
			}

			const std::string name = line.substr(p1 + 1, p2 - p1 - 1);
			fs::path actualPath = dir;

			if (!name.starts_with("/"))
			{
				actualPath.append(name);
			}
			else
			{
				actualPath = fs::path(name);
			}

			const std::string filePath = actualPath.string();
			auto it = mIncludedFiles.find(filePath);

			if (it == mIncludedFiles.end())
			{
				const std::string include = processFile(filePath);
				output.append(include);
				output.append("\n");

				mIncludedFiles.insert(std::pair(filePath, true));
			}
		}
		else if (line.find("#ifdef ") != line.npos)
		{
			const auto pos = line.find("#ifdef ");
			const auto name = line.substr(pos + 1);

			if (std::find(std::begin(mDefines), std::end(mDefines), name) == mDefines.end())
			{
				mExcludeLine = true;
			}
		}
		else if (line.find("#endif") != line.npos)
		{
			mExcludeLine = false;
		}
		else
		{
			if (!mExcludeLine)
			{
				output.append(line);
			}
		}

		return output;
	}

	Shader::~Shader()
	{
		destroy();
	}

	bool Shader::create(const std::string& fileName, ShaderPreProcessor& processor)
	{
		std::string source = processor.process(fileName);
		if (source.empty())
		{
			LogError("Error loading shader source: %s", fileName.c_str());
			return false;
		}

		return createFromSource(source);
	}

	bool Shader::createFromSource(const std::string& source)
	{
		wgpu::ShaderModuleWGSLDescriptor wgslDesc{};
		wgslDesc.code = source.c_str();

		wgpu::ShaderModuleDescriptor moduleDescriptor{};
		moduleDescriptor.nextInChain = &wgslDesc;

		const wgpu::Device& device = GraphicsDevice::get();
		mHandle = device.CreateShaderModule(&moduleDescriptor);

		if (!mHandle)
		{
			LogError("wgpu::Device::CreateShaderModule() failed!!");
			return false;
		}

		return true;
	}

	void Shader::destroy()
	{
		mHandle = nullptr;
	}

	std::type_index Shader::getType() const
	{
		return typeid(Shader);
	}
}