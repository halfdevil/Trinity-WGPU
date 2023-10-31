#include "Graphics/ComputePass.h"
#include "Graphics/GraphicsDevice.h"
#include "Core/Debugger.h"
#include "Core/Logger.h"

namespace Trinity
{
	void ComputePass::destroy()
	{
		Resource::destroy();
		mCommandEncoder = nullptr;
		mComputePassEncoder = nullptr;
	}

	std::type_index ComputePass::getType() const
	{
		return typeid(ComputePass);
	}

	bool ComputePass::begin()
	{
		wgpu::ComputePassDescriptor computePassDesc;

		auto& graphicsDevice = GraphicsDevice::get();
		mCommandEncoder = graphicsDevice.getDevice().CreateCommandEncoder();

		if (!mCommandEncoder)
		{
			LogError("Device::CreateCommandEncoder() failed");
			return false;
		}

		mComputePassEncoder = mCommandEncoder.BeginComputePass(&computePassDesc);
		if (!mComputePassEncoder)
		{
			LogError("wgpu::CommandEncoder::BeginComputePass() failed!!");
			return false;
		}

		return true;
	}

	void ComputePass::end()
	{
		Assert(mComputePassEncoder != nullptr, "ComputePass::begin() not called!!");
		mComputePassEncoder.End();
	}

	void ComputePass::submit()
	{
		Assert(mComputePassEncoder != nullptr, "RenderPass::begin() not called!!");

		auto& graphicsDevice = GraphicsDevice::get();
		wgpu::CommandBuffer commands = mCommandEncoder.Finish();
		graphicsDevice.getQueue().Submit(1, &commands);
	}
}