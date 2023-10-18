#pragma once

#include "Core/Resource.h"
#include "webgpu/webgpu_cpp.h"

namespace Trinity
{
    class FileReader;
    class FileWriter;

    struct SamplerProperties
    {
        wgpu::AddressMode addressModeU{ wgpu::AddressMode::ClampToEdge };
        wgpu::AddressMode addressModeV{ wgpu::AddressMode::ClampToEdge };
        wgpu::AddressMode addressModeW{ wgpu::AddressMode::ClampToEdge };
        wgpu::FilterMode magFilter{ wgpu::FilterMode::Nearest };
        wgpu::FilterMode minFilter{ wgpu::FilterMode::Nearest };
        wgpu::MipmapFilterMode mipmapFilter{ wgpu::MipmapFilterMode::Nearest };
        wgpu::CompareFunction compare{ wgpu::CompareFunction::Undefined };
    };

    class Sampler : public Resource
    {
    public:

        static constexpr const char* kDefault = "/Assets/Framework/Samplers/Default.tsamp";

        Sampler() = default;
        ~Sampler();

        Sampler(const Sampler&) = delete;
        Sampler& operator = (const Sampler&) = delete;

        Sampler(Sampler&&) = default;
        Sampler& operator = (Sampler&&) = default;

        const wgpu::Sampler& getHandle() const
        {
            return mHandle;
        }

        bool isValid() const
        {
            return mHandle != nullptr;
        }

        const SamplerProperties& getProperties() const
        {
            return mProperties;
        }

        virtual bool create(const std::string& fileName, ResourceCache& cache) override;
        virtual bool write() override;
		virtual void destroy();

		virtual bool load(const SamplerProperties& samplerProps);
        virtual void setProperties(SamplerProperties& samplerProps);

        virtual std::type_index getType() const override;

    protected:

		virtual bool read(FileReader& reader, ResourceCache& cache);
		virtual bool write(FileWriter& writer);

    protected:

        SamplerProperties mProperties;
        wgpu::Sampler mHandle;
    };
}