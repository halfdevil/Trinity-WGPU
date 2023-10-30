#include "Graphics/Shader.h"
#include "Graphics/GraphicsDevice.h"
#include "VFS/FileSystem.h"
#include "Core/Debugger.h"
#include "Core/Logger.h"
#include "Core/ResourceCache.h"
#include "Utils/StringHelper.h"
#include <sstream>

namespace Trinity
{
	void ShaderPreProcessor::addDefine(const std::string& define, const std::string& value)
	{
		std::string newDefine;
		std::transform(define.begin(), define.end(), std::back_inserter(newDefine), ::toupper);

		mDefines.insert(std::pair(newDefine, value));
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

	std::string ShaderPreProcessor::processSource(const std::string& fileName, const std::string& source)
	{
		fs::path dir{ fileName };
		dir.remove_filename();

		std::istringstream input(source);
		std::string output;

		for (std::string line; std::getline(input, line);)
		{
			output.append(processLine(input, dir.string(), line));
		}

		return output;
	}

	std::string ShaderPreProcessor::processDefines(const std::string& line)
	{
		std::string newLine;
		char word[512];
		uint32_t l = 0;

		for (size_t i = 0; i < line.length(); i++)
		{
			if (::isalnum(line[i]) || line[i] == '_')
			{
				word[l++] = line[i];
			}
			else
			{
				if (l > 0)
				{
					word[l] = 0;

					if (auto it = mDefines.find(word); it != mDefines.end())
					{
						newLine.append(it->second);
					}
					else
					{
						newLine.append(word);
					}

					l = 0;
				}
					
				newLine.push_back(line[i]);
			}
		}

		if (l > 0)
		{
			word[l] = 0;
			newLine.append(word);
		}

		return newLine;
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

	std::string ShaderPreProcessor::processLine(std::istringstream& input, const std::string& dir, const std::string& line)
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
			auto name = line.substr(pos + 7);
			StringHelper::trim(name);

			if (!mDefines.contains(name))
			{
				mExcludeLine = true;
			}
		}
		else if (line.find("#else") != line.npos)
		{
			mExcludeLine = !mExcludeLine;
		}
		else if (line.find("#endif") != line.npos)
		{
			mExcludeLine = false;
		}
		else if (line.find("#define") != line.npos)
		{
			const auto pos = line.find("#define ");
			const auto p1 = line.find(" ", pos + 8);

			if (pos != line.npos && p1 != line.npos)
			{
				auto nameLen = p1 - pos - 8;
				auto name = line.substr(pos + 8, nameLen);
				auto value = line.substr(p1);

				StringHelper::trim(name);
				StringHelper::trim(value);

				addDefine(name, value);
			}
		}
		else
		{
			if (!mExcludeLine)
			{
				output.append(processDefines(line));
			}
		}

		return output;
	}

	bool Shader::create(const std::string& fileName, ResourceCache& cache, bool loadContent)
	{
		mFileName = fileName;

		if (loadContent)
		{
			ShaderPreProcessor processor;
			return load(fileName, processor);
		}

		return true;
	}

	void Shader::destroy()
	{
		mHandle = nullptr;
	}

	bool Shader::write()
	{
		return true;
	}

	bool Shader::load(const std::string& fileName, ShaderPreProcessor& processor)
	{
		std::string source = processor.process(fileName);
		if (source.empty())
		{
			LogError("Error loading shader source: %s", fileName.c_str());
			return false;
		}

		return loadFromSource(source);
	}

	bool Shader::loadFromSource(const std::string& source)
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

	std::type_index Shader::getType() const
	{
		return typeid(Shader);
	}
}