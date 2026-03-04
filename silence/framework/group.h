#pragma once

#include "element.h"
#include "animator.h"

// easy way to group da elemuntaz
class c_group : public c_element
{

private:

    friend class c_window;
    friend class c_collapsable_group;

    std::vector <c_element*> m_elements{};

public:

    c_group() = default;

    inline c_group(const std::string& name, const std::string& label, const bool default_visibility = true) noexcept
    {

        this->m_label = label;
        this->m_name = name;
        this->m_visible = default_visibility;
        this->m_type = framework_enums::element_type::GROUP;

    }

    template <typename type, typename... args>
    type* insert_element(args... initializers)
    {

        type* allocated_element = new type(initializers...);

        m_elements.push_back((c_element*)allocated_element);

        return allocated_element;

    }

    bool pop_element(const std::string_view& element_name)
    {

        for (const auto& element : m_elements)
            if (element->get_name() == element_name) [[likely]]
            {

                const auto position = std::find(m_elements.begin(), m_elements.end(), element);

                if (position != m_elements.end()) [[unlikely]]
                    m_elements.erase(position);

                return true;

            }

        return false;

    }

    virtual bool render()
    {

        int multiplier = 1;

        for (const auto& element : m_elements)
            if (element->m_type != framework_enums::element_type::KEYBIND)
                ++multiplier;

        // someone fix dis 4 me...
        if (const bool ret = ImGui::BeginChild(m_label.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, (30 * multiplier)), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize))
        {

            const auto text_size = ImGui::CalcTextSize(m_label.c_str());
            ImGui::SetCursorPos(ImVec2(ImGui::GetStyle().WindowPadding.x, (ImGui::GetCurrentWindow()->Size.y - text_size.y) / 2.f)); ImGui::TextGradiented(m_label.c_str(), IM_COL32(213, 125, 124, 255), IM_COL32(65, 23, 24, 255), 130.f); ImGui::Spacing();

            ImGui::BeginGroup();

            for (const auto& element : m_elements)
            {

                // properly handle rendering...

                element->run_render_callback();

                if (element->m_wants_same_line)
                    ImGui::SameLine();

                if (element->m_has_custom_position)
                    ImGui::SetCursorPos(element->m_custom_position);

                if (element->m_custom_font)
                    ImGui::PushFont(element->m_custom_font);

                if (element->render())
                    element->run_interaction_callback(); // run dat hoe

                if (element->m_custom_font)
                    ImGui::PopFont();

#ifdef _DEBUG
                if (GetAsyncKeyState(VK_DELETE))
                    ImGui::Text("^ m_name: %s", element->get_name().c_str());
#endif

            }

            ImGui::EndGroup();

            ImGui::EndChild();

            return ret;

        }

        return false;

    }

};

class c_collapsable_group : public c_group
{

private:

    bool m_is_collapsed{};

    c_animator<float> m_animator{};

public:

    inline c_collapsable_group(const std::string& name, const std::string& label, const bool collapsed = true, const bool default_visibility = true) noexcept
    {

        this->m_label = label;
        this->m_name = name;
        this->m_visible = default_visibility;
        this->m_type = framework_enums::element_type::GROUP;
        this->m_is_collapsed = collapsed;

    }

    virtual bool render()
    {

        if (!m_animator.get_setup_animator())
        {

            m_animator.set_start_value(30.f);
            m_animator.set_duration(0.8f);
            m_animator.set_starting_animation(!m_is_collapsed);

            m_animator.set_setup_animator(true);

        }

        float item_height = 30.f;

        for (const auto& element : m_elements)
            if (element->m_type == framework_enums::element_type::SLIDER_FLOAT || element->m_type == framework_enums::element_type::SLIDER_INT)
                item_height += (25.f + 3.f);
            else
                item_height += (12.f + 3.f);

        if (item_height != 30.f)
            item_height += 6.f;

        auto window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollWithMouse;

        m_animator.set_end_value(item_height);
        m_animator.update_animation();

        // someone fix dis 4 me...
        if (const bool ret = ImGui::BeginChild(m_label.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, m_animator.get_value()), true, window_flags))
        {

            const auto text_size = ImGui::CalcTextSize(m_label.c_str());
            ImGui::SetCursorPos(ImVec2(ImGui::GetStyle().WindowPadding.x, (30.f - text_size.y) / 2.f)); ImGui::TextGradiented(m_label.c_str(), IM_COL32(213, 125, 124, 255), IM_COL32(65, 23, 24, 255), 130.f); ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(200, 200, 200, m_is_collapsed ? 100 : 200));
            ImGui::SameLine(ImGui::GetContentRegionAvail().x - 12.f); ImGui::SetCursorPosY(3.f + 3.f); const bool open_clicked = ImGui::ArrowButton(" ^ ", m_is_collapsed ? ImGuiDir_Up : ImGuiDir_Down);
            ImGui::PopStyleColor();

            if (open_clicked && m_animator.start_animation(m_is_collapsed))
                m_is_collapsed = !m_is_collapsed;

            if (m_is_collapsed)
            {

                ImGui::EndChild();

                return ret;

            }

            ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(213 / 255.f, 125 / 255.f, 124 / 255.f, std::clamp(m_animator.get_animation_percent(), 0.f, 0.6f)));
            ImGui::Separator();
            ImGui::PopStyleColor();

            ImGui::Spacing();

            ImGui::BeginGroup();

            for (const auto& element : m_elements)
            {

                // properly handle rendering...

                element->run_render_callback();

                if (element->m_wants_same_line)
                    ImGui::SameLine();

                if (element->render())
                    element->run_interaction_callback();

#ifdef _DEBUG
                if (GetAsyncKeyState(VK_DELETE))
                    ImGui::Text("^ m_name: %s", element->get_name().c_str());
#endif

            }

            ImGui::EndGroup();

            ImGui::EndChild();

            return ret;

        }

        return false;

    }

};
