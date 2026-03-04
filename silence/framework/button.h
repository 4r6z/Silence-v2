#pragma once

#include "includes.h"

#include "element.h"

class c_button : public c_element
{

private:

    ImVec2 m_size{};
    bool m_full_width{ false };

public:

    c_button(const std::string_view& name, const std::string_view& label, const ImVec2& size = ImVec2(0, 0), const bool default_visibility = true)
    {

        this->m_label = label;
        this->m_name = name;
        this->m_size = size;
        this->m_visible = default_visibility;
        this->m_type = framework_enums::element_type::BUTTON;

    }

    virtual bool render() { return ImGui::Button(m_label.c_str(), m_size); }

    void set_full_width(bool val) { m_full_width = val; }

};
