#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <map>
#include <unordered_map>
#include <array>
#include <string_view>
#include <unordered_map>

/**
 * @brief Custom ImGui drawing helpers and animated widgets.
 */
namespace ImNeo
{
	/**
	 * @brief Adds two ImGui vectors component-wise.
	 * @param lhs Left operand.
	 * @param rhs Right operand.
	 * @return Component-wise vector sum.
	 */
	inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs)
	{
		return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y);
	}

	/**
	 * @brief Subtracts two ImGui vectors component-wise.
	 * @param lhs Left operand.
	 * @param rhs Right operand.
	 * @return Component-wise vector difference.
	 */
	inline ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs)
	{
		return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y);
	}

	/**
	 * @brief Multiplies an ImGui vector by a scalar.
	 * @param lhs Vector operand.
	 * @param rhs Scalar operand.
	 * @return Scaled vector.
	 */
	inline ImVec2 operator*(const ImVec2& lhs, float rhs)
	{
		return ImVec2(lhs.x * rhs, lhs.y * rhs);
	}

	/**
	 * @brief Multiplies an ImGui vector by a scalar.
	 * @param lhs Scalar operand.
	 * @param rhs Vector operand.
	 * @return Scaled vector.
	 */
	inline ImVec2 operator*(float lhs, const ImVec2& rhs)
	{
		return ImVec2(lhs * rhs.x, lhs * rhs.y);
	}

	/**
	 * @brief Divides an ImGui vector by a scalar.
	 * @param lhs Vector operand.
	 * @param rhs Scalar divisor.
	 * @return Scaled vector.
	 */
	inline ImVec2 operator/(const ImVec2& lhs, float rhs)
	{
		return ImVec2(lhs.x / rhs, lhs.y / rhs);
	}

	/**
	 * @brief Simple three-dimensional vector used by custom widget drawing.
	 */
	struct ImVec3
	{
		float x; ///< X coordinate.
		float y; ///< Y coordinate.
		float z; ///< Z coordinate.

		/**
		 * @brief Creates a vector from explicit coordinates.
		 * @param _x X coordinate.
		 * @param _y Y coordinate.
		 * @param _z Z coordinate.
		 */
		ImVec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
	};

	/**
	 * @brief Rotates a vector around the X axis.
	 * @param v Vector to rotate.
	 * @param angle Rotation angle in degrees.
	 * @return Rotated vector.
	 */
	inline ImVec3 RotateX(const ImVec3& v, float angle)
	{
		float rad = angle * (3.14159f / 180.0f);
		float cosA = cos(rad);
		float sinA = sin(rad);

		return ImVec3(v.x, v.y * cosA - v.z * sinA, v.y * sinA + v.z * cosA);
	}

	/**
	 * @brief Rotates a vector around the Y axis.
	 * @param v Vector to rotate.
	 * @param angle Rotation angle in degrees.
	 * @return Rotated vector.
	 */
	inline ImVec3 RotateY(const ImVec3& v, float angle)
	{
		float rad = angle * (3.14159f / 180.0f);
		float cosA = cos(rad);
		float sinA = sin(rad);

		return ImVec3(v.x * cosA + v.z * sinA, v.y, -v.x * sinA + v.z * cosA);
	}

	/**
	 * @brief Draws a projected wireframe cube into the current ImGui window.
	 * @param center Cube center in screen coordinates.
	 * @param size Base cube size.
	 * @param perspective Perspective projection factor.
	 * @param rotationX Rotation around the X axis in degrees.
	 * @param rotationY Rotation around the Y axis in degrees.
	 * @param distance Distance multiplier used before projection.
	 */
	inline void Add3DCube(ImVec2 center, float size, float perspective, float rotationX, float rotationY, float distance)
	{
		std::array<ImVec3, 8> cube_vertices{ {
			ImVec3(-1, -1, -1), ImVec3(1, -1, -1),
			ImVec3(1,  1, -1), ImVec3(-1,  1, -1),
			ImVec3(-1, -1,  1), ImVec3(1, -1,  1),
			ImVec3(1,  1,  1), ImVec3(-1,  1,  1)}
	};

	std::array<ImVec2, 8> projected_vertices;

	for(size_t i = 0; i < cube_vertices.size(); i++)
	{
		ImVec3 rotated = RotateX(cube_vertices[i], rotationX);
		rotated = RotateY(rotated, rotationY);

		rotated.x *= distance;
		rotated.y *= distance;
		rotated.z *= distance;

		float z = rotated.z * perspective + 3.0f;
		projected_vertices[i] = ImVec2(
			center.x + rotated.x * size / z,
			center.y + rotated.y * size / z
		);

		//ImGui::GetWindowDrawList()->AddCircle(
		//    projected_vertices[i], 3.5f, ImColor(255, 255, 0, 255), 24, 2.0f
		//);
	}

	std::array<std::pair<int, int>, 12> edges = { {
		{0, 1}, {1, 2}, {2, 3}, {3, 0},
		{4, 5}, {5, 6}, {6, 7}, {7, 4},
		{0, 4}, {1, 5}, {2, 6}, {3, 7}
	} };

	for(const auto& edge : edges)
	{
		ImGui::GetWindowDrawList()->AddLine(
			projected_vertices[edge.first],
			projected_vertices[edge.second],
			IM_COL32(255, 255, 255, 255), 2.0f
		);
	}
}

/**
 * @brief Per-widget state for the animated loading cube.
 */
struct AnimationState
{
	bool state;             ///< Whether animation speed is currently increasing.
	float target_speed;     ///< Maximum target animation speed.
	float speed_multiplier; ///< Current animation speed multiplier.
	float rotation_x;       ///< Current X-axis rotation.
	float rotation_y;       ///< Current Y-axis rotation.
};

/**
 * @brief Draws an animated loading cube with a text label.
 * @param label Visible label and ImGui id source.
 * @param position Top-left position for the widget.
 */
inline void AddLoadingCube(const char* label, ImVec2 position)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	const ImGuiID id = window->GetID(label);
	if(window->SkipItems) return;

	ImGui::PushID(id);
	const auto& g = *GImGui;
	const auto delta_time = ImGui::GetIO().DeltaTime;

	const ImVec2 text_size = ImGui::CalcTextSize(label);
	const float spacing = 25.f;
	const float cube_size = 50.f;
	const ImVec2 extra_bb = ImVec2(cube_size + spacing, cube_size + spacing);

	const ImRect bb(position, position + extra_bb);
	ImGui::ItemSize(extra_bb);
	ImGui::ItemAdd(bb, id);

	static std::map<ImGuiID, AnimationState> animations;
	if(animations.find(id) == animations.end()) animations[id] = AnimationState{ true, 68.0f, 0.0f, 0.0f, 0.0f };

	AnimationState& anim_state = animations[id];
	const float lerp_factor = delta_time * 3.0f;

	if(anim_state.state)
    {
		anim_state.speed_multiplier = ImLerp(anim_state.speed_multiplier, anim_state.target_speed, lerp_factor);
		if(anim_state.speed_multiplier >= anim_state.target_speed * 0.95f) anim_state.state = false;
	}
	else
    {
		anim_state.speed_multiplier = ImLerp(anim_state.speed_multiplier, 2.0f, lerp_factor);
		if(anim_state.speed_multiplier <= 2.4f) anim_state.state = true;
	}

	anim_state.rotation_x += delta_time * (8.5f * anim_state.speed_multiplier);
	anim_state.rotation_y -= delta_time * (8.0f * anim_state.speed_multiplier);
	const float scale_factor = 1.0f + (anim_state.speed_multiplier * 0.0012f);

	Add3DCube
	(
		position + ImVec2(cube_size / 2.f + spacing / 2.f, cube_size / 2.f + spacing / 4.f),
		cube_size,
		0.6f,
		anim_state.rotation_x,
		anim_state.rotation_y,
		scale_factor
	);

	if(ImGui::GetIO().Fonts->Fonts.Size > 8 && ImGui::GetIO().Fonts->Fonts[8])
	{
		ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[8]);
	}
	else
	{
		ImGui::PushFont(ImGui::GetFont());
	}
	static std::map<ImGuiID, float> values;
	float& val = values[id];
	val = ImLerp(val, 200.f, g.IO.DeltaTime * 10.f);

	ImVec2 text_pos = ImVec2
	(
		bb.Min.x + (bb.GetWidth() - text_size.x) / 2.0f,
		bb.Min.y + bb.GetHeight() + 5.0f
	);

	ImGui::GetWindowDrawList()->AddText(text_pos, ImColor(255, 255, 255, int((anim_state.speed_multiplier * 2) + 135)), label);

	ImGui::PopFont();
	ImGui::PopID();
}



/**
 * @brief Per-widget state for the custom checkbox.
 */
struct CheckboxState
{
	float circle_offset = 0.0f;                                  ///< Animated toggle knob offset.
	ImVec4 background = ImVec4(0.2f, 0.9f, 0.2f, 1.0f);          ///< Current background color.
	ImVec4 text_colored = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);       ///< Current label color.
	ImVec4 circle = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);             ///< Current knob color.
};

/**
 * @brief Animation state storage keyed by ImGui widget id.
 */
inline std::unordered_map<ImGuiID, CheckboxState> checkbox_states;

/**
 * @brief Draws an animated checkbox toggle.
 * @param label Visible label and ImGui id source.
 * @param value Boolean value updated when the control is pressed.
 * @param height Control height in pixels.
 * @param toggle_width Toggle body width in pixels.
 * @param toggle_height Toggle body height in pixels.
 * @param radius Toggle knob radius in pixels.
 * @param spacing Spacing between label and toggle.
 * @return True when the checkbox was pressed in this frame.
 */
inline bool Checkbox(const char* label, bool* value, float height = 22.0f, float toggle_width = 30.0f, float toggle_height = 14.0f,float radius = 5.0f,float spacing = 8.0f)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if(!window || window->SkipItems) return false;

	ImGuiID id = window->GetID(label);
	ImVec2 label_size = ImGui::CalcTextSize(label);
	const float total_width = label_size.x + spacing + toggle_width;

	const ImVec2 pos = ImGui::GetCursorScreenPos();
	const ImVec2 toggle_pos = ImVec2(pos.x + label_size.x + spacing, pos.y + (height - toggle_height) * 0.5f);

	const ImVec2 toggle_size = ImVec2(toggle_width, toggle_height);

	const ImRect total_bb(pos, ImVec2(pos.x + total_width, pos.y + height));
	ImGui::ItemSize(total_bb);

	if(!ImGui::ItemAdd(total_bb, id)) return false;

	bool hovered, held;
	const bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);

	auto& state = checkbox_states[id];

	ImGuiIO& io = ImGui::GetIO();
	ImVec4 target_text = *value ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) :
			hovered ? ImVec4(0.9f, 0.9f, 0.9f, 1.0f) :
			ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
	ImVec4 target_bg = *value ? ImVec4(0.3f, 0.5f, 0.85f, 1.0f) : ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
	ImVec4 target_circle = *value ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(0.5f, 0.5f, 0.5f, 1.0f);

	state.text_colored = ImLerp(state.text_colored, target_text, io.DeltaTime * 8.0f);
	state.background = ImLerp(state.background, target_bg, io.DeltaTime * 8.0f);
	state.circle = ImLerp(state.circle, target_circle, io.DeltaTime * 8.0f);
	float left_offset = radius + 3.0f;
	float right_offset = toggle_width - radius - 3.0f;
	state.circle_offset = ImLerp(state.circle_offset, *value ? right_offset : left_offset, io.DeltaTime * 10.0f);

	if(pressed)
	{
		*value = !(*value);
		ImGui::MarkItemEdited(id);
	}

	// Draw toggle
	const ImVec2 toggle_min = toggle_pos;
	const ImVec2 toggle_max = ImVec2(toggle_min.x + toggle_width, toggle_min.y + toggle_height);
	const ImVec2 circle_center = ImVec2(toggle_min.x + state.circle_offset, toggle_min.y + toggle_height * 0.5f);

	const ImU32 bg_col = ImGui::ColorConvertFloat4ToU32(state.background);
	const ImU32 circle_col = ImGui::ColorConvertFloat4ToU32(state.circle);
	const ImU32 text_col = ImGui::ColorConvertFloat4ToU32(state.text_colored);

	window->DrawList->AddRectFilled(toggle_min, toggle_max, bg_col, toggle_height * 0.5f); // Rounded toggle
	window->DrawList->AddCircleFilled(circle_center, radius, circle_col, 12);             // Circle

	// Draw label
	window->DrawList->AddText(pos, text_col, label);

	// Fix SameLine
	ImGui::SetCursorScreenPos(ImVec2(pos.x + total_width, pos.y));

	return pressed;
}



/**
 * @brief Extra custom widgets built on top of ImGui primitives.
 */
namespace Widgets
{
	/**
	 * @brief Per-widget state for custom buttons.
	 */
	struct ButtonState
	{
		ImVec4 background = ImVec4(0.15f, 0.15f, 0.15f, 1.f); ///< Current animated background color.
	};

	/**
	 * @brief Animation state storage keyed by ImGui button id.
	 */
	inline std::unordered_map<ImGuiID, ButtonState> button_states;

	/**
	 * @brief Draws a custom animated button.
	 * @param label Visible text and ImGui id source.
	 * @param size Button size in pixels.
	 * @param highlight_on_hover Whether hover state should use the accent color.
	 * @return True when the button was pressed in this frame.
	 */
	inline bool Button(std::string_view label, const ImVec2& size, bool highlight_on_hover = false)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (!window || window->SkipItems)
		{
			return false;
		}

		const ImGuiID id = window->GetID(label.data());
		const ImVec2 pos = window->DC.CursorPos;
		const ImRect rect(pos, pos + size);

		ImGui::ItemSize(size);
		if (!ImGui::ItemAdd(rect, id))
		{
			return false;
		}

		bool hovered, held;
		bool pressed = ImGui::ButtonBehavior(rect, id, &hovered, &held);

		auto& state = button_states[id];
		ImVec4 target_color = ImVec4(0.2f, 0.2f, 0.2f, 1.f);

		if (ImGui::IsItemActive())
		{
			target_color = ImVec4(0.2f, 0.6f, 1.0f, 1.f);
		}
		else if (highlight_on_hover && hovered)
		{
			target_color =ImVec4(0.3f, 0.5f, 0.85f, 1.0f);
		}

		state.background = ImLerp(state.background, target_color, ImGui::GetIO().DeltaTime * 8.f);

		window->DrawList->AddRectFilled(rect.Min, rect.Max,
		ImGui::ColorConvertFloat4ToU32(state.background), 4.0f);

		window->DrawList->AddText(rect.Min + ImVec2(size.x * 0.5f - ImGui::CalcTextSize(label.data()).x * 0.5f,
													size.y * 0.5f - ImGui::GetFontSize() * 0.5f), IM_COL32_WHITE, label.data());

		return pressed;
	}
}

}
