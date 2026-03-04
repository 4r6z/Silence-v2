#pragma once
#ifdef ENABLE_BINDS

#include "includes.h"
#include "element.h"
#include "imgui/imgui_internal.h"

#include <Windows.h>

enum keybind_display_type : uint16_t
{

    WITH_LABEL,
    WITHOUT_LABEL

};

enum keybind_interaction_type : uint16_t
{

    TOGGLE,
    HELD,
    ALWAYS,
    NEVER

};

const char* const key_names[] = {
    "Unknown",
    "LBUTTON",
    "RBUTTON",
    "CANCEL",
    "MBUTTON",
    "XBUTTON1",
    "XBUTTON2",
    "Unknown",
    "BACK",
    "TAB",
    "Unknown",
    "Unknown",
    "CLEAR",
    "RETURN",
    "Unknown",
    "Unknown",
    "SHIFT",
    "CONTROL",
    "MENU",
    "PAUSE",
    "CAPITAL",
    "KANA",
    "Unknown",
    "JUNJA",
    "FINAL",
    "KANJI",
    "Unknown",
    "ESCAPE",
    "CONVERT",
    "NONCONVERT",
    "ACCEPT",
    "MODECHANGE",
    "SPACE",
    "PRIOR",
    "NEXT",
    "END",
    "HOME",
    "LEFT",
    "UP",
    "RIGHT",
    "DOWN",
    "SELECT",
    "PRINT",
    "EXECUTE",
    "SNAPSHOT",
    "INSERT",
    "DELETE",
    "HELP",
    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "A",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    "I",
    "J",
    "K",
    "L",
    "M",
    "N",
    "O",
    "P",
    "Q",
    "R",
    "S",
    "T",
    "U",
    "V",
    "W",
    "X",
    "Y",
    "Z",
    "LWIN",
    "RWIN",
    "APPS",
    "Unknown",
    "SLEEP",
    "NUMPAD0",
    "NUMPAD1",
    "NUMPAD2",
    "NUMPAD3",
    "NUMPAD4",
    "NUMPAD5",
    "NUMPAD6",
    "NUMPAD7",
    "NUMPAD8",
    "NUMPAD9",
    "MULTIPLY",
    "ADD",
    "SEPARATOR",
    "SUBTRACT",
    "DECIMAL",
    "DIVIDE",
    "F1",
    "F2",
    "F3",
    "F4",
    "F5",
    "F6",
    "F7",
    "F8",
    "F9",
    "F10",
    "F11",
    "F12",
    "F13",
    "F14",
    "F15",
    "F16",
    "F17",
    "F18",
    "F19",
    "F20",
    "F21",
    "F22",
    "F23",
    "F24",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "NUMLOCK",
    "SCROLL",
    "OEM_NEC_EQUAL",
    "OEM_FJ_MASSHOU",
    "OEM_FJ_TOUROKU",
    "OEM_FJ_LOYA",
    "OEM_FJ_ROYA",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "LSHIFT",
    "RSHIFT",
    "LCONTROL",
    "RCONTROL",
    "LMENU",
    "RMENU",
    "NONE"
};

class c_binding
{

private:

    friend class c_keybind;

    std::string m_name{};
    keybind_interaction_type m_type{ keybind_interaction_type::NEVER };
    int m_current_key{ NULL };
    bool m_enabled{ false };
    bool m_waiting_for_input{ false };

    bool bind_new_key()
    {

        if (ImGui::IsKeyPressedMap(ImGuiKey_Escape))
        {

            m_current_key = NULL;
            ImGui::ClearActiveID();
            return true;

        }

        for (auto i = 1; i < 5; i++)
        {

            if (ImGui::GetIO().MouseDown[i])
            {

                switch (i)
                {
                case 1:
                    m_current_key = VK_RBUTTON;
                    break;
                case 2:
                    m_current_key = VK_MBUTTON;
                    break;
                case 3:
                    m_current_key = VK_XBUTTON1;
                    break;
                case 4:
                    m_current_key = VK_XBUTTON2;
                    break;
                }
                return true;

            }

        }

        for (auto i = VK_BACK; i <= VK_RMENU; i++)
        {

            if (ImGui::GetIO().KeysDown[i])
            {

                m_current_key = i;
                return true;

            }

        }

        return false;

    }

public:

    c_binding(const std::string_view& name, const int default_key = VK_LBUTTON)
    {

        this->m_name = name;
        this->m_current_key = default_key;
        this->m_type = keybind_interaction_type::TOGGLE;

    }

    std::string get_key_name() const
    {

        if (!m_current_key)
            return "none";

        std::string tmp = key_names[m_current_key];

        std::transform(tmp.begin(), tmp.end(), tmp.begin(),
            [](unsigned char c) { return std::tolower(c); });

        tmp[0] = std::tolower(tmp[0]);

        return tmp;

    }

    std::string get_bind_name() const { return m_name; }

    keybind_interaction_type get_bind_type() const { return m_type; }

    int get_bound_key() const { return m_current_key; }

    bool get_enabled() const { return m_enabled; }

    void update_state()
    {

        switch (m_type)
        {

        case keybind_interaction_type::ALWAYS:
            m_enabled = true;
            break;
        case keybind_interaction_type::HELD:
            m_enabled = GetAsyncKeyState(m_current_key);
            break;
        case keybind_interaction_type::TOGGLE:
            if (GetAsyncKeyState(m_current_key) & 1)
                m_enabled = !m_enabled;
            break;
        case keybind_interaction_type::NEVER:
            m_enabled = false;
            break;

        }

    }

    void set_key(int key) { m_current_key = key; }

    void set_mode(keybind_interaction_type type) { m_type = type; }

};

class c_keybind : public c_element
{

private:

    c_binding* m_bind{};
    keybind_display_type m_display_type{};

public:

    c_keybind(const std::string_view& name, const std::string_view& label, c_binding* bind, keybind_display_type display_type = keybind_display_type::WITH_LABEL, const bool default_visibility = true)
    {

        this->m_name = name;
        this->m_label = label;
        this->m_bind = bind;
        this->m_display_type = display_type;
        this->m_visible = default_visibility;
        this->m_type = framework_enums::element_type::KEYBIND;

    }

    virtual bool render()
    {

        static const auto h_sz = ImGui::CalcTextSize("H").y;
        static const auto t_sz = ImGui::CalcTextSize("T").y;
        static const auto a_sz = ImGui::CalcTextSize("A").y;

        static const auto window_size = ImVec2{ 71, (h_sz + t_sz + a_sz + 5) };

        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        if (m_display_type == keybind_display_type::WITH_LABEL)
        {

            ImGui::Text(m_label.c_str());
            ImGui::SameLine(ImGui::GetContentRegionAvail().x - 60.f);

        }
        else
        {

            ImGui::SameLine(ImGui::GetContentRegionAvail().x - 60.f);

        }

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(m_bind->get_bind_name().c_str());
        const ImVec2 label_size = ImGui::CalcTextSize(m_bind->get_bind_name().c_str());

        ImVec2 pos = window->DC.CursorPos;
        ImVec2 size = ImGui::CalcItemSize(ImVec2(0, 0), 60.f, 13.f);

        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ImGui::ItemSize(size, style.FramePadding.y);
        if (!ImGui::ItemAdd(bb, id))
            return false;

        bool hovered, held;
        bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, NULL);

        bool value_changed = false;
        int key = m_bind->get_bound_key();

        auto io = ImGui::GetIO();

        std::string name = m_bind->get_key_name();

        if (m_bind->m_waiting_for_input)
            name = "...";

        if (ImGui::GetIO().MouseClicked[0] && hovered)
        {

            if (g.ActiveId == id)
            {

                m_bind->m_waiting_for_input = true;

            }

        }
        else if (ImGui::GetIO().MouseClicked[1] && hovered) {
            ImGui::OpenPopup(m_bind->get_bind_name().c_str());
        }
        else if (ImGui::GetIO().MouseClicked[0] && !hovered) {
            if (g.ActiveId == id)
                ImGui::ClearActiveID();
        }

        if (m_bind->m_waiting_for_input)
            if (m_bind->bind_new_key())
            {

                ImGui::ClearActiveID();
                m_bind->m_waiting_for_input = false;

            }

        window->DrawList->AddRectFilled(bb.Min, bb.Max, ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_ChildBg)), ImGui::GetStyle().FrameRounding);

        if (ImGui::GetStyle().FrameBorderSize)
            window->DrawList->AddRect(bb.Min, bb.Max, ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_Border)), ImGui::GetStyle().FrameRounding);

        const auto text_sz = ImGui::CalcTextSize(name.c_str());
        const auto centered_x = (size.x - text_sz.x) * 0.5f;
        const auto centered_y = (size.y - text_sz.y) * 0.3f;

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 0.6f));

        if (m_bind->m_waiting_for_input)
            window->DrawList->AddText(ImVec2(bb.Min.x + centered_x, bb.Min.y + centered_y), ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_Text)), name.c_str());
        else
            window->DrawList->AddText(ImVec2(bb.Min.x + centered_x, bb.Min.y + centered_y), ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_Text)), name.c_str());
        ImGui::PopStyleColor();

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

        ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y + 21));
        ImGui::SetNextWindowSize(ImVec2(window_size.x, window_size.y));

        {

            if (ImGui::BeginPopup(m_bind->get_bind_name().c_str(), window_flags)) {
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.f); {
                    ImGui::SetCursorPos(ImVec2(6, 9)); {
                        ImGui::BeginGroup(); {

                            if (ImGui::Selectable("held", m_bind->m_type == keybind_interaction_type::HELD)) m_bind->m_type = keybind_interaction_type::HELD;
                            if (ImGui::Selectable("toggle", m_bind->m_type == keybind_interaction_type::TOGGLE)) m_bind->m_type = keybind_interaction_type::TOGGLE;

                        } ImGui::EndGroup();
                    }
                } ImGui::PopStyleVar();
                ImGui::EndPopup();
            }
        }

        return pressed;

    }

    c_binding* get_bind() const { return m_bind; }

    keybind_display_type get_display_type() const { return m_display_type; }

};

#endif