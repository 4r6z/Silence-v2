#include "imgui/imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"
#include "imgui/imgui_freetype.h"
#include <dwmapi.h>
#include <d3d11.h>
#include <tchar.h>
#include <filesystem>
#include <thread>
#include <vector>
#include <string>
#include <algorithm>

#include "Overlay.hpp"
#include "../rbx/esp/ESP.hpp"
#include "../rbx/aimbot/Aimbot.hpp"
#include "../rbx/globals/GlobalVars.hpp"
#include "../rbx/Configurations/Configurations.hpp"

static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

inline static ImFont* verdana_12{ NULL };
ImFont* big_verdana = nullptr;

#include "../framework/framework.h"

c_window cheat_window("cheat_window", { 604, 334 }, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

c_animator<float> alpha_animator{ 0.f, 1.f, 0.32f };

c_single_listbox* player_listbox = nullptr;
c_child* whitelist_child = nullptr;
c_label* selected_player_label = nullptr;

float top_bar_sz = 35.f;
float tab_bar_sz = 146.f;
float bottom_bar_sz = 28.f;

int current_tab = 0;

bool menu_open = false;

std::vector<BYTE> mc_vals = {};

int current_cfg = 0;
std::vector<std::string> cfg_items = {};

char cfgName[20];

const void savecfg() {
    configs::save(cfgName);
    return;
}

const void loadcfg() {
    configs::load(cfg_items[current_cfg].c_str());
    return;
}

const void res_hipheight() {
    GlobalVars::hipheight_val = GlobalVars::original_hipheight;
    return;
}

std::vector<PartState> partStates = {
    {"head", false},
    {"rootPart", false},
    {"upperTorso", false},
    {"lowerTorso", false},
    {"leftUpperLeg", false},
    {"leftFoot", false},
    {"rightFoot", false},
    {"leftUpperArm", false},
    {"leftHand", false},
    {"rightUpperArm", false},
    {"rightHand", false},
};

void setup_combat_tab()
{
    static auto show_if_tab = []() -> bool {
        return current_tab == 0;
        };

    auto main_child = cheat_window.insert_child("aimbot", { 130.f, 15.f }, { 222.f, 305.f });
    {

        main_child->insert_element<c_toggle>("aim_enable", "enable aimbot", &GlobalVars::aimbot);

        main_child->insert_element<c_keybind>("aim_key", "aimbot hotkey", &GlobalVars::aimbot_bind);

        main_child->insert_element<c_toggle>("aim_sticky", "sticky aim", &GlobalVars::aimbot_sticky);

        main_child->insert_element<c_multi_combo>("aim_checks", "checks", &GlobalVars::aimbot_checks, std::vector<std::string>{ "knocked", "grabbed", "team"});

        main_child->insert_element<c_combo>("aim_hbx", "hitbox", &GlobalVars::aimbot_part, std::vector<std::string>{ "head", "root part", "upper chest", "lower chest" });

        main_child->insert_element<c_combo>("aim_easingstyle", "easing styles", &GlobalVars::aimbot_easing_style, std::vector<std::string>{ "linear", "sine", "quad", "cubic", "quart", "exponential", "circular", "back", "bounce", "elastic" });

        main_child->insert_element<c_toggle>("aim_nearestpart", "nearest part", &GlobalVars::closest_part);

        main_child->insert_element<c_toggle>("aim_resolver", "resolver", &GlobalVars::resolver);

        main_child->insert_element<c_toggle>("aim_infov", "in fov", &GlobalVars::in_fov_only);

    } main_child->set_visibility_callback(show_if_tab);

    auto other_child = cheat_window.insert_child("options", { 367.f, 15.f }, { 222.f, 305.f });
    {
        auto camera_type = other_child->insert_element<c_combo>("aim_type", "aimbot type", &GlobalVars::aimbot_type, std::vector<std::string>{ "mouse", "camera", "free aim"});

        auto camera_smoothness = other_child->insert_element<c_slider_float>("aim_smoothness", "camera smoothness", &GlobalVars::smoothness_camera, 0.1f, 100.f, "%.3f smoothness");
        auto camera_prediction = other_child->insert_element<c_toggle>("camera_prediction", "enable camera prediction", &GlobalVars::camera_prediction);
        auto camera_prediction_x = other_child->insert_element<c_slider_float>("aim_cameraprediction_x", "camera prediction x", &GlobalVars::camera_prediction_x, 1.f, 12, "%.3f prediction x");
        auto camera_prediction_y = other_child->insert_element<c_slider_float>("aim_cameraprediction_y", "camera prediction y", &GlobalVars::camera_prediction_y, 1.f, 12, "%.3f prediction y");

        camera_smoothness->set_visibility_callback([camera_type]() -> int {
            return camera_type->get_value() == 1;
            });

        camera_prediction->set_visibility_callback([camera_type]() -> int {
            return camera_type->get_value() == 1;
            });

        camera_prediction_x->set_visibility_callback([camera_type, camera_prediction]() -> int {
            return camera_type->get_value() == 1 && camera_prediction->get_value() == 1;
            });

        camera_prediction_y->set_visibility_callback([camera_type, camera_prediction]() -> int {
            return camera_type->get_value() == 1 && camera_prediction->get_value() == 1;
            });

        auto mouse_sensitivity = other_child->insert_element<c_slider_float>("aim_sensitivity", "mouse sensitivity", &GlobalVars::mouse_sensitivity, 0.1f, 5.f, "sensitivity %.3f");
        auto mouse_smoothness = other_child->insert_element<c_slider_float>("aim_smoothness", "mouse smoothness", &GlobalVars::smoothness_mouse, 1.f, 100.f, "%.3f smoothness");

        auto mouse_prediction = other_child->insert_element<c_toggle>("mouse_prediction", "enable mouse prediction", &GlobalVars::mouse_prediction);

        auto mouse_prediction_x = other_child->insert_element<c_slider_float>("aim_mouseprediction_x", "mouse prediction x", &GlobalVars::mouse_prediction_x, 1.f, 12.f, "%.3f prediction x");
        auto mouse_prediction_y = other_child->insert_element<c_slider_float>("aim_mouseprediction_y", "mouse prediction y", &GlobalVars::mouse_prediction_y, 1.1f, 12.f, "%.3f prediction y");

        mouse_sensitivity->set_visibility_callback([camera_type]() -> int {
            return camera_type->get_value() == 0;
            });

        mouse_smoothness->set_visibility_callback([camera_type]() -> int {
            return camera_type->get_value() == 0;
            });

        mouse_prediction->set_visibility_callback([camera_type]() -> int {
            return camera_type->get_value() == 0;
            });

        mouse_prediction_x->set_visibility_callback([camera_type, mouse_prediction]() -> int {
            return camera_type->get_value() == 0 && mouse_prediction->get_value() == 1;
            });

        mouse_prediction_y->set_visibility_callback([camera_type, mouse_prediction]() -> int {
            return camera_type->get_value() == 0 && mouse_prediction->get_value() == 1;
            });

        auto free_aim_sensitivity = other_child->insert_element<c_slider_float>("aim_free_aimsensitivity", "free aim sensitivity", &GlobalVars::free_aim_sensitivity, 0.1f, 5.f, "sensitivity %.3f");
        auto free_aim_smoothness = other_child->insert_element<c_slider_float>("aim_free_aimsmoothness", "free aim smoothness", &GlobalVars::smoothness_free_aim, 1.f, 100.f, "%.3f smoothness");

        auto free_aim_prediction = other_child->insert_element<c_toggle>("free_aim_prediction", "enable free aim prediction", &GlobalVars::free_aim_prediction);

        auto free_aim_prediction_x = other_child->insert_element<c_slider_float>("aim_free_aimprediction_x", "free aim prediction x", &GlobalVars::free_aim_prediction_x, 1.f, 12.f, "%.3f prediction x");
        auto free_aim_prediction_y = other_child->insert_element<c_slider_float>("aim_free_aimprediction_y", "free aim prediction y", &GlobalVars::free_aim_prediction_y, 1.1f, 12.f, "%.3f prediction y");

        free_aim_sensitivity->set_visibility_callback([camera_type]() -> int {
            return camera_type->get_value() == 2;
            });

        free_aim_smoothness->set_visibility_callback([camera_type]() -> int {
            return camera_type->get_value() == 2;
            });

        free_aim_prediction->set_visibility_callback([camera_type]() -> int {
            return camera_type->get_value() == 2;
            });

        free_aim_prediction_x->set_visibility_callback([camera_type, free_aim_prediction]() -> int {
            return camera_type->get_value() == 2 && free_aim_prediction->get_value() == 1;
            });

        free_aim_prediction_y->set_visibility_callback([camera_type, free_aim_prediction]() -> int {
            return camera_type->get_value() == 2 && free_aim_prediction->get_value() == 1;
            });

        other_child->insert_element<c_label_divider>("shake_diviser", "shake");

        auto shake_toggle = other_child->insert_element<c_toggle>("aim_shake", "enable aimbot shake", &GlobalVars::shake);

        auto shake_x = other_child->insert_element<c_slider_float>("aim_shake_x", "shake x", &GlobalVars::shake_x, 0.f, 5.f, "%.2f shake x");
        auto shake_y = other_child->insert_element<c_slider_float>("aim_shake_y", "shake y", &GlobalVars::shake_y, 0.f, 5.f, "%.2f shake y");

        shake_x->set_visibility_callback([shake_toggle]() -> bool {
            return shake_toggle->get_value() == true;
            });

        shake_y->set_visibility_callback([shake_toggle]() -> bool {
            return shake_toggle->get_value() == true;
            });

    } other_child->set_visibility_callback(show_if_tab);

}

void setup_triggerbot_tab()
{
    static auto show_if_tab = []() -> bool {
        return current_tab == 1;
        };

    auto main_child = cheat_window.insert_child("triggerbot", { 130.f, 15.f }, { 222.f, 305.f });
    {

        main_child->insert_element<c_toggle>("trigger_enable", "enable triggerbot", &GlobalVars::triggerbot);

        main_child->insert_element<c_slider_float>("trigger_radius", "trigger radius", &GlobalVars::triggerbot_fov_radius, 0.1f, 200.f, "%.3f radius");

    } main_child->set_visibility_callback(show_if_tab);
}

void setup_visuals_tab()
{
    static auto show_if_tab = []() -> bool {
        return current_tab == 2;
        };

    auto main_child = cheat_window.insert_child("visuals", { 130.f, 15.f }, { 222.f, 305.f });
    {

        auto esp_toggle = main_child->insert_element<c_toggle>("vis_enable", "enable esp", &GlobalVars::esp);

        auto esp_checks = main_child->insert_element<c_multi_combo>("esp_checks", "checks", &GlobalVars::esp_checks, std::vector<std::string>{ "knocked", "grabbed", "team"});

        auto esp_renderdist = main_child->insert_element<c_slider_float>("vis_render_distance", "max render distance", &GlobalVars::max_render_distance, 10.f, 1250.f, "%.2f studs");

        auto box_toggle = main_child->insert_element<c_toggle>("vis_box", "box", &GlobalVars::box_esp);

        auto health_toggle = main_child->insert_element<c_toggle>("vis_health", "health bar", &GlobalVars::health_bar);

        auto shield_toggle = main_child->insert_element<c_toggle>("vis_shield", "shield bar", &GlobalVars::shield_bar);

        auto names_toggle = main_child->insert_element<c_toggle>("vis_name", "name", &GlobalVars::name_esp);

        auto distance_toggle = main_child->insert_element<c_toggle>("vis_distance", "distance", &GlobalVars::distance_esp);

        auto skeletons_toggle = main_child->insert_element<c_toggle>("vis_skeletons", "skeleton", &GlobalVars::skeleton_esp);

        auto tracers_toggle = main_child->insert_element<c_toggle>("vis_tracers", "snaplines", &GlobalVars::snapline_esp);

        auto circle_toggle = main_child->insert_element<c_toggle>("vis_circle", "player circle", &GlobalVars::undercircle_esp);

        auto looktracer_toggle = main_child->insert_element<c_toggle>("vis_looktracer", "aim line", &GlobalVars::looktracer);

        auto aimviewer_toggle = main_child->insert_element<c_toggle>("vis_aimview", "aim viewer", &GlobalVars::aimviewer);

        auto prediction_dot_toggle = main_child->insert_element<c_toggle>("vis_preddot", "visualize prediction", &GlobalVars::prediction_dot);

        auto fovchanger_toggle = main_child->insert_element<c_toggle>("aim_fov", "fov changer", &GlobalVars::camera_fieldOfViewChange);

        auto drawfov_toggle = main_child->insert_element<c_toggle>("vis_fov", "draw fov", &GlobalVars::drawFov);

        esp_checks->set_visibility_callback([esp_toggle]() -> bool {
            return esp_toggle->get_value();
            });

        esp_renderdist->set_visibility_callback([esp_toggle]() -> bool {
            return esp_toggle->get_value();
            });

        box_toggle->set_visibility_callback([esp_toggle]() -> bool {
            return esp_toggle->get_value();
            });

        circle_toggle->set_visibility_callback([esp_toggle]() -> bool {
            return esp_toggle->get_value();
            });

        skeletons_toggle->set_visibility_callback([esp_toggle]() -> bool {
            return esp_toggle->get_value();
            });

        names_toggle->set_visibility_callback([esp_toggle]() -> bool {
            return esp_toggle->get_value();
            });

        distance_toggle->set_visibility_callback([esp_toggle]() -> bool {
            return esp_toggle->get_value();
            });

        tracers_toggle->set_visibility_callback([esp_toggle]() -> bool {
            return esp_toggle->get_value();
            });

        looktracer_toggle->set_visibility_callback([esp_toggle]() -> bool {
            return esp_toggle->get_value();
            });

        health_toggle->set_visibility_callback([esp_toggle]() -> bool {
            return esp_toggle->get_value();
            });

        shield_toggle->set_visibility_callback([esp_toggle]() -> bool {
            return esp_toggle->get_value();
            });

        prediction_dot_toggle->set_visibility_callback([esp_toggle]() -> bool {
            return esp_toggle->get_value();
            });

        aimviewer_toggle->set_visibility_callback([esp_toggle]() -> bool {
            return esp_toggle->get_value();
            });

    } main_child->set_visibility_callback(show_if_tab);

    auto other_child = cheat_window.insert_child("options", { 367.f, 15.f }, { 222.f, 305.f });
    {

        other_child->insert_element<c_colorpicker>("box_color", "box color", &GlobalVars::box_color);
        other_child->insert_element<c_colorpicker>("filledbox_color", "filled box color", &GlobalVars::filled_box_color);
        other_child->insert_element<c_colorpicker>("name_color", "name color", &GlobalVars::name_color);
        other_child->insert_element<c_colorpicker>("distance_color", "distance color", &GlobalVars::distance_color);
        other_child->insert_element<c_colorpicker>("skeleton_color", "skeleton color", &GlobalVars::skeleton_color);
        other_child->insert_element<c_colorpicker>("tracer_color", "snaplines color", &GlobalVars::snapline_color);

        other_child->insert_element<c_colorpicker>("looktracer_color", "aim line color", &GlobalVars::looktracer_color);
        other_child->insert_element<c_colorpicker>("aimview_color", "aim viewer color", &GlobalVars::aimviewer_color);
        other_child->insert_element<c_colorpicker>("dot_color", "visualize prediction color", &GlobalVars::prediction_dot_color);
        other_child->insert_element<c_colorpicker>("undercircle_color", "circle color", &GlobalVars::undercircle_color);

        other_child->insert_element<c_label_divider>("boxes_diviser", "box");
        other_child->insert_element<c_combo>("box_type", "box type", &GlobalVars::box_type, std::vector<std::string>{ "static standard", "static cornered" });
        other_child->insert_element<c_toggle>("box_outline", "box filled", &GlobalVars::box_filled);

        other_child->insert_element<c_label_divider>("fov_diviser", "fov");
        other_child->insert_element<c_slider_int>("fov_radius", "fov radius", &GlobalVars::fovRadius, 10, 600, "%d pixels");

        other_child->insert_element<c_label_divider>("snapline_diviser", "snaplines");
        other_child->insert_element<c_combo>("snapline_opt", "snaplines options", &GlobalVars::snapline_position, std::vector<std::string>{ "bottom", "top", "mouse"});

        other_child->insert_element<c_label_divider>("name_diviser", "name");
        other_child->insert_element<c_toggle>("name_outline", "name outline", &GlobalVars::name_outline);

        other_child->insert_element<c_label_divider>("distance_diviser", "distance");
        other_child->insert_element<c_toggle>("snapline_outline", "distance outline", &GlobalVars::distance_outline);

        other_child->insert_element<c_label_divider>("undercircle_diviser", "player circle");
        other_child->insert_element<c_toggle>("undercircle_outline", "filled circle", &GlobalVars::undercircle_filled);
        other_child->insert_element<c_slider_float>("undercircle_radius", "circle radius", &GlobalVars::undercircle_radius, 0.1f, 50.f, "%.3f radius");
        other_child->insert_element<c_slider_int>("undercircle_segments", "circle segments", &GlobalVars::undercircle_segments, 3, 64, "%d segments");

        other_child->insert_element<c_slider_float>("aim_fieldofview", "camera fov", &GlobalVars::camera_fieldOfView, 10.f, 120.f, "%.2f degrees");

    } other_child->set_visibility_callback(show_if_tab);
}

void setup_character_tab()
{
    static auto show_if_tab = []() -> bool {
        return current_tab == 3;
        };

    auto character = cheat_window.insert_child("character", { 130.f, 15.f }, { 222.f, 305.f });
    {

        character->insert_element<c_toggle>("exp_cflytoggle", "cfly", &GlobalVars::cfly_enabled);
        character->insert_element<c_keybind>("cfly_key", "cfly hotkey", &GlobalVars::cfly_bind);
        character->insert_element<c_slider_float>("exp_cflyspeed", "cfly speed", &GlobalVars::fly_factor, 0.f, 20.f, "%.2f speed");

    } character->set_visibility_callback(show_if_tab);
}

void setup_cfg_tab()
{
    static auto show_if_tab = []() -> bool {
        return current_tab == 4;
        };

    auto config = cheat_window.insert_child("configs", { 130.f, 15.f }, { 222.f, 305.f });
    {

        config->insert_element<c_single_listbox>("cfg_lst", "saved configs", &current_cfg, &cfg_items);

        config->insert_element<c_input_text>("cfg_name", "enter a config name", cfgName, 20);

        auto save_cfg = config->insert_element<c_button>("save_cfg", "save");

        save_cfg->set_interaction_callback(savecfg);

        auto load_cfg = config->insert_element<c_button>("load_cfg", "load");
        load_cfg->set_same_line(true);

        load_cfg->set_interaction_callback(loadcfg);

    } config->set_visibility_callback(show_if_tab);

    auto settings = cheat_window.insert_child("overlay options", { 367.f, 15.f }, { 222.f, 305.f });
    {
        settings->insert_element<c_toggle>("set_vsync", "v-sync", &GlobalVars::vsync);
        settings->insert_element<c_toggle>("set_steamproof", "streamproof", &GlobalVars::streamproof);
        settings->insert_element<c_toggle>("set_espcpuusage", "smooth esp", &GlobalVars::highcpuusageesp);

    } settings->set_visibility_callback(show_if_tab);
}

void setup_menu()
{

    auto tabs_child = cheat_window.insert_child("tabs", { 15.f, 15.f }, { 100.f, 304.f }, false);
    {

        tabs_child->insert_element<c_gradient_label>("brand_label", "     silence", IM_COL32(213, 125, 124, 255), IM_COL32(65, 23, 24, 255));

        tabs_child->insert_element<c_divider>("div", "div");

        tabs_child->insert_element<c_click_label>("aim_tab", "aimbot", 0, &current_tab);

        tabs_child->insert_element<c_click_label>("trigger_tab", "trigger-bot", 1, &current_tab);

        tabs_child->insert_element<c_click_label>("vis_tab", "visuals", 2, &current_tab);

        tabs_child->insert_element<c_click_label>("char_tab", "character", 3, &current_tab);

        tabs_child->insert_element<c_click_label>("cfg_tab", "configs", 4, &current_tab);

    }

    cheat_window.insert_attachment([](const ImVec2& pos, const ImVec2& sz, ImDrawList* draw) -> void {

        constexpr auto alpha_freq = 255 / 0.5f;
        static float alpha = 0;
        static bool should_reverse = false;

        if (!should_reverse)
            alpha += alpha_freq * ImGui::GetIO().DeltaTime;
        else
            alpha -= alpha_freq * ImGui::GetIO().DeltaTime;

        alpha = std::clamp(alpha, 0.f, ImGui::GetStyle().Alpha * 255.f);

        if (alpha >= 255.f)
            should_reverse = true;
        else if (alpha <= 0.f)
            should_reverse = false;

        draw->AddRectFilledMultiColor(pos - ImVec2(2, 2), pos + ImVec2(sz + ImVec2(2, 2)), IM_COL32(213, 125, 124, static_cast<int>(alpha)), IM_COL32(213, 125, 124, static_cast<int>(alpha)), IM_COL32(65, 23, 24, static_cast<int>(alpha)), IM_COL32(24, 23, 24, static_cast<int>(alpha)));

        });

    setup_combat_tab();

    setup_triggerbot_tab();

    setup_visuals_tab();

    setup_cfg_tab();

    setup_character_tab();

    cheat_window.set_visibility_callback([]() -> bool {
        return alpha_animator.get_value() > 0.f;
        });

    alpha_animator.set_starting_animation(menu_open);

}

bool Overlay::FullScreen(HWND windowHandle)
{
    MONITORINFO monitorInfo = { sizeof(MONITORINFO) };
    if (GetMonitorInfo(MonitorFromWindow(windowHandle, MONITOR_DEFAULTTOPRIMARY), &monitorInfo))
    {
        RECT windowRect;
        if (GetWindowRect(windowHandle, &windowRect))
        {
            return windowRect.left == monitorInfo.rcMonitor.left
                && windowRect.right == monitorInfo.rcMonitor.right
                && windowRect.top == monitorInfo.rcMonitor.top
                && windowRect.bottom == monitorInfo.rcMonitor.bottom;
        }
    }
}

void Overlay::Move_Window(HWND hw)
{
    HWND target = FindWindowA("WINDOWSCLIENT", NULL);
    HWND foregroundWindow = GetForegroundWindow();

    if (target != foregroundWindow && hw != foregroundWindow)
    {
        MoveWindow(hw, 0, 0, 0, 0, true);
    }
    else
    {
        RECT rect;
        GetWindowRect(target, &rect);

        int rsize_x = rect.right - rect.left;
        int rsize_y = rect.bottom - rect.top;

        if (FullScreen(target))
        {
            rsize_x += 16;

        }
        else
        {
            rsize_y -= 39;
            rect.left += 8;
            rect.top += 31;
        }

        MoveWindow(hw, rect.left, rect.top, rsize_x, rsize_y, TRUE);
    }
}

void Overlay::CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void Overlay::CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void Overlay::CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI Overlay::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam);
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

void InitializeFonts() {
    ImGuiIO& io = ImGui::GetIO();
    verdana_12 = io.Fonts->AddFontFromFileTTF(("C:\\Windows\\Fonts\\Tahoma.ttf"), 13.f);
}

bool Overlay::Render()
{
    ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEX wc{};
    wc.cbClsExtra = NULL;
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.cbWndExtra = NULL;
    wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpfnWndProc = WndProc;
    wc.lpszClassName = TEXT("wc");
    wc.lpszMenuName = nullptr;
    wc.style = CS_VREDRAW | CS_HREDRAW;

    ::RegisterClassEx(&wc);
    HWND hwnd = ::CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE,
        wc.lpszClassName,
        TEXT("hwnd"),
        WS_POPUP,
        0,
        0,
        GetSystemMetrics(SM_CXSCREEN),
        GetSystemMetrics(SM_CYSCREEN),
        nullptr,
        nullptr,
        wc.hInstance,
        nullptr
    );

    SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
    const MARGINS margin = { -1 };
    DwmExtendFrameIntoClientArea(hwnd, &margin);

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        return false;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();
    ImGui::CreateShadowTexture(g_pd3dDevice);

    ImFontConfig fn_cfg{};
    ImFontConfig cfg{};
    ImFontConfig big_cfg{};
    fn_cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_Monochrome | ImGuiFreeTypeBuilderFlags_MonoHinting;
    big_cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_ForceAutoHint;

    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Verdana.ttf", 12.f, &fn_cfg);
    verdana_12 = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Verdana.ttf", 13.f, &cfg);
    big_verdana = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Comfortaa.ttf", 60.f, &big_cfg);
    ImGui::CreateShadowTexture(g_pd3dDevice);

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    setup_menu();

    static ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    static float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };

    DEVMODE dm;
    EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);
    int wait_time = std::floor((static_cast<double>(1000) / dm.dmDisplayFrequency) / 1.5);

    bool done = false;
    std::string configFolder = GetConfigFolderPath();
    HWND rblx = FindWindowA("WINDOWSCLIENT", NULL);

    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }

        if (done)
            break;

        HWND current_rblx = FindWindowA("WINDOWSCLIENT", NULL);
        if (current_rblx != NULL) {
            rblx = current_rblx;
        }

        Move_Window(hwnd);

        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        {

            if (GetForegroundWindow() == rblx || GetForegroundWindow() == hwnd) {
                ImGui::Begin(XorStr("silence_overlay"), nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground);
                {
                    static auto lastTime = std::chrono::high_resolution_clock::now();
                    auto currentTime = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<float> elapsed = currentTime - lastTime;
                    g_DeltaTime = elapsed.count();
                    lastTime = currentTime;

                    ImGui::PushFont(verdana_12);
                    Roblox::hook_esp();
                    ImGui::PopFont();
                }
                ImGui::End();
            }

            alpha_animator.update_animation();

            if ((GetAsyncKeyState(VK_INSERT) & 1) && alpha_animator.start_animation(!menu_open))
                menu_open ^= 1;

            if (alpha_animator.get_value() > 0.f) {
                ImGui::GetStyle().Alpha = alpha_animator.get_value();
            }

            SetWindowDisplayAffinity(hwnd, GlobalVars::streamproof == true ? WDA_EXCLUDEFROMCAPTURE : WDA_NONE);

            cheat_window.render();

            std::vector<std::string> configFiles;
            for (auto& file : std::filesystem::directory_iterator(configFolder)) {
                std::filesystem::path filePath = file;
                std::string extension = filePath.extension().string();

                if (extension == XorStr(".cfg")) {
                    if (!std::filesystem::is_directory(file.path())) {
                        auto path2 = file.path().string().substr(configFolder.length() + 1);
                        configFiles.push_back(path2.c_str());
                    }
                }
            }

            cfg_items = configFiles;

            SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_TOPMOST | (menu_open ? 0 : WS_EX_TRANSPARENT) | WS_EX_LAYERED | WS_EX_TOOLWINDOW);

            ImGui::Render();
            g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
            g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

            g_pSwapChain->Present(GlobalVars::vsync, 0);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(!GlobalVars::highcpuusageesp)));
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}

bool Overlay::CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED)
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}