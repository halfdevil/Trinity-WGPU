#pragma once

#include <functional>
#include <cstdint>
#include <vector>

namespace Trinity
{
    template <typename ...Args>
    class Observer
    {
    public:

        int32_t subscribe(std::function<void(Args...)> subscriber)
        {
            mSubscribers.push_back(std::move(subscriber));
            return (int32_t)mSubscribers.size() - 1;
        }

        void unsubscribe(int32_t index)
        {
            if (index >= 0 && index < mSubscribers.size())
            {
                mSubscribers[index] = nullptr;
            }
        }

        void notify(Args... args)
        {
            for (auto& subscriber : mSubscribers)
            {
                if (subscriber)
                {
                    subscriber(args...);
                }
            }
        }

    private:

        std::vector<std::function<void(Args...)>> mSubscribers;
    };
}