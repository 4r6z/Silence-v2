#pragma once

#include "element.h"
#include "animator.h"

class c_progress_bar : public c_element
{

private:

    c_animator<float> m_animator{ 0.f, 0.f, 0.2f };
    ImVec2 m_size{};
    float m_value{};
    float m_max_value{};

public:

    inline c_progress_bar(const std::string_view& name, const std::string_view& label, ImVec2 size, float max_value = 1.f, float value = 0.f, const bool default_visibility = true) noexcept
    {

        this->m_name = name;
        this->m_label = label;
        this->m_visible = default_visibility;
        this->m_type = framework_enums::element_type::PROGRESS_BAR;
        this->m_size = size;
        this->m_max_value = max_value;
        this->m_value = value;

    }

    virtual bool render()
    {

        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(m_label.c_str());

        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size = m_size;

        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ImGui::ItemSize(bb, style.FramePadding.y);
        if (!ImGui::ItemAdd(bb, id))
            return false;
        
        m_animator.update_animation();

        static auto scale_number = [](float number, float minInput, float maxInput, float minOutput, float maxOutput) -> auto {

            double inputRange = maxInput - minInput;
            double outputRange = maxOutput - minOutput;
            double scaledNumber = (number - minInput) / inputRange * outputRange + minOutput;

            return round(scaledNumber);
        };

        const float current_value = std::clamp(m_animator.get_value(), 0.f, m_max_value) / m_max_value;

        const float circleStart = size.x;
        const float circleEnd = size.x;
        const float circleWidth = circleEnd - circleStart;
        const float rounding = ImGui::GetStyle().FrameRounding;

        window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart, bb.Max.y), ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_FrameBg)), rounding);

        if (current_value) window->DrawList->AddRectFilled(bb.Min + ImVec2(0, 1), ImVec2(pos.x + circleStart * current_value, bb.Max.y - 1.f), IM_COL32(137, 254, 236, static_cast<int>(ImGui::GetStyle().Alpha * 255.f)), rounding);

        window->DrawList->AddRect(bb.Min, ImVec2(pos.x + circleStart, bb.Max.y), ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_Border)), rounding);

    }

    void set_max_value(float value) { m_max_value = value; }
    void set_value(float value) { m_value = value; }
    void increment_value(float how_much) 
    {
    
        m_animator.set_start_value(m_value);
        m_animator.set_end_value(m_value + how_much);
    
        m_animator.start_animation(true, true);

        m_value += how_much;

    }

    void reset() 
    {

        set_max_value(1.f);
        set_value(1.f);
        increment_value(-1.f);

    }

    bool is_animating() const { return m_animator.is_animating(); }

};