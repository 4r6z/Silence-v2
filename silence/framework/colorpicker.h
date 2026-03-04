#pragma once

#include "element.h"

#ifdef USE_CUSTOM_COLORS

class c_color
{

public:
    float r{}, g{}, b{}, a{};

    inline c_color() noexcept
    {

        this->r = this->g = this->b = this->a = 1.f;

    }

    inline c_color(int r, int g, int b, int a) noexcept
    {

        this->r = r / 255.f;
        this->g = g / 255.f;
        this->b = b / 255.f;
        this->a = a / 255.f;

    }

    operator ImColor() const { return ImVec4(r, g, b, a); }

    ImU32 u32()
    {
        return IM_COL32(r * 255.f, g * 255.f, b * 255.f, a * 255.f);
    }

    void setColor(int r, int g, int b, int a)
    {
        this->r = r / 255.f;
        this->g = g / 255.f;
        this->b = b / 255.f;
        this->a = a / 255.f;
    }

};

__forceinline c_color from_u32(unsigned int col)
{

    c_color ret{};
    ret.r = ((col) & 0xFF) / 255.0;
    ret.g = ((col >> IM_COL32_G_SHIFT) & 0xFF) / 255.0;
    ret.b = ((col >> IM_COL32_B_SHIFT) & 0xFF) / 255.0;
    ret.a = ((col >> IM_COL32_A_SHIFT) & 0xFF) / 255.0;

    return ret;

}

class c_colorpicker : public c_element
{

private:

    c_color* m_col{};

public:

    inline c_colorpicker(const std::string_view& name, const std::string_view& label, c_color* value, const bool default_visibility = true) noexcept
    {

        this->m_label = label;
        this->m_name = name;
        this->m_visible = default_visibility;
        this->m_col = value;
        this->m_type = framework_enums::element_type::COLORPICKER;

    }

    virtual bool render()
    {

        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        //ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - (24 * 2));
        //ImGui::SameLine(ImGui::GetContentRegionAvail().x - 24.f);
        ImGui::Text(m_label.c_str()); ImGui::SameLine(ImGui::GetContentRegionAvail().x - 24.f);

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const float square_sz = ImGui::GetFrameHeight();
        const float w_full = ImGui::CalcItemWidth();
        const float w_button = (NULL & ImGuiColorEditFlags_NoSmallPreview) ? 0.0f : (square_sz + style.ItemInnerSpacing.x);
        const float w_inputs = w_full - w_button;
        const char* label_display_end = ImGui::FindRenderedTextEnd(m_label.c_str());
        g.NextItemData.ClearFlags();

        c_color* real_color = m_col;

        auto col = &real_color->r;

        ImGui::BeginGroup();
        ImGui::PushID(m_label.c_str());

        auto flags = NULL;

        // If we're not showing any slider there's no point in doing any HSV conversions
        const ImGuiColorEditFlags flags_untouched = flags;
        //if (flags & ImGuiColorEditFlags_NoInputs)
        //    flags = (flags & (~ImGuiColorEditFlags__DisplayMask)) | ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoOptions;

        // Context menu: display and modify options (before defaults are applied)
        //if (!(flags & ImGuiColorEditFlags_NoOptions))
        //    ColorEditOptionsPopup(col, flags);

        flags |= ImGuiColorEditFlags_NoLabel;
        flags |= ImGuiColorEditFlags_NoOptions;
        flags |= ImGuiColorEditFlags_NoDragDrop;
        flags |= ImGuiColorEditFlags_NoTooltip;

        // Read stored options
        //if (!(flags & ImGuiColorEditFlags__DisplayMask))
        //    flags |= (g.ColorEditOptions & ImGuiColorEditFlags__DisplayMask);
        //if (!(flags & ImGuiColorEditFlags__DataTypeMask))
        //    flags |= (g.ColorEditOptions & ImGuiColorEditFlags__DataTypeMask);
        //if (!(flags & ImGuiColorEditFlags__PickerMask))
        //    flags |= (g.ColorEditOptions & ImGuiColorEditFlags__PickerMask);
        //if (!(flags & ImGuiColorEditFlags__InputMask))
        //    flags |= (g.ColorEditOptions & ImGuiColorEditFlags__InputMask);
       // flags |= (g.ColorEditOptions & ~(ImGuiColorEditFlags__DisplayMask | ImGuiColorEditFlags__DataTypeMask | ImGuiColorEditFlags__PickerMask | ImGuiColorEditFlags__InputMask));
        //IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags__DisplayMask)); // Check that only 1 is selected
        //IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags__InputMask));   // Check that only 1 is selected

        const bool alpha = (flags & ImGuiColorEditFlags_NoAlpha) == 0;
        const bool hdr = (flags & ImGuiColorEditFlags_HDR) != 0;
        const int components = alpha ? 4 : 3;

        // Convert to the formats we need
        float f[4] = { col[0], col[1], col[2], alpha ? col[3] : 1.0f };
        if ((flags & ImGuiColorEditFlags_InputHSV) && (flags & ImGuiColorEditFlags_DisplayRGB))
            ImGui::ColorConvertHSVtoRGB(f[0], f[1], f[2], f[0], f[1], f[2]);
        else if ((flags & ImGuiColorEditFlags_InputRGB) && (flags & ImGuiColorEditFlags_DisplayHSV))
        {
            // Hue is lost when converting from greyscale rgb (saturation=0). Restore it.
            ImGui::ColorConvertRGBtoHSV(f[0], f[1], f[2], f[0], f[1], f[2]);
            if (memcmp((void*)g.ColorEditSavedColor, col, sizeof(float) * 3) == 0)
            {
                if (f[1] == 0)
                    f[0] = g.ColorEditSavedHue;
                if (f[2] == 0)
                    f[1] = g.ColorEditSavedHue;
            }
        }
        int i[4] = { IM_F32_TO_INT8_UNBOUND(f[0]), IM_F32_TO_INT8_UNBOUND(f[1]), IM_F32_TO_INT8_UNBOUND(f[2]), IM_F32_TO_INT8_UNBOUND(f[3]) };

        bool value_changed = false;
        bool value_changed_as_float = false;

        const ImVec2 pos = window->DC.CursorPos;
        const float inputs_offset_x = (style.ColorButtonPosition == ImGuiDir_Left) ? w_button : 0.0f;
        window->DC.CursorPos.x = pos.x + inputs_offset_x;

        flags |= ImGuiColorEditFlags_NoInputs;

        //ImGui::PushFontShadow(IM_COL32_BLACK);

        if ((flags & (ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_DisplayHSV)) != 0 && (flags & ImGuiColorEditFlags_NoInputs) == 0)
        {
            // RGB/HSV 0..255 Sliders
            const float w_item_one = ImMax(1.0f, IM_FLOOR((w_inputs - (style.ItemInnerSpacing.x) * (components - 1)) / (float)components));
            const float w_item_last = ImMax(1.0f, IM_FLOOR(w_inputs - (w_item_one + style.ItemInnerSpacing.x) * (components - 1)));

            const bool hide_prefix = (w_item_one <= ImGui::CalcTextSize((flags & ImGuiColorEditFlags_Float) ? "M:0.000" : "M:000").x);
            static const char* ids[4] = { "##X", "##Y", "##Z", "##W" };
            static const char* fmt_table_int[3][4] =
            {
                {   "%3d",   "%3d",   "%3d",   "%3d" }, // Short display
                { "R:%3d", "G:%3d", "B:%3d", "A:%3d" }, // Long display for RGBA
                { "H:%3d", "S:%3d", "V:%3d", "A:%3d" }  // Long display for HSVA
            };
            static const char* fmt_table_float[3][4] =
            {
                {   "%0.3f",   "%0.3f",   "%0.3f",   "%0.3f" }, // Short display
                { "R:%0.3f", "G:%0.3f", "B:%0.3f", "A:%0.3f" }, // Long display for RGBA
                { "H:%0.3f", "S:%0.3f", "V:%0.3f", "A:%0.3f" }  // Long display for HSVA
            };
            const int fmt_idx = hide_prefix ? 0 : (flags & ImGuiColorEditFlags_DisplayHSV) ? 2 : 1;

            for (int n = 0; n < components; n++)
            {
                if (n > 0)
                    ImGui::SameLine(0, style.ItemInnerSpacing.x);
                ImGui::SetNextItemWidth((n + 1 < components) ? w_item_one : w_item_last);

                // FIXME: When ImGuiColorEditFlags_HDR flag is passed HS values snap in weird ways when SV values go below 0.
                if (flags & ImGuiColorEditFlags_Float)
                {
                    value_changed |= ImGui::DragFloat(ids[n], &f[n], 1.0f / 255.0f, 0.0f, hdr ? 0.0f : 1.0f, fmt_table_float[fmt_idx][n]);
                    value_changed_as_float |= value_changed;
                }
                else
                {
                    value_changed |= ImGui::DragInt(ids[n], &i[n], 1.0f, 0, hdr ? 0 : 255, fmt_table_int[fmt_idx][n]);
                }
                if (!(flags & ImGuiColorEditFlags_NoOptions))
                    ImGui::OpenPopupOnItemClick("context");
            }
        }
        else if ((flags & ImGuiColorEditFlags_DisplayHex) != 0)
        {
            // RGB Hexadecimal Input
            char buf[64];
            if (alpha)
                ImFormatString(buf, IM_ARRAYSIZE(buf), "#%02X%02X%02X%02X", ImClamp(i[0], 0, 255), ImClamp(i[1], 0, 255), ImClamp(i[2], 0, 255), ImClamp(i[3], 0, 255));
            else
                ImFormatString(buf, IM_ARRAYSIZE(buf), "#%02X%02X%02X", ImClamp(i[0], 0, 255), ImClamp(i[1], 0, 255), ImClamp(i[2], 0, 255));
            ImGui::SetNextItemWidth(w_inputs);
            if (ImGui::InputText("##Text", buf, IM_ARRAYSIZE(buf), ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase))
            {
                value_changed = true;
                char* p = buf;
                while (*p == '#' || ImCharIsBlankA(*p))
                    p++;
                i[0] = i[1] = i[2] = 0;
                i[3] = 0xFF; // alpha default to 255 is not parsed by scanf (e.g. inputting #FFFFFF omitting alpha)
                int r;
                if (alpha)
                    r = sscanf(p, "%02X%02X%02X%02X", (unsigned int*)&i[0], (unsigned int*)&i[1], (unsigned int*)&i[2], (unsigned int*)&i[3]); // Treat at unsigned (%X is unsigned)
                else
                    r = sscanf(p, "%02X%02X%02X", (unsigned int*)&i[0], (unsigned int*)&i[1], (unsigned int*)&i[2]);
                IM_UNUSED(r); // Fixes C6031: Return value ignored: 'sscanf'.
            }
            if (!(flags & ImGuiColorEditFlags_NoOptions))
                ImGui::OpenPopupOnItemClick("context");
        }

        ImGuiWindow* picker_active_window = NULL;
        if (!(flags & ImGuiColorEditFlags_NoSmallPreview))
        {
            const float button_offset_x = ((flags & ImGuiColorEditFlags_NoInputs) || (style.ColorButtonPosition == ImGuiDir_Left)) ? 0.0f : w_inputs + style.ItemInnerSpacing.x;
            window->DC.CursorPos = ImVec2(pos.x + button_offset_x, pos.y);

            const ImVec4 col_v4(col[0], col[1], col[2], alpha ? col[3] : 1.0f);
            if (ImGui::ColorButton("##ColorButton", col_v4, flags))
            {
                if (!(flags & ImGuiColorEditFlags_NoPicker))
                {
                    // Store current color and open a picker
                    g.ColorPickerRef = col_v4;
                    ImGui::OpenPopup("picker");
                    //ImGui::SetNextWindowPos(window->DC.La.GetBL() + ImVec2(-1, style.ItemSpacing.y));
                }
            }
            if (!(flags & ImGuiColorEditFlags_NoOptions))
                ImGui::OpenPopupOnItemClick("context");

            if (ImGui::BeginPopup("picker"))
            {
                picker_active_window = g.CurrentWindow;
                if (m_label.c_str() != label_display_end)
                {
                    ImGui::TextEx(m_label.c_str(), label_display_end);
                    ImGui::Spacing();
                }
                ImGuiColorEditFlags picker_flags_to_forward = ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_AlphaBar;
                ImGuiColorEditFlags picker_flags = (flags_untouched & picker_flags_to_forward) | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_AlphaBar;
                ImGui::SetNextItemWidth(square_sz * 12.0f); // Use 256 + bar sizes?
                value_changed |= ImGui::ColorPicker4("##picker", col, picker_flags, &g.ColorPickerRef.x);
                //ImGui::Checkbox("enable rainbow", &real_color->rainbow);
                //ImGui::SliderFloat("rainbow speed", &real_color->rainbow_speed, 0.1f, 10.f);
                ImGui::EndPopup();
            }
        }

        if (m_label.c_str() != label_display_end && !(flags & ImGuiColorEditFlags_NoLabel))
        {
            const float text_offset_x = (flags & ImGuiColorEditFlags_NoInputs) ? w_button : w_full + style.ItemInnerSpacing.x;
            window->DC.CursorPos = ImVec2(pos.x + text_offset_x, pos.y + style.FramePadding.y);
            ImGui::TextEx(m_label.c_str(), label_display_end);
        }

        // Convert back
        if (value_changed && picker_active_window == NULL)
        {
            if (!value_changed_as_float)
                for (int n = 0; n < 4; n++)
                    f[n] = i[n] / 255.0f;
            if ((flags & ImGuiColorEditFlags_DisplayHSV) && (flags & ImGuiColorEditFlags_InputRGB))
            {
                g.ColorEditSavedHue = f[0];
                g.ColorEditSavedSat = f[1];
                ImGui::ColorConvertHSVtoRGB(f[0], f[1], f[2], f[0], f[1], f[2]);
                memcpy((void*)g.ColorEditSavedColor, f, sizeof(float) * 3);
            }
            if ((flags & ImGuiColorEditFlags_DisplayRGB) && (flags & ImGuiColorEditFlags_InputHSV))
                ImGui::ColorConvertRGBtoHSV(f[0], f[1], f[2], f[0], f[1], f[2]);

            col[0] = f[0];
            col[1] = f[1];
            col[2] = f[2];
            if (alpha)
                col[3] = f[3];
        }

        ImGui::PopID();
        ImGui::EndGroup();

        // Drag and Drop Target
        // NB: The flag test is merely an optional micro-optimization, BeginDragDropTarget() does the same test.
        //if ((window->DC.FG & ImGuiItemStatusFlags_HoveredRect) && !(flags & ImGuiColorEditFlags_NoDragDrop) && BeginDragDropTarget())
        //{
        //    bool accepted_drag_drop = false;
        //    if (const ImGuiPayload* payload = AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_3F))
        //    {
        //        memcpy((float*)col, payload->Data, sizeof(float) * 3); // Preserve alpha if any //-V512
        //        value_changed = accepted_drag_drop = true;
        //    }
        //    if (const ImGuiPayload* payload = AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_4F))
        //    {
        //        memcpy((float*)col, payload->Data, sizeof(float) * components);
        //        value_changed = accepted_drag_drop = true;
        //    }

        //    // Drag-drop payloads are always RGB
        //    if (accepted_drag_drop && (flags & ImGuiColorEditFlags_InputHSV))
        //        ColorConvertRGBtoHSV(col[0], col[1], col[2], col[0], col[1], col[2]);
        //    EndDragDropTarget();
        //}

        // When picker is being actively used, use its active id so IsItemActive() will function on ColorEdit4().
        if (picker_active_window && g.ActiveId != 0 && g.ActiveIdWindow == picker_active_window)
            window->NavLastIds[0] = g.ActiveId;

        if (value_changed)
            ImGui::MarkItemEdited(window->NavLastIds[0]);

        //ImGui::PopFontShadow();

        return value_changed;

    }

};

#endif
