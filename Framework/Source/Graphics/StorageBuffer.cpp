#include "Graphics/StorageBuffer.h"
#include "Graphics/GraphicsDevice.h"
#include "Core/Logger.h"
#include "Core/Debugger.h"

namespace Trinity
{
	StorageBuffer::~StorageBuffer()
	{
		destroy();
	}

	bool StorageBuffer::create(uint32_t size, const void* data)
	{
		const wgpu::Device& device = GraphicsDevice::get();
		mSize = size;

		wgpu::BufferDescriptor bufferDescriptor{};
		bufferDescriptor.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst;
		bufferDescriptor.size = mSize;
		bufferDescriptor.mappedAtCreation = false;

		mHandle = device.CreateBuffer(&bufferDescriptor);
		if (!mHandle)
		{
			LogError("wgpu::Device::CreateBuffer() failed!!");
			return false;
		}

		if (data)
		{
			write(0, mSize, data);
		}

		return true;
	}

	void StorageBuffer::destroy()
	{
		if (mHandle)
		{
			mHandle.Destroy();
			mHandle = nullptr;
		}
	}
}