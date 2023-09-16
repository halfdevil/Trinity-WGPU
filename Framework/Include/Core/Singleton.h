#pragma once

namespace Trinity
{
	template <typename T>
	class Singleton
	{
	public:

		Singleton()
		{
			mInstance = static_cast<T*>(this);
		}

		static bool hasInstance()
		{
			return mInstance != nullptr;
		}

		static T* getPtr()
		{
			return mInstance;
		}

		static T& get()
		{
			return *mInstance;
		}

	private:

		inline static T* mInstance{ nullptr };
	};
}