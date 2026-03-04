#include <Windows.h>
#include <thread>
#include <vector>
#include <algorithm>
#include <cmath>
#include "../rbx/globals/GlobalVars.hpp"
#include "../rbx/triggerbot/Triggerbot.hpp"
#include "../Headers/XorStr.hpp"
#include "../rbx/class/Classes.hpp"
#include "../Includes/imgui/imgui.h"

static bool isWithinTriggerbotFOV(const Roblox::Vector3& hit_position_3D) {
    POINT cursor_point;
    GetCursorPos(&cursor_point);
    ScreenToClient(FindWindowA("WINDOWSCLIENT", NULL), &cursor_point);

    auto cursor_pos_x = cursor_point.x;
    auto cursor_pos_y = cursor_point.y;

    Roblox::Instance visualengine = GlobalVars::visualengine;
    Roblox::Vector2 screen_dimensions = visualengine.getScreenDimensions();
    Roblox::Vector2 hit_position_2D = Roblox::worldToScreen(hit_position_3D, screen_dimensions, visualengine.getViewMatrix());

    float magnitude = (hit_position_2D - Roblox::Vector2{ static_cast<float>(cursor_pos_x), static_cast<float>(cursor_pos_y) }).getMagnitude();

    return (magnitude <= GlobalVars::triggerbot_fov_radius);
}

static void shoot() {
    if (!GlobalVars::triggerbot) return;

    INPUT input{};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    SendInput(1, &input, sizeof(input));

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, &input, sizeof(input));
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
    float shortestDistance = FLT_MAX;

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

        Roblox::Vector3 predictedPosition = partPosition;
        if (GlobalVars::mouse_prediction) {
            Roblox::Vector3 velocity_vec;
            if (GlobalVars::prediction_type == 0) {
                velocity_vec = part.getPartVelocity() /
                    Roblox::Vector3{ GlobalVars::camera_prediction_x, GlobalVars::camera_prediction_y, GlobalVars::camera_prediction_x };
            }
            else {
                velocity_vec = part.getPartVelocity() *
                    Roblox::Vector3{ GlobalVars::camera_prediction_x, GlobalVars::camera_prediction_y, GlobalVars::camera_prediction_x };
            }
            predictedPosition = partPosition + velocity_vec;
        }
        else {
            predictedPosition = partPosition;
        }

        Roblox::Vector2 partPositionOnScreen = Roblox::worldToScreen(predictedPosition, dimensions, viewmatrix);

        float distance_from_cursor = (partPositionOnScreen - cursor).getMagnitude();

        if (shortestDistance > distance_from_cursor)
        {
            closestPlayer = player;
            shortestDistance = distance_from_cursor;
        }
    }

    return closestPlayer;
}

void Roblox::hook_triggerbot() {
    HWND rblx = FindWindowA("WINDOWSCLIENT", NULL);

    while (true) {
        HWND current_rblx = FindWindowA("WINDOWSCLIENT", NULL);
        if (current_rblx != NULL) {
            rblx = current_rblx;
        }

        if (GetForegroundWindow() != rblx || rblx == NULL || !GlobalVars::triggerbot) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        POINT cursor_point;
        GetCursorPos(&cursor_point);

        if (GlobalVars::drawFov) {
            ImDrawList* draw = ImGui::GetBackgroundDrawList();
            ScreenToClient(rblx, &cursor_point);
            draw->AddCircle(ImVec2(cursor_point.x, cursor_point.y), GlobalVars::triggerbot_fov_radius, GlobalVars::fov_color.u32(), 32, 1.0f);
        }

        Roblox::Entity closestPlayer = getClosestPlayerFromCursor();

        if (closestPlayer.address != 0) {
            Roblox::Vector3 hit_position_3D = closestPlayer.head.getPartPosition();
            if (isWithinTriggerbotFOV(hit_position_3D)) {
                shoot();
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
