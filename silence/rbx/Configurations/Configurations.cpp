#include "Configurations.hpp"
#include "../Headers/json.hpp"
#include "../Headers/XorStr.hpp"
#include "../rbx/globals/GlobalVars.hpp"

#include <iostream>
#include <fstream>

using json = nlohmann::json;

static void replace(std::string& str, const std::string& from, const std::string& to) {
    size_t startPosition = str.find(from);
    if (startPosition != std::string::npos)
        str.replace(startPosition, from.length(), to);
}

static void saveFile(const std::string& filePath, const std::string& data) {
    std::ofstream outFile(filePath);
    if (outFile.is_open()) {
        outFile << data;
        outFile.close();
    }
}

static void loadFile(const std::string& filePath, std::string& data) {
    std::ifstream inFile(filePath);
    if (inFile.is_open()) {
        std::getline(inFile, data, '\0');
        inFile.close();
    }
}

template<typename T>
static void loadConfigOption(T& Globalsetting, const json& configObject, const std::string& configSetting) {
    auto it = configObject.find(configSetting);
    if (it != configObject.end())
        Globalsetting = *it;
}

void configs::save(const char* name)
{
    json config;

    config["ESP"] = GlobalVars::esp;

    config["BOXES"] = GlobalVars::box_esp;
    config["BOXES_TYPE"] = GlobalVars::box_type;
    config["BOXES_OUTLINE"] = GlobalVars::box_outline;

    config["HEALTH_BAR"] = GlobalVars::health_bar;

    config["SKELETONS"] = GlobalVars::skeleton_esp;

    config["NAMES"] = GlobalVars::name_esp;
    config["NAMES_OUTLINE"] = GlobalVars::name_outline;

    config["DISTANCES"] = GlobalVars::distance_esp;
    config["DISTANCES_OUTLINE"] = GlobalVars::distance_outline;

    config["UNDERCIRCLES"] = GlobalVars::undercircle_esp;
    config["UNDERCIRCLES_FILLED"] = GlobalVars::undercircle_filled;
    config["UNDERCIRCLES_SEGMENTS"] = GlobalVars::undercircle_segments;
    config["UNDERCIRCLES_RADIUS"] = GlobalVars::undercircle_radius;

    config["TRACERS"] = GlobalVars::snapline_esp;

    config["LOOKTRACERS"] = GlobalVars::looktracer;

    config["AIM_VIEWER"] = GlobalVars::aimviewer;
    config["VISUALIZE_PREDICTION"] = GlobalVars::prediction_dot;

    config["FOV_DRAW"] = GlobalVars::drawFov;
    config["FOV_RADIUS"] = GlobalVars::fovRadius;

    config["AIMBOT"] = GlobalVars::aimbot;
    config["AIMBOT_BIND"] = GlobalVars::aimbot_bind.get_bound_key();
    config["AIMBOT_TYPE"] = GlobalVars::aimbot_type;
    config["AIMBOT_PART"] = GlobalVars::aimbot_part;
    config["AIMBOT_STICKY"] = GlobalVars::aimbot_sticky;

    config["SMOOTHNESS_MOUSE"] = GlobalVars::smoothness_mouse;
    config["SMOOTHNESS_CAMERA"] = GlobalVars::smoothness_camera;

    config["RESOLVER"] = GlobalVars::resolver;

    config["PREDICTION_CAMERA"] = GlobalVars::camera_prediction;
    config["PREDICTION_CAMERA_X"] = GlobalVars::camera_prediction_x;
    config["PREDICTION_CAMERA_Y"] = GlobalVars::camera_prediction_y;

    config["PREDICTION_MOUSE"] = GlobalVars::mouse_prediction;
    config["PREDICTION_MOUSE_X"] = GlobalVars::mouse_prediction_x;
    config["PREDICTION_MOUSE_Y"] = GlobalVars::mouse_prediction_y;

    config["SENSITIVITY_MOUSE"] = GlobalVars::mouse_sensitivity;

    config["SHAKE"] = GlobalVars::shake;
    config["SHAKE_X"] = GlobalVars::shake_x;
    config["SHAKE_Y"] = GlobalVars::shake_y;

    config["FIELDOFVIEW_CHANGE"] = GlobalVars::camera_fieldOfViewChange;
    config["FIELDOFVIEW_VALUE"] = GlobalVars::camera_fieldOfView;

    config["INFOV_ONLY"] = GlobalVars::in_fov_only;
    config["NOJUMP_COOLDOWN"] = GlobalVars::nojump_cooldown;

    config["WALKSPEED"] = GlobalVars::walkspeed;
    config["SPEED"] = GlobalVars::walkspeed_val;

    config["JUMPPOWER"] = GlobalVars::jumppower;
    config["POWER"] = GlobalVars::jumppower_val;

    config["HIPHEIGHT"] = GlobalVars::hipheight;
    config["HEIGHT"] = GlobalVars::hipheight_val;

    saveFile(get_appdata_path() + XorStr("\\silence\\") + XorStr("\\configs\\") + static_cast<std::string>(name) + XorStr(".cfg"), config.dump());

    return;
}

void configs::load(const char* name)
{
    std::string configJson = "";
    loadFile(get_appdata_path() + XorStr("\\silence\\") + XorStr("\\configs\\") + static_cast<std::string>(name), configJson);

    json config = json::parse(configJson);

    loadConfigOption<bool>(GlobalVars::esp, config, XorStr("ESP"));

    loadConfigOption<bool>(GlobalVars::box_esp, config, XorStr("BOXES"));
    loadConfigOption<int>(GlobalVars::box_type, config, XorStr("BOXES_TYPE"));
    loadConfigOption<bool>(GlobalVars::box_outline, config, XorStr("BOXES_OUTLINE"));

    loadConfigOption<bool>(GlobalVars::health_bar, config, XorStr("HEALTH_BAR"));

    loadConfigOption<bool>(GlobalVars::skeleton_esp, config, XorStr("SKELETONS"));

    loadConfigOption<bool>(GlobalVars::name_esp, config, XorStr("NAMES"));
    loadConfigOption<bool>(GlobalVars::name_outline, config, XorStr("NAMES_OUTLINE"));

    loadConfigOption<bool>(GlobalVars::distance_esp, config, XorStr("DISTANCES"));
    loadConfigOption<bool>(GlobalVars::distance_outline, config, XorStr("DISTANCES_OUTLINE"));

    loadConfigOption<bool>(GlobalVars::undercircle_esp, config, XorStr("UNDERCIRCLES"));
    loadConfigOption<bool>(GlobalVars::undercircle_filled, config, XorStr("UNDERCIRCLES_FILLED"));
    loadConfigOption<int>(GlobalVars::undercircle_segments, config, XorStr("UNDERCIRCLES_SEGMENTS"));
    loadConfigOption<float>(GlobalVars::undercircle_radius, config, XorStr("UNDERCIRCLES_RADIUS"));

    loadConfigOption<bool>(GlobalVars::snapline_esp, config, XorStr("TRACERS"));

    loadConfigOption<bool>(GlobalVars::looktracer, config, XorStr("LOOKTRACERS"));

    loadConfigOption<bool>(GlobalVars::aimviewer, config, XorStr("AIM_VIEWER"));
    loadConfigOption<bool>(GlobalVars::prediction_dot, config, XorStr("VISUALIZE_PREDICTION"));

    loadConfigOption<bool>(GlobalVars::drawFov, config, XorStr("FOV_DRAW"));
    loadConfigOption<int>(GlobalVars::fovRadius, config, XorStr("FOV_RADIUS"));

    loadConfigOption<bool>(GlobalVars::aimbot, config, XorStr("AIMBOT"));

    int key;
    loadConfigOption<int>(key, config, XorStr("AIMBOT_BIND"));
    loadConfigOption<int>(GlobalVars::aimbot_type, config, XorStr("AIMBOT_TYPE"));
    loadConfigOption<int>(GlobalVars::aimbot_part, config, XorStr("AIMBOT_PART"));
    loadConfigOption<bool>(GlobalVars::aimbot_sticky, config, XorStr("AIMBOT_STICKY"));

    loadConfigOption<float>(GlobalVars::smoothness_mouse, config, XorStr("SMOOTHNESS_MOUSE"));
    loadConfigOption<float>(GlobalVars::smoothness_camera, config, XorStr("SMOOTHNESS_CAMERA"));

    loadConfigOption<bool>(GlobalVars::resolver, config, XorStr("RESOLVER"));

    loadConfigOption<bool>(GlobalVars::camera_prediction, config, XorStr("PREDICTION_CAMERA"));
    loadConfigOption<float>(GlobalVars::camera_prediction_x, config, XorStr("PREDICTION_CAMERA_X"));
    loadConfigOption<float>(GlobalVars::camera_prediction_y, config, XorStr("PREDICTION_CAMERA_Y"));

    loadConfigOption<bool>(GlobalVars::mouse_prediction, config, XorStr("PREDICTION_MOUSE"));
    loadConfigOption<float>(GlobalVars::mouse_prediction_x, config, XorStr("PREDICTION_MOUSE_X"));
    loadConfigOption<float>(GlobalVars::mouse_prediction_y, config, XorStr("PREDICTION_MOUSE_Y"));

    loadConfigOption<float>(GlobalVars::mouse_sensitivity, config, XorStr("SENSITIVITY_MOUSE"));

    loadConfigOption<bool>(GlobalVars::shake, config, XorStr("SHAKE"));
    loadConfigOption<float>(GlobalVars::shake_x, config, XorStr("SHAKE_X"));
    loadConfigOption<float>(GlobalVars::shake_y, config, XorStr("SHAKE_Y"));

    loadConfigOption<bool>(GlobalVars::in_fov_only, config, XorStr("INFOV_ONLY"));
    loadConfigOption<bool>(GlobalVars::nojump_cooldown, config, XorStr("NOJUMP_COOLDOWN"));

    loadConfigOption<bool>(GlobalVars::walkspeed, config, XorStr("WALKSPEED"));
    loadConfigOption<float>(GlobalVars::walkspeed_val, config, XorStr("SPEED"));

    loadConfigOption<bool>(GlobalVars::jumppower, config, XorStr("JUMPPOWER"));
    loadConfigOption<float>(GlobalVars::jumppower_val, config, XorStr("POWER"));

    loadConfigOption<bool>(GlobalVars::hipheight, config, XorStr("HIPHEIGHT"));
    loadConfigOption<float>(GlobalVars::hipheight_val, config, XorStr("HEIGHT"));

    GlobalVars::aimbot_bind.set_key(key);

    return;
}