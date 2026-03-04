#pragma once

#include "includes.h"

namespace framework_enums
{

    enum element_type : int16_t
    {

        LABEL,
        CHECKBOX,
        SLIDER_INT,
        SLIDER_FLOAT,
        SINGLE_COMBO,
        MULTI_COMBO,
        BUTTON,
        DIVIDER,
        LABEL_DIVIDER,
        SINGLE_LISTBOX,
        MULTI_LISTBOX,
        COLORPICKER,
        PROGRESS_BAR,
        KEYBIND,
        TAB,
        GROUP,
        SWITCH,
        INPUT_TEXT,
        CLICK_LABEL

    };

    enum element_flags : int16_t
    {

        WITH_SECONDARY_LABEL,
        WITH_ANIMATION

    };

}

class c_element
{

    // look at all this guy's friends... woah!

    friend class c_window;
    friend class c_child;

    friend class c_label;
    friend class c_checkbox;
    friend class c_slider_int;
    friend class c_slider_float;
    friend class c_combo;
    friend class c_multi_combo;
    friend class c_button;
    friend class c_divider;
    friend class c_label_divider;
    friend class c_single_listbox;
    friend class c_multi_listbox;
    friend class c_single_listbox;
    friend class c_colorpicker;
    friend class c_keybind;
    friend class c_tab;
    friend class c_group;
    friend class c_toggle;
    friend class c_collapsable_group;
    friend class c_color_label;
    friend class c_input_text;
    friend class c_click_label;
    friend class c_gradient_label;
    friend class c_progress_bar;

private:

    std::string m_name{};
    std::string m_label{};
    std::function<bool(void)> m_visibility_callback{};
    std::function<void(void)> m_render_callback{};
    std::function<void(void)> m_interaction_callback{};
    std::function<void(void)> m_start_render_callback{};
    ImFont* m_custom_font{ NULL };
    ImVec2 m_custom_position{};
    bool m_has_custom_position{};
    bool m_center_x{ false };
    bool m_visible{};
    bool m_wants_same_line{};
    framework_enums::element_type m_type{};
    framework_enums::element_flags m_flags{};

    void update_visibility(const bool force_visibility = false) noexcept
    {

        if (!force_visibility && m_visibility_callback) // automatically set visibility based on the callback, else force visibility via parameter.
            m_visible = m_visibility_callback();
        else if (!force_visibility && m_visibility_callback)
            m_visible = force_visibility;

    }

    void run_render_callback() noexcept
    {

        if (m_render_callback)
            m_render_callback();

    }

    void run_interaction_callback() noexcept
    {

        if (m_interaction_callback)
            m_interaction_callback();

    }

    virtual bool render() = 0;

public:

    // dis hoe aint need no constructor...

    bool get_visible() const noexcept { return m_visible; }
    void set_visible(const bool value) noexcept { m_visible = value; }

    void set_visibility_callback(const std::function<bool(void)>& value) { m_visibility_callback = value; } // callback to allow users to change visibility based on variables
    void set_render_callback(const std::function<void(void)>& value) { m_render_callback = value; } // callback before rendering of the element
    void set_interaction_callback(const std::function<void(void)>& value) { m_interaction_callback = value; } // callback on interaction with an element
    void set_start_render_callback(const std::function<void(void)>& value) { m_start_render_callback = value; } // callback on interaction with an element

    std::string get_name() const noexcept { return m_name; }

    void set_label(const std::string_view& label) { m_label = label; }

    void set_same_line(bool same_line) { m_wants_same_line = same_line; }

    void set_custom_position(const ImVec2& postion) { m_has_custom_position = true; m_custom_position = postion; }

};
