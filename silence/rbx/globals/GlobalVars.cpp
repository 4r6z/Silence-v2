#include "GlobalVars.hpp"
#include "../Headers/XorStr.hpp"

Roblox::Instance GlobalVars::game{};
Roblox::Instance GlobalVars::visualengine{};
Roblox::Instance GlobalVars::players{};
Roblox::Instance GlobalVars::workspace{};
Roblox::Instance GlobalVars::mouse_service{};

bool GlobalVars::threadcrash = false;
bool GlobalVars::highcpuusageesp = false;
int GlobalVars::mostFreq = 0;
float g_Time = 0.0f;
float g_DeltaTime = 0.0f;

Roblox::Entity GlobalVars::localplayer{};
std::vector<Roblox::Entity> GlobalVars::cached_players{};

float GlobalVars::max_render_distance = 1000.f;

bool GlobalVars::esp = false;

bool GlobalVars::chams = true;
c_color GlobalVars::chams_color{};

bool GlobalVars::aimviewer = false;
c_color GlobalVars::aimviewer_color{};

bool GlobalVars::box_esp = false;
bool GlobalVars::box_filled = false;
int GlobalVars::box_type = 0;
bool GlobalVars::box_outline = false;
c_color GlobalVars::box_color{};
c_color GlobalVars::filled_box_color{};

bool GlobalVars::health_bar = false;
bool GlobalVars::shield_bar = false;

bool GlobalVars::skeleton_esp = false;
bool GlobalVars::skeleton_outline = false;
c_color GlobalVars::skeleton_color{};

bool GlobalVars::name_esp = false;
bool GlobalVars::name_outline = true;
c_color GlobalVars::name_color{};

bool GlobalVars::distance_esp = false;
bool GlobalVars::distance_outline = true;
c_color GlobalVars::distance_color{};

bool GlobalVars::undercircle_esp = false;
bool GlobalVars::undercircle_filled = false;
int GlobalVars::undercircle_segments = 16;
float GlobalVars::undercircle_radius = 3.14f;
c_color GlobalVars::undercircle_color{};

bool GlobalVars::snapline_esp = false;
int GlobalVars::snapline_position = 2;
c_color GlobalVars::snapline_color{};

bool GlobalVars::looktracer = false;
c_color GlobalVars::looktracer_color{};

bool GlobalVars::prediction_dot = false;
c_color GlobalVars::prediction_dot_color{};

bool GlobalVars::aimbot = false;
int GlobalVars::aimbot_type = 1;
int GlobalVars::aimbot_part = 1;
int GlobalVars::aimbot_easing_style = 0;
bool GlobalVars::closest_part = false;
bool GlobalVars::aimbot_sticky = false;

float GlobalVars::smoothness_mouse = 1.f;
float GlobalVars::smoothness_free_aim = 1.f;
float GlobalVars::smoothness_camera = 0.1f;

bool GlobalVars::resolver = false;
std::vector<BYTE> GlobalVars::aimbot_checks{};
std::vector<BYTE> GlobalVars::esp_checks{};

bool GlobalVars::camera_prediction = true;
float GlobalVars::camera_prediction_x = 7.05f;
float GlobalVars::camera_prediction_y = 7.45f;
int GlobalVars::prediction_type = 0;

bool GlobalVars::camera_fieldOfViewChange = false;
float GlobalVars::camera_fieldOfView = 0;
float GlobalVars::camera_fieldOfViewScaleFactor = 180 / (std::atan(1.0) * 4);

bool GlobalVars::mouse_prediction = true;
float GlobalVars::mouse_prediction_x = 6.0f;
float GlobalVars::mouse_prediction_y = 6.0f;


bool GlobalVars::free_aim_prediction = true;
float GlobalVars::free_aim_prediction_x = 7.5f;
float GlobalVars::free_aim_prediction_y = 8.25f;

float GlobalVars::free_aim_sensitivity = 3.0f;
float GlobalVars::mouse_sensitivity = 3.0f;

bool GlobalVars::deadzone = false;
int GlobalVars::deadzone_value = 50;

bool GlobalVars::shake = false;
float GlobalVars::shake_x = 0.15;
float GlobalVars::shake_y = 0.15;

bool GlobalVars::drawFov = false;
int GlobalVars::fovRadius = 40;

bool GlobalVars::in_fov_only = false;
c_color GlobalVars::fov_color{};
c_binding GlobalVars::aimbot_bind{ XorStr("aimbot") };
c_binding GlobalVars::cframe_bind{ XorStr("cframe") };
c_binding GlobalVars::cfly_bind{ XorStr("cfly") };

bool GlobalVars::nojump_cooldown = true;

bool GlobalVars::walkspeed = false;
float GlobalVars::walkspeed_val = 16;

bool GlobalVars::jumppower = false;
float GlobalVars::jumppower_val = 50;

bool GlobalVars::hipheight = false;
float GlobalVars::hipheight_val = 2;
float GlobalVars::original_hipheight = 2;

bool GlobalVars::vsync = false;
bool GlobalVars::streamproof = false;

bool GlobalVars::cframe_walk = false;
float GlobalVars::cframe_speed = 0.5f;

bool GlobalVars::cfly_enabled = false;
float GlobalVars::fly_factor = 1.5f;

bool GlobalVars::triggerbot = false;
float GlobalVars::triggerbot_fov_radius = 20.f;