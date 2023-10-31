#pragma once

#include "Core/Resource.h"
#include <webgpu/webgpu_cpp.h>

namespace Trinity
{
	class ComputePass : public Resource
	{
	public:

        ComputePass() = default;
        virtual ~ComputePass() = default;

        ComputePass(const ComputePass&) = delete;
        ComputePass& operator = (const ComputePass&) = delete;

        ComputePass(ComputePass&&) = default;
        ComputePass& operator = (ComputePass&&) = default;

        virtual void destroy() override;
        virtual std::type_index getType() const override;

        virtual bool begin();
        virtual void end();
        virtual void submit();

	protected:

		wgpu::CommandEncoder mCommandEncoder{ nullptr };
		wgpu::ComputePassEncoder mComputePassEncoder{ nullptr };
	};
}