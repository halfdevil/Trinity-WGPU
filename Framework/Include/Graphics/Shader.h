#pragma once

#include "Core/Resource.h"
#include <webgpu/webgpu_cpp.h>
#include <unordered_map>
#include <string>

namespace Trinity
{
    class ShaderPreProcessor
    {
    public:

        ShaderPreProcessor() = default;

        void addDefine(const std::string& define, const std::string& value = "");
        void addDefines(const std::vector<std::string>& defines);

        std::string process(const std::string& fileName);

    private:

        std::string processDefines(const std::string& line);
        std::string processFile(const std::string& fileName);
        std::string processLine(std::istringstream& input, const std::string& dir,
            const std::string& line);

    private:

        std::unordered_map<std::string, bool> mIncludedFiles;
        std::unordered_map<std::string, std::string> mDefines;
        bool mExcludeLine{ false };
    };

    class Shader : public Resource
    {
    public:

        Shader() = default;
        ~Shader();

        Shader(const Shader&) = delete;
        Shader& operator = (const Shader&) = delete;

        Shader(Shader&&) = default;
        Shader& operator = (Shader&&) = default;

        const wgpu::ShaderModule& getHandle() const
        {
            return mHandle;
        }

        bool isValid() const
        {
            return mHandle != nullptr;
        }

        bool create(const std::string& fileName, ShaderPreProcessor& processor);
        bool createFromSource(const std::string& source);
        void destroy();

        virtual std::type_index getType() const override;

    private:

        wgpu::ShaderModule mHandle;
    };
}