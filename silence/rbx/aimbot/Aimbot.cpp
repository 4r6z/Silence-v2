#include <Windows.h>
#include <thread>
#include <random>
#include <vector>
#include <immintrin.h>
#include <cmath>

#include "../esp/ESP.hpp"
#include "Aimbot.hpp"
#include "../globals/GlobalVars.hpp"
#include "../Includes/imgui/imgui.h"
#include "../Headers/XorStr.hpp"
#include "../class/Classes.hpp"

#define M_PI           3.14159265358979323846

// FRAME SKIP
//static int min_reaction_time_ms = 100;
//static int max_reaction_time_ms = 300;
//
//static int GenerateReactionTime() {
//    std::random_device rd;
//    std::mt19937 gen(rd());
//    std::uniform_int_distribution<> dis(min_reaction_time_ms, max_reaction_time_ms);
//    return dis(gen);
//}


// BETTER AIMBOT
//static float deceptive_targeting_chance = 0.2f;
//static float deceptive_offset_factor = 0.05f;
//
//static Roblox::Vector3 ApplyDeceptiveTargeting(const Roblox::Vector3& target_position) {
//    std::random_device rd;
//    std::mt19937 gen(rd());
//    std::uniform_real_distribution<float> chance_dis(0.0f, 1.0f);
//    std::uniform_real_distribution<float> offset_dis(-deceptive_offset_factor, deceptive_offset_factor);
//
//    if (chance_dis(gen) <= deceptive_targeting_chance) {
//        return {
//            target_position.x + offset_dis(gen),
//            target_position.y + offset_dis(gen),
//            target_position.z + offset_dis(gen)
//        };
//    }

//    // Return the original target position if not applying deceptive targeting
//    return target_position;
//}

static float Ease(float t) {
    switch (GlobalVars::aimbot_easing_style) {
    case 0: // linear
        return t;
    case 1: // sine
        return 1 - std::cos((t * M_PI) / 2);
    case 2: // quad
        return t * t;
    case 3: // cubic
        return t * t * t;
    case 4: // quart
        return t * t * t * t;
    case 5: // quint
        return t * t * t * t * t;
    case 6: // exponential
        return t == 0 ? 0 : std::pow(2, 10 * (t - 1));
    case 7: // circular
        return 1 - std::sqrt(1 - std::pow(t, 2));
    case 8: // back
        return t * t * (2.70158f * t - 1.70158f);
    case 9: // bounce
        if (t < 1 / 2.75f)
            return 7.5625f * t * t;
        else if (t < 2 / 2.75f)
            return 7.5625f * (t -= 1.5f / 2.75f) * t + 0.75f;
        else if (t < 2.5f / 2.75f)
            return 7.5625f * (t -= 2.25f / 2.75f) * t + 0.9375f;
        else
            return 7.5625f * (t -= 2.625f / 2.75f) * t + 0.984375f;
    case 10: // elastic
        return t == 0 ? 0 : t == 1 ? 1 : -std::pow(2, 10 * (t - 1)) * std::sin((t - 1.1f) * 5 * M_PI);
    default:
        return t;
    }
}

static Roblox::Vector3 Recalculate_Velocity(Roblox::Entity player)
{
    Roblox::Vector3 old_Position = player.rootPart.getPartPosition();
    //std::this_thread::sleep_for(std::chrono::milliseconds(85));
    return (player.rootPart.getPartPosition() - old_Position) / 0.085;
}

Roblox::Instance FindPartByName(Roblox::Instance& character, const std::string& partName) {
    Roblox::Instance part = character.findFirstChild(partName, true);
    return part;
}

static float sigmoid(float x) {
    return 1 / (1 + std::exp(-x));
}

static Roblox::Vector3 Random_Vector3(const float x, const float y) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis_x(-x, x);
    std::uniform_real_distribution<float> dis_y(-y, y);

    return { dis_x(gen) , dis_y(gen) , dis_x(gen) };
}


static Roblox::Matrix3x3 Lerp_Matrix3(const Roblox::Matrix3x3& a, const Roblox::Matrix3x3& b, float t) {
    t = Ease(t);
    if (t == 1) return b;

    Roblox::Matrix3x3 result{};
    for (int i = 0; i < 9; ++i) {
        result.data[i] = a.data[i] + (b.data[i] - a.data[i]) * t;
    }
    return result;
}

static Roblox::Vector3 Cross_Product(const Roblox::Vector3& vec1, const Roblox::Vector3& vec2) {
    return {
        vec1.y * vec2.z - vec1.z * vec2.y,
        vec1.z * vec2.x - vec1.x * vec2.z,
        vec1.x * vec2.y - vec1.y * vec2.x
    };
}

static Roblox::Matrix3x3 Look_At_To_Matrix(const Roblox::Vector3& cameraPosition, const Roblox::Vector3& targetPosition) {
    Roblox::Vector3 forward = Roblox::Vector3{ targetPosition.x - cameraPosition.x, targetPosition.y - cameraPosition.y, targetPosition.z - cameraPosition.z }.normalize();
    Roblox::Vector3 right = Cross_Product({ 0, 1, 0 }, forward).normalize();
    Roblox::Vector3 up = Cross_Product(forward, right);

    Roblox::Matrix3x3 lookAtMatrix{};
    lookAtMatrix.data[0] = -right.x;  lookAtMatrix.data[1] = up.x;  lookAtMatrix.data[2] = -forward.x;
    lookAtMatrix.data[3] = right.y;  lookAtMatrix.data[4] = up.y;  lookAtMatrix.data[5] = -forward.y;
    lookAtMatrix.data[6] = -right.z;  lookAtMatrix.data[7] = up.z;  lookAtMatrix.data[8] = -forward.z;

    return lookAtMatrix;
}

void InitializePlayerParts(Roblox::Entity& player) {
    std::vector<std::string> partNames = {
        "Head", "HumanoidRootPart", "UpperTorso", "LowerTorso",
        "LeftUpperLeg", "LeftUpperArm",
        "LeftHand", "RightUpperArm", "RightHand"
    };

    std::vector<Roblox::Instance> parts;

    for (const std::string& partName : partNames) {
        parts.push_back(FindPartByName(player.character, partName));
    }

    player.head = parts[0];
    player.rootPart = parts[1];
    player.upperTorso = parts[2];
    player.lowerTorso = parts[3];
    player.leftUpperLeg = parts[4];
    player.leftUpperArm = parts[5];
    player.leftHand = parts[6];
    player.rightUpperArm = parts[7];
    player.rightHand = parts[8];
}

Roblox::Instance getClosestPart(Roblox::Entity& player, const POINT& cursor_point) {
    std::vector<Roblox::Instance> parts = {
        player.head,
        player.rootPart,
        player.upperTorso,
        player.lowerTorso,
        player.leftUpperLeg,
        player.leftFoot,
        player.rightFoot,
        player.leftUpperArm,
        player.leftHand,
        player.rightUpperArm,
        player.rightHand,
    };

    Roblox::Vector2 dimensions = GlobalVars::visualengine.getScreenDimensions();
    Roblox::Matrix4x4 view_matrix = GlobalVars::visualengine.getViewMatrix();
    Roblox::Vector2 cursor = { static_cast<float>(cursor_point.x), static_cast<float>(cursor_point.y) };
    float min_distance = FLT_MAX;
    Roblox::Instance closest_part;

    for (size_t i = 0; i < parts.size(); ++i) {
        if (!parts[i].address) {
            continue;
        }

        Roblox::Vector3 part_position = parts[i].getPartPosition();
        Roblox::Vector2 part_screen_position = Roblox::worldToScreen(part_position, dimensions, view_matrix);
        float distance = (part_screen_position - cursor).getMagnitude();

        if (distance < min_distance) {
            min_distance = distance;
            closest_part = parts[i];
        }
    }

    return closest_part;
}

static Roblox::Entity getClosestPlayerFromCursor()
{
    POINT cursor_point;
    GetCursorPos(&cursor_point);
    ScreenToClient(FindWindowA("WINDOWSCLIENT", NULL), &cursor_point);

    Roblox::Vector2 cursor =
    {
        static_cast<float>(cursor_point.x),
        static_cast<float>(cursor_point.y)
    };

    std::vector<Roblox::Entity>& cached_players = GlobalVars::cached_players;

    Roblox::Entity closestPlayer{};
    int shortestDistance = 9e9;

    Roblox::Entity localPlayer = GlobalVars::localplayer;
    Roblox::Instance localPlayerTeam = localPlayer.team;

    Roblox::Vector2 dimensions = GlobalVars::visualengine.getScreenDimensions();
    Roblox::Matrix4x4 viewmatrix = GlobalVars::visualengine.getViewMatrix();

    for (Roblox::Entity& player : cached_players)
    {
        if (player.address == localPlayer.address || !player.character.address || !player.humanoid.address)
            continue;

        if (g_whitelisted_players.size() > 0)
        {

            auto ite = std::find(g_whitelisted_players.begin(), g_whitelisted_players.end(), player.name);
            auto is_whitelisted = ite != g_whitelisted_players.end();

            if (!is_whitelisted)
                continue;

        }

        bool knockedCheck = GlobalVars::aimbot_checks[0];
        bool grabbedCheck = GlobalVars::aimbot_checks[1];
        bool teamCheck = GlobalVars::aimbot_checks[2];

        if (knockedCheck && player.knockedOut.getBoolFromValue())
            continue;

        if (grabbedCheck && player.bodyEffects.findFirstChild(XorStr("GRABBING_CONSTRAINT"), true).address != 0)
            continue;

        if (teamCheck && player.team.address == localPlayerTeam.address)
            continue;

        Roblox::Instance part = player.rootPart;

        Roblox::Vector3 partPosition = part.getPartPosition();
        Roblox::Vector2 partPositionOnScreen = Roblox::worldToScreen(partPosition, dimensions, viewmatrix);

        float distance_from_cursor = (partPositionOnScreen - cursor).getMagnitude();

        if (shortestDistance > distance_from_cursor)
        {
            closestPlayer = player;
            shortestDistance = distance_from_cursor;
        }
    }

    return closestPlayer;
}

static bool isWithinFOV(const Roblox::Vector3& hit_position_3D) {
    POINT cursor_point;
    GetCursorPos(&cursor_point);
    ScreenToClient(FindWindowA("WINDOWSCLIENT", NULL), &cursor_point);

    auto cursor_pos_x = cursor_point.x;
    auto cursor_pos_y = cursor_point.y;

    Roblox::Instance visualengine = GlobalVars::visualengine;
    Roblox::Vector2 screen_dimensions = visualengine.getScreenDimensions();
    Roblox::Vector2 hit_position_2D = Roblox::worldToScreen(hit_position_3D, screen_dimensions, visualengine.getViewMatrix());

    float magnitude = (hit_position_2D - Roblox::Vector2{ static_cast<float>(cursor_pos_x), static_cast<float>(cursor_pos_y) }).getMagnitude();

    return (magnitude <= GlobalVars::fovRadius);
}

static void run(Roblox::Entity player, Roblox::Vector3 resolvedVelocity, POINT cursor_point)
{
    Roblox::Vector3 hit_position_3D{};

    Roblox::Instance character = player.character;
    Roblox::Instance hitbox{};
    Roblox::Entity localPlayer = GlobalVars::localplayer;

    bool knockedCheck = GlobalVars::aimbot_checks[0];
    bool grabbedCheck = GlobalVars::aimbot_checks[1];
    bool teamCheck = GlobalVars::aimbot_checks[2];

    if (knockedCheck == true && player.knockedOut.getBoolFromValue() == true)
        return;

    if (grabbedCheck == true && player.character.findFirstChild(XorStr("GRABBING_CONSTRAINT"), true).address != 0)
        return;

    if (teamCheck == true && player.team.address == localPlayer.team.address)
        return;

    if (GlobalVars::closest_part) {
        hitbox = getClosestPart(player, cursor_point);
    }
    else {
        switch (GlobalVars::aimbot_part) {
        case 0: hitbox = player.head; break;
        case 1: hitbox = player.rootPart; break;
        case 2: hitbox = player.upperTorso; break;
        case 3: hitbox = player.lowerTorso; break;
        case 4: hitbox = player.leftHand; break;
        case 5: hitbox = player.rightHand; break;
        case 6: hitbox = player.leftUpperArm; break;
        case 7: hitbox = player.rightUpperArm; break;
        case 8: hitbox = player.leftUpperLeg; break;
        case 9: hitbox = player.rightUpperLeg; break;
        case 10: hitbox = player.leftFoot; break;
        case 11: hitbox = player.rightFoot; break;
        }
    }

    hit_position_3D = hitbox.getPartPosition();

    if (GlobalVars::aimbot_type == 0) { // mouse lock
        POINT cursor_point;
        GetCursorPos(&cursor_point);
        ScreenToClient(FindWindowA("WINDOWSCLIENT", NULL), &cursor_point);

        auto cursor_pos_x = cursor_point.x;
        auto cursor_pos_y = cursor_point.y;

        Roblox::Instance visualengine = GlobalVars::visualengine;
        Roblox::Vector2 dimensions = visualengine.getScreenDimensions();
        Roblox::Matrix4x4 view_matrix = visualengine.getViewMatrix();

        if (GlobalVars::mouse_prediction) {
            Roblox::Vector3 velocity_vec;
            if (GlobalVars::prediction_type == 0) {
                velocity_vec = (GlobalVars::resolver ? resolvedVelocity : hitbox.getPartVelocity()) / Roblox::Vector3{ GlobalVars::mouse_prediction_x, GlobalVars::mouse_prediction_y, GlobalVars::mouse_prediction_x };
            }
            else {
                velocity_vec = (GlobalVars::resolver ? resolvedVelocity : hitbox.getPartVelocity()) * Roblox::Vector3{ GlobalVars::mouse_prediction_x, GlobalVars::mouse_prediction_y, GlobalVars::mouse_prediction_x };
            }
            hit_position_3D = (hitbox.getPartPosition() + velocity_vec) + (GlobalVars::shake == true ? Random_Vector3(GlobalVars::shake_x, GlobalVars::shake_y) : Roblox::Vector3{});
        }
        else {
            hit_position_3D = hitbox.getPartPosition() + (GlobalVars::shake == true ? Random_Vector3(GlobalVars::shake_x, GlobalVars::shake_y) : Roblox::Vector3{});
        }

        Roblox::Vector2 hit_position_2D = Roblox::worldToScreen(hit_position_3D, dimensions, view_matrix);

        if (hit_position_2D.x == -1)
            return;

        float magnitude = (hit_position_2D - Roblox::Vector2{ static_cast<float>(cursor_pos_x), static_cast<float>(cursor_pos_y) }).getMagnitude();

        if (GlobalVars::in_fov_only && magnitude > GlobalVars::fovRadius)
            return;

        Roblox::Vector2 relative_2D = { 0, 0 };

        float sensitivity = GlobalVars::mouse_sensitivity;
        float smoothness_mouse = GlobalVars::smoothness_mouse;

        relative_2D.x = (hit_position_2D.x - cursor_pos_x) * sensitivity / smoothness_mouse;
        relative_2D.y = (hit_position_2D.y - cursor_pos_y) * sensitivity / smoothness_mouse;

        if (relative_2D.x == -1 || relative_2D.y == -1)
            return;

        INPUT input{};
        input.mi.time = 0;
        input.type = INPUT_MOUSE;
        input.mi.mouseData = 0;
        input.mi.dx = relative_2D.x;
        input.mi.dy = relative_2D.y;
        input.mi.dwFlags = MOUSEEVENTF_MOVE;
        SendInput(1, &input, sizeof(input));
    }
    else if (GlobalVars::aimbot_type == 1) { // camera aimbot
        if (GlobalVars::camera_prediction) {
            Roblox::Vector3 velocity_vec = (GlobalVars::resolver ? resolvedVelocity : hitbox.getPartVelocity());
            if (GlobalVars::prediction_type == 0) {
                velocity_vec = velocity_vec / Roblox::Vector3{ GlobalVars::camera_prediction_x, GlobalVars::camera_prediction_y, GlobalVars::camera_prediction_x };
            }
            else {
                velocity_vec = velocity_vec * Roblox::Vector3{ GlobalVars::camera_prediction_x, GlobalVars::camera_prediction_y, GlobalVars::camera_prediction_x };
            }
            hit_position_3D = hitbox.getPartPosition() + velocity_vec + (GlobalVars::shake ? Random_Vector3(GlobalVars::shake_x, GlobalVars::shake_y) : Roblox::Vector3{});
        }
        else {
            hit_position_3D = hitbox.getPartPosition() + (GlobalVars::shake ? Random_Vector3(GlobalVars::shake_x, GlobalVars::shake_y) : Roblox::Vector3{});
        }

        if (GlobalVars::in_fov_only && !isWithinFOV(hit_position_3D)) {
            return;
        }

        float smoothness_camera = (100.1f - GlobalVars::smoothness_camera) / 100.0f;

        Roblox::Instance camera = GlobalVars::workspace.getCamera();
        Roblox::Matrix3x3 hit_matrix = Look_At_To_Matrix(camera.getCameraPosition(), hit_position_3D);
        Roblox::Matrix3x3 relative_matrix = Lerp_Matrix3(camera.getCameraRotation(), hit_matrix, smoothness_camera);

        camera.writeCameraRotation(relative_matrix);
    }
    else if (GlobalVars::aimbot_type == 2) { // free aim
        POINT cursor_point;
        GetCursorPos(&cursor_point);
        ScreenToClient(FindWindowA("WINDOWSCLIENT", NULL), &cursor_point);

        auto cursor_pos_x = cursor_point.x;
        auto cursor_pos_y = cursor_point.y;

        Roblox::Instance visualengine = GlobalVars::visualengine;
        Roblox::Vector2 dimensions = visualengine.getScreenDimensions();
        Roblox::Matrix4x4 view_matrix = visualengine.getViewMatrix();

        if (GlobalVars::free_aim_prediction) {
            Roblox::Vector3 velocity_vec;
            if (GlobalVars::prediction_type == 0) {
                velocity_vec = (GlobalVars::resolver ? resolvedVelocity : hitbox.getPartVelocity()) / Roblox::Vector3{ GlobalVars::free_aim_prediction_x, GlobalVars::free_aim_prediction_y, GlobalVars::free_aim_prediction_x };
            }
            else {
                velocity_vec = (GlobalVars::resolver ? resolvedVelocity : hitbox.getPartVelocity()) * Roblox::Vector3{ GlobalVars::free_aim_prediction_x, GlobalVars::free_aim_prediction_y, GlobalVars::free_aim_prediction_x };
            }
            hit_position_3D = (hitbox.getPartPosition() + velocity_vec) + (GlobalVars::shake ? Random_Vector3(GlobalVars::shake_x, GlobalVars::shake_y) : Roblox::Vector3{});
        }
        else {
            hit_position_3D = hitbox.getPartPosition() + (GlobalVars::shake ? Random_Vector3(GlobalVars::shake_x, GlobalVars::shake_y) : Roblox::Vector3{});
        }

        Roblox::Vector2 hit_position_2D = Roblox::worldToScreen(hit_position_3D, dimensions, view_matrix);

        if (hit_position_2D.x == -1 || hit_position_2D.y == -1)
            return;

        float magnitude = (hit_position_2D - Roblox::Vector2{ static_cast<float>(cursor_pos_x), static_cast<float>(cursor_pos_y) }).getMagnitude();

        if (GlobalVars::in_fov_only && magnitude > GlobalVars::fovRadius)
            return;

        Roblox::Vector2 target_position = hit_position_2D;
        Roblox::Vector2 current_position = { static_cast<float>(cursor_pos_x), static_cast<float>(cursor_pos_y) };

        Roblox::Vector2 move_delta = target_position - current_position;

        float sensitivity = GlobalVars::free_aim_sensitivity;
        float smoothness_free_aim = GlobalVars::smoothness_free_aim;

        Roblox::Instance mouse_service = GlobalVars::game.findFirstChild(XorStr("MouseService"), false);
        std::uint64_t mouse_service_address = mouse_service.address;
        Roblox::Instance::initializeMouseService(mouse_service_address);
        Roblox::Instance::writeMousePosition(target_position.x, target_position.y);
    }

    return;
}

void HideCursor() {
    ShowCursor(FALSE);
}

void ShowCursor() {
    ShowCursor(TRUE);
}

void Roblox::hook_aimbot() {
    Roblox::Entity savedPlayer{};
    bool isAimboting = false;
    Roblox::Vector3 calculatedVelocity{};
    Roblox::Entity currentPlayer;
    Roblox::Entity localplayer = GlobalVars::localplayer;
    Roblox::Instance localplayerHead = localplayer.head;

    HWND rblx = FindWindowA("WINDOWSCLIENT", NULL);

    HCURSOR invisibleCursor = LoadCursor(NULL, IDC_ARROW);

    while (true) {
        POINT cursor_point;
        GetCursorPos(&cursor_point);

        HWND current_rblx = FindWindowA("WINDOWSCLIENT", NULL);
        if (current_rblx != NULL) {
            rblx = current_rblx;
        }

        if (GetForegroundWindow() != rblx || rblx == NULL) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        GlobalVars::aimbot_bind.update_state();

        if (!GlobalVars::aimbot_bind.get_enabled() || !GlobalVars::aimbot) {
            isAimboting = false;
            ShowCursor();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        currentPlayer = (GlobalVars::aimbot_sticky && isAimboting && savedPlayer.address != 0) ? savedPlayer : getClosestPlayerFromCursor();

        if (currentPlayer.address == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            isAimboting = false;
            ShowCursor();
            continue;
        }

        if (GlobalVars::smoothness_camera > 3 || GlobalVars::aimbot_type == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        if (GlobalVars::closest_part) {
            InitializePlayerParts(currentPlayer);
        }

        if (!isAimboting) {
            SetCursor(invisibleCursor);
            HideCursor();
        }

        run(currentPlayer, calculatedVelocity, cursor_point);
        savedPlayer = currentPlayer;
        isAimboting = true;

        if (GlobalVars::resolver) {
            std::thread([&calculatedVelocity, &currentPlayer]() {
                calculatedVelocity = Recalculate_Velocity(currentPlayer);
                }).detach();
        }
    }
}