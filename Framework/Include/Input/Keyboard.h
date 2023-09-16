#pragma once

#include "Input/Types.h"
#include "Core/Observer.h"
#include <cstdint>

namespace Trinity
{
    class Keyboard
    {
    public:

        Keyboard() = default;

        Keyboard(const Keyboard&) = delete;
        Keyboard& operator = (const Keyboard&) = delete;

        Keyboard(Keyboard&&) = delete;
        Keyboard& operator = (Keyboard&&) = delete;

        bool isKeyDown(int32_t key) const;
        bool isKeyUp(int32_t key) const;
        bool isKeyHeld(int32_t key) const;
        bool isKeyiggered(int32_t key) const;

        void update(int32_t key, bool pressed);
        void postUpdate();

    public:

        Observer<int32_t, bool> onKeyStateUpdated;

    private:

        bool mCurrentState[KEY_LAST];
        bool mOldState[KEY_LAST];
    };
}