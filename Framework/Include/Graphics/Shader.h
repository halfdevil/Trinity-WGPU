#pragma once

#include "Graphics/Resource.h"
#include <webgpu/webgpu_cpp.h>
#include <unordered_map>
#include <string>
#include <sstream>

namespace Trinity
{
    class ShaderPreProcessor
    {
    public:

        ShaderPreProcessor() = default;

        void addDefine(const std::string& define);
        void addDefines(const std::vector<std::string>& defines);

        std::string process(const std::string& fileName);

    private:

        std::string processFile(const std::string& fileName);
        std::string processLine(std::istringstream& input, const std::string& dir,
            const std::string& line);

    private:

        std::unordered_map<std::string, bool> mIncludedFiles;
        std::vector<std::string> mDefines;
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