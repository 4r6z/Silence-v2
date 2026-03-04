#pragma once

#include "element.h"
#include "animator.h"

// dis one goes out to my homie t0ggle

class c_toggle : public c_element
{

private:

    bool* m_value{};

    c_animator<float> m_animator{};

public:

    inline c_toggle(const std::string_view& name, const std::string_view& label, bool* value, const bool default_visibility = true) noexcept
    {

        this->m_value = value;
        this->m_name = name;
        this->m_label = label;
        this->m_visible = default_visibility;
        this->m_type = framework_enums::element_type::SWITCH;

    }

    bool get_value() const noexcept { return *m_value; }

    virtual bool render()
    {

        ImGui::Text(m_label.c_str()); ImGui::SameLine(ImGui::GetContentRegionAvail().x - (12.f * 2.f));

        ImVec2 p = ImGui::GetCursorScreenPos();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        const float height = 12.f;
        const float width = height * 2.f;
        const float radius = height * 0.5f;

        const bool value_modified = ImGui::InvisibleButton(m_name.c_str(), ImVec2(width, height), ImGuiButtonFlags_AlignTextBaseLine);

        const float end_x = p.x + width - radius;
        const float base_x = p.x + radius;
        constexpr float duration = 0.24f;

        if (!m_animator.get_setup_animator())
        {

            m_animator.set_duration(duration);
            m_animator.set_starting_animation(*m_value);
            m_animator.set_setup_animator(true);

        }

        m_animator.set_end_value(end_x);
        m_animator.set_start_value(base_x);

        if (value_modified && !m_animator.is_animating())
        {

            *m_value = !*m_value;
            m_animator.start_animation(*m_value);

        }

        m_animator.update_animation();

        ImU32 col_bg;

        const bool hovered = ImGui::IsItemHovered();

        if (hovered)
            col_bg = *m_value ? IM_COL32(213, 125, 124, 230 * std::clamp((m_animator.get_animation_percent()), 0.f, 1.f)) : ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_FrameBgHovered));
        else
            col_bg = *m_value ? IM_COL32(213, 125, 124, (ImGui::GetStyle().Alpha * 200.f) * std::clamp((m_animator.get_animation_percent()), 0.f, 1.f)) : ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_FrameBg));

        if (m_animator.get_animation_percent() > 0.f && !*m_value)
            col_bg = IM_COL32(213, 125, 124, (ImGui::GetStyle().Alpha * 200.f) * std::clamp((m_animator.get_animation_percent()), 0.f, 1.f));

        draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, ImGui::GetStyle().FrameRounding);
        if (!*m_value) draw_list->AddRect(p, ImVec2(p.x + width, p.y + height), ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_Border)), ImGui::GetStyle().FrameRounding);
        draw_list->AddCircleFilled(ImVec2(m_animator.get_value(), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, ImGui::GetStyle().Alpha * 200.f));

        return value_modified;

    }

};
