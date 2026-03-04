#pragma once

#include "../rbx/class/Classes.hpp"
#include "../framework/framework.h"

#ifndef GLOBALS_H
#define GLOBALS_H

extern float g_Time;
extern float g_DeltaTime;

#endif // GLOBALS_H

struct GlobalVars
{
    static Roblox::Instance game;
    static Roblox::Instance visualengine;
    static Roblox::Instance players;
    static Roblox::Instance workspace;
    static Roblox::Instance mouse_service;

    static bool threadcrash;
    static bool highcpuusageesp;
    static int mostFreq;

    static Roblox::Entity localplayer;
    static std::vector<Roblox::Entity> cached_players;

    static bool esp;

    static float max_render_distance;

    static bool chams;
    static c_color chams_color;

    static bool aimviewer;
    static c_color aimviewer_color;

    static bool box_esp;
    static bool box_filled;
    static int box_type;
    static bool box_outline;
    static c_color box_color;
    static c_color filled_box_color;

    static bool health_bar;
    static bool shield_bar;

    static bool skeleton_esp;
    static bool skeleton_outline;
    static c_color skeleton_color;

    static bool name_esp;
    static bool name_outline;
    static c_color name_color;

    static bool distance_esp;
    static bool distance_outline;
    static c_color distance_color;

    static bool undercircle_esp;
    static bool undercircle_filled;
    static int undercircle_segments;
    static float undercircle_radius;
    static c_color undercircle_color;

    static bool snapline_esp;
    static int snapline_position;
    static c_color snapline_color;

    static bool looktracer;
    static c_color looktracer_color;

    static bool prediction_dot;
    static c_color prediction_dot_color;

    static bool aimbot;
    static c_binding aimbot_bind;
    static c_binding cfly_bind;
    static c_binding cframe_bind;
    static int aimbot_type;
    static int aimbot_part;
    static int aimbot_easing_style;
    static int selected_ignore_part;
    static bool aimbot_sticky;
    static bool closest_part;

    static float smoothness_mouse;
    static float smoothness_free_aim;
    static float smoothness_camera;

    static bool resolver;
    static std::vector<BYTE> aimbot_checks;
    static std::vector<BYTE> esp_checks;

    static bool prediction;
    static float prediction_x;
    static float prediction_y;

    static bool camera_prediction;
    static float camera_prediction_x;
    static float camera_prediction_y;
    static int prediction_type;

    static bool camera_fieldOfViewChange;
    static float camera_fieldOfView;
    static float camera_fieldOfViewScaleFactor;

    static bool mouse_prediction;
    static float mouse_prediction_x;
    static float mouse_prediction_y;

    static bool free_aim_prediction;
    static float free_aim_prediction_x;
    static float free_aim_prediction_y;

    static float free_aim_sensitivity;
    static float mouse_sensitivity;

    static bool deadzone;
    static int deadzone_value;

    static bool shake;
    static float shake_x;
    static float shake_y;

    static bool drawFov;
    static bool in_fov_only;
    static int fovRadius;
    static c_color fov_color;

    static bool nojump_cooldown;

    static bool walkspeed;
    static float walkspeed_val;

    static bool jumppower;
    static float jumppower_val;

    static bool hipheight;
    static float original_hipheight;
    static float hipheight_val;

    static bool vsync;
    static bool streamproof;

    static bool cframe_walk;
    static float cframe_speed;

    static bool cfly_enabled;
    static float fly_factor;

    static bool triggerbot;
    static float triggerbot_fov_radius;

};