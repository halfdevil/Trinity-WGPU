#pragma once

namespace Trinity
{
	template <typename T>
	class Singleton
	{
	public:

		Singleton()
		{
			mInstance = (T*)this;
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

	public:

		template <typename U>
		static U* getPtrAs()
		{
			return (U*)mInstance;
		}

		template <typename U>
		static U& getAs()
		{
			return *((U*)mInstance);
		}

	private:

		inline static T* mInstance{ nullptr };
	};
}