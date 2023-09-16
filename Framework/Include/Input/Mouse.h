#pragma once

#include "Input/Types.h"
#include "Core/Observer.h"
#include <cstdint>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

namespace Trinity
{
    class Mouse
    {
    public:

        Mouse() = default;

        Mouse(const Mouse&) = delete;
        Mouse& operator = (const Mouse&) = delete;

        Mouse(Mouse&&) = delete;
        Mouse& operator = (Mouse&&) = delete;

        const glm::vec2& getPosition() const
        {
            return mCurrentPosition;
        }

        glm::ivec2 getRelativePosition() const
        {
            return mCurrentPosition - mOldPosition;
        }

        const glm::vec2& getScroll() const
        {
            return mScroll;
        }

        bool isButtonDown(int32_t button) const;
        bool isButtonUp(int32_t button) const;
        bool isButtonHeld(int32_t button) const;
        bool isButtoniggered(int32_t button) const;

        void update(int32_t button, bool pressed);
        void updatePosition(float x, float y);
        void updateScroll(float x, float y);
        void postUpdate();

    public:

        Observer<int32_t, bool> onButtonStateUpdated;
        Observer<int32_t, int32_t> onPositionUpdated;
        Observer<int32_t, int32_t> onScrollUpdated;

    private:

        bool mCurrentState[MOUSE_BUTTON_LAST];
        bool mOldState[MOUSE_BUTTON_LAST];
        glm::vec2 mCurrentPosition;
        glm::vec2 mOldPosition;
        glm::vec2 mScroll;
    };
}