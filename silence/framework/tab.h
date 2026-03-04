#pragma once

#include "element.h"

#include "animator.h"

class c_click_label : public c_element
{

private:

    c_animator<float> m_animator{0.f, 6.f, 0.21f};
    int* m_current_tab{};
    int m_this_tab{};

public:

    inline c_click_label(const std::string_view& name, const std::string_view& label, int this_tab, int* current_tab, bool default_visibility = true)
    {

        this->m_label = label;
        this->m_name = name;
        this->m_visible = default_visibility;
        this->m_type = framework_enums::element_type::CLICK_LABEL;
        this->m_current_tab = current_tab;
        this->m_this_tab = this_tab;

    }

    virtual bool render()
    {

        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        char Buf[64];
        _snprintf(Buf, 62, "%s", m_label.c_str());

        char getid[128];
        _snprintf(getid, 128, "%s%s", m_label.c_str(), std::to_string(m_this_tab).c_str());

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(getid);
        const ImVec2 label_size = ImGui::CalcTextSize(m_label.c_str(), NULL, true);

        const ImRect check_bb(window->DC.CursorPos, ImVec2(label_size.y + style.FramePadding.y * 2 + window->DC.CursorPos.x, window->DC.CursorPos.y + label_size.y + style.FramePadding.y * 2));
        ImGui::ItemSize(check_bb, style.FramePadding.y);

        ImRect total_bb = check_bb;

        if (label_size.x > 0)
        {
            ImGui::SameLine(0, style.ItemInnerSpacing.x);
            const ImRect text_bb(ImVec2(window->DC.CursorPos.x, window->DC.CursorPos.y + style.FramePadding.y), ImVec2(window->DC.CursorPos.x + label_size.x, window->DC.CursorPos.y + style.FramePadding.y + label_size.y));

            ImGui::ItemSize(ImVec2(text_bb.GetWidth(), check_bb.GetHeight()), style.FramePadding.y);
            total_bb = ImRect(ImMin(check_bb.Min, text_bb.Min), ImMax(check_bb.Max, text_bb.Max));
        }

        if (!ImGui::ItemAdd(total_bb, id))
            return false;

        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
        auto is_current_tab = *m_current_tab == m_this_tab;
        auto label_position = check_bb.GetTL(); label_position.x += m_animator.get_value();
        auto bar_position = check_bb.GetTL(); bar_position.y += (label_size.y + 1.f); bar_position.x += m_animator.get_value();

        m_animator.update_animation();

        if (m_animator.get_animation_percent() != 1.f && hovered) m_animator.start_animation(true);
        else if (!hovered && m_animator.get_animation_percent() == 1.f) m_animator.start_animation(false);

        if (pressed)
            *m_current_tab = m_this_tab;

        if (is_current_tab)
            ImGui::TextGradiented(m_label.c_str(), label_position, IM_COL32(213, 125, 124, 255), IM_COL32(65, 23, 24, 255), 100.f * m_animator.get_animation_percent());
        else
            ImGui::RenderText(label_position, m_label.c_str());

        auto imgui_alpha = ImGui::GetStyle().Alpha;
        auto s = [imgui_alpha](int x) { return imgui_alpha < 0.8f ? imgui_alpha : x; };

        ImGui::GetCurrentWindow()->DrawList->AddRectFilledMultiColor(bar_position, bar_position + ImVec2(m_animator.get_value() * (label_size.x * 0.15f), 1.f), IM_COL32(213, 125, 124, s(255)), IM_COL32(213, 125, 124, s(10)), IM_COL32(213, 125, 124, s(10)), IM_COL32(213, 125, 124, s(255)));
        ImGui::GetCurrentWindow()->DrawList->AddRectFilledMultiColor(bar_position + ImVec2(0.f, 1.f), bar_position + ImVec2(m_animator.get_value() * (label_size.x * 0.15f), 2.f), IM_COL32(213, 125, 124, s(200)), IM_COL32(213, 125, 124, s(10)), IM_COL32(213, 125, 124, s(10)), IM_COL32(213, 125, 124, s(200)));

        ImGui::GetCurrentWindow()->DrawList->AddRectFilled(bar_position + ImVec2(0.f, 2.f), bar_position + ImVec2(m_animator.get_value() * (label_size.x * 0.15f), 3.f), IM_COL32(65, 23, 24, s(255)));

        return pressed;

    }

};

// very similar to a button, except it toggles l0l
class c_tab : public c_element
{

private:

    ImVec2 m_size{};
    c_animator<float> m_animator{ 0.2f, 0.8f, 0.83f };
    bool m_show_name{};
    int* m_current_tab{};
    float* m_dpi_scale{};
    int m_this_tab{};

public:

    inline c_tab(const std::string_view& name, const std::string_view& label, const ImVec2& size, const int this_tab, int* current_tab, float* dpi_scale = nullptr, const bool show_name = false, const bool default_visibility = true) noexcept
    {

        this->m_label = label;
        this->m_name = name;
        this->m_visible = default_visibility;
        this->m_type = framework_enums::element_type::TAB;
        this->m_current_tab = current_tab;
        this->m_this_tab = this_tab;
        this->m_size = size;
        this->m_show_name = show_name;
        this->m_dpi_scale = dpi_scale;

    }

    virtual bool render()
    {

        static auto custom_button = [](const char* label, ImVec2 esize, bool tabbed, const char* real_name, c_animator<float>* anim = NULL, bool show_name = false) -> bool
        {
            ImGuiWindow* window = ImGui::GetCurrentWindow();
            int flags = 0;
            ImVec2 size_arg = esize;
            if (window->SkipItems)
                return false;

            ImGuiContext& g = *GImGui;
            const ImGuiStyle& style = g.Style;
            const ImGuiID id = window->GetID(label);
            const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

            ImVec2 pos = window->DC.CursorPos;
            if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
                pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
            ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

            const ImRect bb(pos, pos + size);
            ImGui::ItemSize(size, style.FramePadding.y);
            if (!ImGui::ItemAdd(bb, id))
                return false;

            if (g.LastItemData.InFlags & ImGuiItemFlags_ButtonRepeat)
                flags |= ImGuiButtonFlags_Repeat;

            bool hovered, held;
            bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);

            // Render
            const ImU32 col = ImGui::GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
            ImGui::RenderNavHighlight(bb, id);
            ImGui::RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);

            if (hovered && !tabbed)
                anim->start_animation(false, true);

            anim->update_animation();

            if (tabbed)
                ImGui::TextGradiented(label, bb.GetCenter() - label_size * 0.5f, IM_COL32(213, 125, 124, 185), IM_COL32(65, 23, 24, 255), 130.f);
            else {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, anim->get_value()));
                const auto tmp_sz = ImGui::CalcTextSize(real_name);
                if (!hovered || !show_name) ImGui::RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);
                else ImGui::RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, real_name, NULL, &tmp_sz, style.ButtonTextAlign, &bb);
                ImGui::PopStyleColor();
            }

            IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
            return pressed;
        };

        if (*m_current_tab == m_this_tab)
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));

        const auto ret = custom_button(m_label.c_str(), m_dpi_scale ? (m_size * *m_dpi_scale) : m_size, (*m_current_tab == m_this_tab), m_name.c_str(), &m_animator, m_show_name);

        if (*m_current_tab == m_this_tab)
            ImGui::PopStyleColor();

        if (ret) *m_current_tab = m_this_tab;

        return ret;

    }

};
