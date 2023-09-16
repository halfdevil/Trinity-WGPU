#include "Input/Keyboard.h"

namespace Trinity
{
    bool Keyboard::isKeyDown(int32_t key) const
    {
        if (key >= 0 && key < KEY_LAST)
        {
            return mCurrentState[key];
        }

        return false;
    }

    bool Keyboard::isKeyUp(int32_t key) const
    {
        if (key >= 0 && key < KEY_LAST)
        {
            return !mCurrentState[key];
        }

        return false;
    }

    bool Keyboard::isKeyHeld(int32_t key) const
    {
        if (key >= 0 && key < KEY_LAST)
        {
            return mCurrentState[key] && mOldState[key];
        }

        return false;
    }

    bool Keyboard::isKeyiggered(int32_t key) const
    {
        if (key >= 0 && key < KEY_LAST)
        {
            return mCurrentState[key] && !mOldState[key];
        }

        return false;
    }

    void Keyboard::update(int32_t key, bool pressed)
    {
        mCurrentState[key] = pressed;
        onKeyStateUpdated.notify(key, pressed);
    }

    void Keyboard::postUpdate()
    {
        memcpy(mOldState, mCurrentState, sizeof(bool) * KEY_LAST);
    }
}