#pragma once

#include "element.h"

class c_input_text : public c_element
{

private:

	char* m_text{};
	size_t m_text_size{};
    ImGuiInputTextFlags m_flags{};

public:

    inline c_input_text(const std::string_view& name, const std::string_view& label, char* value, size_t size, ImGuiInputTextFlags flags = NULL, const bool default_visibility = true) noexcept
    {

        this->m_label = label;
        this->m_name = name;
        this->m_visible = default_visibility;
        this->m_text_size = size;
        this->m_text = value;
        this->m_type = framework_enums::element_type::INPUT_TEXT;
        this->m_flags = flags;

    }

    virtual bool render() 
    { 

        //ImGui::SetCursorPosX((330.f - 204.f) * 0.5f);
        ImGui::Text(m_label.c_str());
        //ImGui::SetNextItemWidth(204.f);
        //ImGui::SetCursorPosX((330.f - 204.f) * 0.5f);
        return ImGui::InputText(std::string("##" + m_label).c_str(), m_text, m_text_size, m_flags);
    
    }

};
