#pragma once

#include "Core/Window.h"
#include "Input/Keyboard.h"
#include "Input/Mouse.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

namespace Trinity
{
	struct InputAction
	{
		std::string name;
		std::vector<int32_t> keyboardKeys;
		std::vector<int32_t> mouseButtons;
	};

	struct InputAxisData
	{
		int32_t type;
		float scale;
	};

	struct InputAxis
	{
		std::string name;
		std::vector<InputAxisData> keyboardKeys;
		std::vector<InputAxisData> mouseButtons;
		std::vector<InputAxisData> mousePositions;
		std::vector<InputAxisData> mouseScrolls;
	};

	enum InputEvent : int32_t
	{
		Pressed = 0,
		Released,
		Triggered,
		Max
	};

	class Input
	{
	public:

		Input() = default;
		~Input();

		Input(const Input&) = delete;
		Input& operator = (const Input&) = delete;

		Input(Input&&) = delete;
		Input& operator = (Input&&) = delete;

		const Keyboard& getKeyboard() const
		{
			return mKeyboard;
		}

		const Mouse& getMouse() const
		{
			return mMouse;
		}

		bool create(Window& window);
		void destroy();
		void update();
		void postUpdate();

		void addAction(const InputAction& action);
		void addAxis(const InputAxis& axis);

		void bindAction(const std::string& actionName, InputEvent event,
			std::function<void(int32_t)> callback);

		void bindAxis(const std::string& axisName, std::function<void(float)> callback);

	private:

		void onKeyCallback(int32_t key, bool pressed);
		void onMousePosCallback(float x, float y);
		void onMouseButtonCallback(int32_t button, bool pressed, int32_t mods);
		void onMouseScrollCallback(float xoffset, float yoffset);

	private:

		Keyboard mKeyboard;
		Mouse mMouse;

		std::unordered_map<std::string, InputAction> mActionMap;
		std::unordered_map<std::string, InputAxis> mAxisMap;
		std::unordered_map<int32_t, std::string> mKeyboardKeyActionMap;
		std::unordered_map<int32_t, std::string> mMouseButtonActionMap;
		std::vector<std::unordered_map<std::string, Observer<int32_t>>> mActionBindings;
		std::unordered_map<std::string, Observer<float>> mAxisBindings;
	};
}