#include "ESP.hpp"
#include "../globals/GlobalVars.hpp"
#include "../Headers/XorStr.hpp"

#include <Windows.h>
#include <iomanip>
#include <sstream>
#include <thread>
#include <iostream>
#include <algorithm>
#include "../Includes/imgui/imgui.h"
#include "../driver/DriverImplementation.hpp"
#include "../Offsets.hpp"

inline static ImFont* verdana_12{ NULL };

#define M_PI           3.14159265358979323846

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

static float dotProduct(const Roblox::Vector3& vec1, const Roblox::Vector3& vec2) {
    return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z;
}

void text_shadowed(ImVec2 pos, const std::string& text, ImU32 col, ImFont* font, float font_size = 13.f)
{
    auto alpha = col >> 24;

    if (alpha > 0)
    {
        ImGui::GetBackgroundDrawList()->AddText(font, font_size, ImVec2(pos.x - 1.f, pos.y + 1.f), IM_COL32(0, 0, 0, alpha), text.c_str());
        ImGui::GetBackgroundDrawList()->AddText(font, font_size, ImVec2(pos.x + 1.f, pos.y - 1.f), IM_COL32(0, 0, 0, alpha), text.c_str());
        ImGui::GetBackgroundDrawList()->AddText(font, font_size, ImVec2(pos.x + 1.f, pos.y + 1.f), IM_COL32(0, 0, 0, alpha), text.c_str());
        ImGui::GetBackgroundDrawList()->AddText(font, font_size, ImVec2(pos.x - 1.f, pos.y - 1.f), IM_COL32(0, 0, 0, alpha), text.c_str());
        ImGui::GetBackgroundDrawList()->AddText(font, font_size, ImVec2(pos.x, pos.y + 1.f), IM_COL32(0, 0, 0, alpha), text.c_str());
        ImGui::GetBackgroundDrawList()->AddText(font, font_size, ImVec2(pos.x, pos.y - 1.f), IM_COL32(0, 0, 0, alpha), text.c_str());
        ImGui::GetBackgroundDrawList()->AddText(font, font_size, ImVec2(pos.x + 1.f, pos.y), IM_COL32(0, 0, 0, alpha), text.c_str());
        ImGui::GetBackgroundDrawList()->AddText(font, font_size, ImVec2(pos.x - 1.f, pos.y), IM_COL32(0, 0, 0, alpha), text.c_str());
    }

    ImGui::GetBackgroundDrawList()->AddText(font, font_size, pos, col, text.c_str());
}

static Roblox::Vector3 crossProduct(const Roblox::Vector3& vec1, const Roblox::Vector3& vec2) {
    return {
        vec1.y * vec2.z - vec1.z * vec2.y,
        vec1.z * vec2.x - vec1.x * vec2.z,
        vec1.x * vec2.y - vec1.y * vec2.x
    };
}

static Roblox::Matrix3x3 LookAtToMatrix(const Roblox::Vector3& from, const Roblox::Vector3& to) {
    Roblox::Vector3 forward = Roblox::Vector3{ to.x - from.x, to.y - from.y, to.z - from.z }.normalize();
    Roblox::Vector3 right = crossProduct({ 0, 1, 0 }, forward).normalize();
    Roblox::Vector3 up = crossProduct(forward, right);

    Roblox::Matrix3x3 lookAtMatrix{};
    lookAtMatrix.data[0] = -right.x;  lookAtMatrix.data[1] = up.x;  lookAtMatrix.data[2] = -forward.x;
    lookAtMatrix.data[3] = right.y;  lookAtMatrix.data[4] = up.y;  lookAtMatrix.data[5] = -forward.y;
    lookAtMatrix.data[6] = -right.z;  lookAtMatrix.data[7] = up.z;  lookAtMatrix.data[8] = -forward.z;

    return lookAtMatrix;
}

static std::vector<Roblox::Vector3> GetCorners(const Roblox::Vector3& partCF, const Roblox::Vector3& partSize) {
    std::vector<Roblox::Vector3> corners;

    for (int X = -1; X <= 1; X += 2) {
        for (int Y = -1; Y <= 1; Y += 2) {
            for (int Z = -1; Z <= 1; Z += 2) {
                Roblox::Vector3 cornerPosition = {
                    partCF.x + partSize.x * X,
                    partCF.y + partSize.y * Y,
                    partCF.z + partSize.z * Z
                };
                corners.push_back(cornerPosition);
            }
        }
    }
    return corners;
}

static ImVec4 getHealthColor(float healthPercentage) {
    if (healthPercentage <= 10) return ImColor(245, 66, 66);
    if (healthPercentage <= 20) return ImColor(245, 102, 66);
    if (healthPercentage <= 30) return ImColor(245, 123, 66);
    if (healthPercentage <= 40) return ImColor(245, 150, 66);
    if (healthPercentage <= 50) return ImColor(245, 176, 66);
    if (healthPercentage <= 60) return ImColor(245, 215, 66);
    if (healthPercentage <= 70) return ImColor(239, 245, 66);
    if (healthPercentage <= 80) return ImColor(161, 245, 66);
    if (healthPercentage <= 90) return ImColor(66, 245, 111);
    return ImColor(66, 245, 164);
}

void Roblox::hook_esp()
{
    ImDrawList* draw = ImGui::GetBackgroundDrawList();

    POINT cursor_pos;
    GetCursorPos(&cursor_pos);
    ScreenToClient(FindWindowA("WINDOWSCLIENT", NULL), &cursor_pos);

    Roblox::Instance visualengine = GlobalVars::visualengine;
    if (visualengine.address == 0) return;

    Roblox::Vector2 dimensions = visualengine.getScreenDimensions();
    Roblox::Entity localplayer = GlobalVars::localplayer;
    Roblox::Instance localplayerHead = localplayer.head;

    if (GlobalVars::camera_fieldOfViewChange) {

        Roblox::Instance camera = GlobalVars::workspace.getCamera();
        camera.writeCameraFieldOfView(GlobalVars::camera_fieldOfView);
    }

    const ImU32 black_color = ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1));

    const float box_width = 3.25f;
    const float box_height = 5.5f;

    if (GlobalVars::drawFov) {

        draw->AddCircle(ImVec2(cursor_pos.x, cursor_pos.y), GlobalVars::fovRadius, GlobalVars::fov_color.u32(), 32, 1.0f);
    }

    if (GlobalVars::esp)
    {

        for (Roblox::Entity& player : GlobalVars::cached_players)
        {
            if (player.address == localplayer.address)
                continue;

            if (g_whitelisted_players.size() > 0)
            {

                auto ite = std::find(g_whitelisted_players.begin(), g_whitelisted_players.end(), player.name);
                auto is_whitelisted = ite != g_whitelisted_players.end();

                if (!is_whitelisted)
                    continue;

            }

            Roblox::Instance character = player.character;
            Roblox::Instance humanoid = player.humanoid;
            if (!character.address || !humanoid.address)
                continue;

            Roblox::Instance head = player.head;
            Roblox::Instance upperTorso = player.upperTorso;
            Roblox::Instance humanoidRootPart = player.rootPart;
            Roblox::Instance lowerTorso = player.lowerTorso;
            Roblox::Instance leftUpperLeg = player.leftUpperLeg;
            Roblox::Instance rightUpperLeg = player.rightUpperLeg;
            Roblox::Instance leftUpperArm = player.leftUpperArm;
            Roblox::Instance rightUpperArm = player.rightUpperArm;
            Roblox::Instance leftHand = player.leftHand;
            Roblox::Instance rightHand = player.rightHand;
            Roblox::Instance leftFoot = player.leftFoot;
            Roblox::Instance rightFoot;
            if (!head.address || !humanoidRootPart.address)
                continue;

            Roblox::Matrix4x4 viewMatrix = visualengine.getViewMatrix();

            Roblox::Vector3 headPosition = head.getPartPosition();
            Roblox::Vector3 rootPartPosition = humanoidRootPart.getPartPosition();

            Roblox::Vector2 headPosition2D = Roblox::worldToScreen(headPosition, dimensions, viewMatrix);
            Roblox::Matrix3x3 rootPartRotation = humanoidRootPart.getPartRotation();

            Roblox::Vector3 rootPartLookVector = { -rootPartRotation.data[2] / 100, -rootPartRotation.data[5] / 100, -rootPartRotation.data[8] / 100 };

            Roblox::Vector3 look_position_3D = rootPartPosition + rootPartLookVector;
            Roblox::Vector3 look_direction = rootPartLookVector.normalize();
            Roblox::Vector3 side_vector1 = crossProduct({ 0.0f, 1.0f, 0.0f }, look_direction);
            Roblox::Vector3 side_vector2 = crossProduct(look_direction, side_vector1);

            float distance = (localplayerHead.getPartPosition() - player.rootPart.getPartPosition()).getMagnitude();

            if (distance > GlobalVars::max_render_distance)
                continue;

            bool knockedCheck = GlobalVars::esp_checks[0];
            bool grabbedCheck = GlobalVars::esp_checks[1];
            bool teamCheck = GlobalVars::esp_checks[2];

            if (knockedCheck && player.knockedOut.getBoolFromValue()) {
                continue;
            }

            if (grabbedCheck && player.character.findFirstChild(XorStr("GRABBING_CONSTRAINT"), true).address != 0) {
                continue;
            }

            if (teamCheck && player.team.address == localplayer.team.address) {
                continue;
            }

            if (GlobalVars::box_esp) {
                ImU32 box_color = GlobalVars::box_color.u32();
                ImU32 filled_box_color = GlobalVars::filled_box_color.u32();
                ImU32 box_outline_color = IM_COL32(0, 0, 0, 255);

                std::vector<Roblox::Vector2> screenPoints;
                Roblox::Instance parts[] = {
                    player.head, player.upperTorso, player.lowerTorso,
                    player.leftUpperLeg, player.rightUpperLeg,
                    player.leftUpperArm, player.rightUpperArm,
                    player.leftHand, player.rightHand,
                    player.rightFoot, player.leftFoot,
                };

                for (auto& part : parts) {
                    if (!part.address)
                        continue;

                    Roblox::Vector2 screenPoint = Roblox::worldToScreen(part.getPartPosition(), dimensions, viewMatrix);
                    if (screenPoint.x != -1 && screenPoint.y != -1) {
                        screenPoints.push_back(screenPoint);
                    }
                }

                if (screenPoints.size() < 2) {
                    continue;
                }

                float left = screenPoints[0].x;
                float top = screenPoints[0].y;
                float right = screenPoints[0].x;
                float bottom = screenPoints[0].y;

                for (const auto& pt : screenPoints) {
                    if (pt.x < left) left = pt.x;
                    if (pt.y < top) top = pt.y;
                    if (pt.x > right) right = pt.x;
                    if (pt.y > bottom) bottom = pt.y;
                }

                float expand_amount = 3.0f;
                ImVec2 top_left(left - expand_amount, top - expand_amount);
                ImVec2 bottom_right(right + expand_amount, bottom + expand_amount);

                float box_thickness = 1.0f;

                switch (GlobalVars::box_type) {
                case 0: {
                    draw->AddRect(ImVec2(top_left.x - box_thickness, top_left.y - box_thickness), ImVec2(bottom_right.x + box_thickness, bottom_right.y + box_thickness), box_outline_color);
                    draw->AddRect(ImVec2(top_left.x - box_thickness + 1, top_left.y - box_thickness + 1), ImVec2(bottom_right.x + box_thickness - 1, bottom_right.y + box_thickness - 1), box_color);
                    draw->AddRect(ImVec2(top_left.x - box_thickness + 2, top_left.y - box_thickness + 2), ImVec2(bottom_right.x + box_thickness - 2, bottom_right.y + box_thickness - 2), box_outline_color);

                    if (GlobalVars::box_filled) {
                        draw->AddRectFilled(ImVec2(top_left.x, top_left.y), ImVec2(bottom_right.x, bottom_right.y), filled_box_color);
                    }
                    break;
                }
                case 1: {
                    auto cornered_rect = [&](ImVec2 pos, ImVec2 size, ImU32 col) {
                        auto draw = ImGui::GetBackgroundDrawList();

                        float X = pos.x; float Y = pos.y;
                        float W = size.x; float H = size.y;

                        float lineW = (size.x / 4);
                        float lineH = (size.y / 4);
                        float lineT = 1;

                        auto outline = IM_COL32(0, 0, 0, col >> 24);

                        // Outline
                        draw->AddLine({ X - lineT + 1.f, Y - lineT }, { X + lineW, Y - lineT }, outline); // top left
                        draw->AddLine({ X - lineT, Y - lineT }, { X - lineT, Y + lineH }, outline);

                        draw->AddLine({ X + W - lineW, Y - lineT }, { X + W + lineT, Y - lineT }, outline); // top right
                        draw->AddLine({ X + W + lineT, Y - lineT }, { X + W + lineT, Y + lineH }, outline);

                        draw->AddLine({ X + W + lineT, Y + H - lineH }, { X + W + lineT, Y + H + lineT }, outline); // bot right
                        draw->AddLine({ X + W - lineW, Y + H + lineT }, { X + W + lineT, Y + H + lineT }, outline);

                        draw->AddLine({ X - lineT, Y + H - lineH }, { X - lineT, Y + H + lineT }, outline); // bot left
                        draw->AddLine({ X - lineT, Y + H + lineT }, { X + lineW, Y + H + lineT }, outline);

                        // Inner lines
                        draw->AddLine({ X - (lineT - 3), Y - (lineT - 2) }, { X + lineW, Y - (lineT - 2) }, outline); // top left
                        draw->AddLine({ X - (lineT - 2), Y - (lineT - 2) }, { X - (lineT - 2), Y + lineH }, outline);

                        draw->AddLine({ X - (lineT - 2), Y + H - lineH }, { X - (lineT - 2), Y + H + (lineT - 2) }, outline); // bot left
                        draw->AddLine({ X - (lineT - 2), Y + H + (lineT - 2) }, { X + lineW, Y + H + (lineT - 2) }, outline);

                        draw->AddLine({ X + W - lineW, Y - (lineT - 2) }, { X + W + (lineT - 2), Y - (lineT - 2) }, outline); // top right
                        draw->AddLine({ X + W + (lineT - 2), Y - (lineT - 2) }, { X + W + (lineT - 2), Y + lineH }, outline);

                        draw->AddLine({ X + W + (lineT - 2), Y + H - lineH }, { X + W + (lineT - 2), Y + H + (lineT - 2) }, outline); // bot right
                        draw->AddLine({ X + W - lineW, Y + H + (lineT - 2) }, { X + W + (lineT - 2), Y + H + (lineT - 2) }, outline);

                        // Inline
                        draw->AddLine({ X, Y }, { X, Y + lineH }, col); // top left
                        draw->AddLine({ X + 1.f, Y }, { X + lineW, Y }, col);

                        draw->AddLine({ X + W - lineW, Y }, { X + W, Y }, col); // top right
                        draw->AddLine({ X + W , Y }, { X + W, Y + lineH }, col);

                        draw->AddLine({ X, Y + H - lineH }, { X, Y + H }, col); // bot left
                        draw->AddLine({ X, Y + H }, { X + lineW, Y + H }, col);

                        draw->AddLine({ X + W - lineW, Y + H }, { X + W, Y + H }, col); // bot right
                        draw->AddLine({ X + W, Y + H - lineH }, { X + W, Y + H }, col);
                        };

                    cornered_rect({ top_left.x, top_left.y }, { bottom_right.x - top_left.x, bottom_right.y - top_left.y }, box_color);
                    if (GlobalVars::box_filled) {
                        draw->AddRectFilled(ImVec2(top_left.x, top_left.y), ImVec2(bottom_right.x, bottom_right.y), filled_box_color);
                    }
                    break;
                }
                }

                float alpha_factor = 1.0f;

                if (GlobalVars::health_bar) {
                    float health = humanoid.getHumanoidHealth();
                    float maxHealth = humanoid.getHumanoidMaxHealth();

                    float health_bar_width = 5.0f;
                    ImVec2 pos = ImVec2(top_left.x - health_bar_width - 4.0f, top_left.y);
                    ImVec2 size = ImVec2(health_bar_width, bottom_right.y - top_left.y);

                    auto health_bar = [=](float max, float current, ImVec2 pos, ImVec2 size, float alpha_factor, float x_pad = 4.f, float bar_width = 5.f, bool blue = false) {
                        int clamped_health = std::min(max, current);
                        float bar_height = size.y * clamped_health / max;

                        draw->AddRectFilled(ImVec2(pos.x, pos.y), ImVec2(pos.x + bar_width, pos.y + size.y), IM_COL32(60, 60, 60, static_cast<int>(200 * alpha_factor)));

                        auto modifier = (std::clamp(current, (max * 0.25f), (max * 0.75f)) - (max * 0.25f)) / (max * 0.5f);
                        auto r = static_cast<int>(100.f + (135.f * (1.f - modifier)));
                        auto g = static_cast<int>(50.f + ((155.f) * modifier));

                        draw->AddRectFilled(ImVec2(pos.x + 1.0f, pos.y + size.y - bar_height), ImVec2(pos.x + bar_width - 1.0f, pos.y + size.y), IM_COL32(blue ? 70 : r, blue ? 30.f : g, blue ? g : 70.f, static_cast<int>(255 * alpha_factor)));

                        draw->AddRect(ImVec2(pos.x, pos.y), ImVec2(pos.x + bar_width, pos.y + size.y), IM_COL32(0, 0, 0, static_cast<int>(255 * alpha_factor)));

                        std::string healthText = std::to_string(static_cast<int>(current));
                        ImVec2 textSize = ImGui::CalcTextSize(healthText.c_str());

                        ImVec2 textPos = ImVec2(pos.x + (bar_width - textSize.x) / 2.0f, pos.y + size.y - bar_height - textSize.y - -11.5f);

                        text_shadowed(textPos, healthText, IM_COL32(255, 255, 255, static_cast<int>(255 * alpha_factor)), verdana_12, 9.f);
                        };

                    health_bar(maxHealth, health, pos, size, alpha_factor);
                }


                int current_armor = 0;

                if (GlobalVars::shield_bar) {
                    auto armor_obj = player.armor_obj;

                    if (armor_obj.address)
                        current_armor = read<int>(armor_obj.address + Offsets::Misc::Value);

                    bool render_shield = current_armor > 0;

                    if (render_shield) {
                        auto shield_bar = [=](float max, float current, ImVec2 shield_pos, ImVec2 shield_size, float alpha_factor) {
                            auto adj_pos_x = shield_pos.x - 1;
                            auto adj_size_x = shield_size.x + 2;

                            auto adj_from = ImVec2(adj_pos_x, shield_pos.y);
                            auto adj_to = ImVec2(adj_pos_x + adj_size_x, shield_pos.y + shield_size.y);

                            auto bar_width = current * adj_size_x / max;

                            draw->AddRectFilled(adj_from, adj_to, IM_COL32(60, 60, 60, static_cast<int>(200 * alpha_factor)));
                            draw->AddRectFilled(ImVec2((adj_pos_x + adj_size_x) - bar_width, shield_pos.y), ImVec2(adj_pos_x + adj_size_x, shield_pos.y + shield_size.y), IM_COL32(52, 103, 235, static_cast<int>(255 * alpha_factor)));
                            draw->AddRect(ImVec2(adj_from.x - 1.f, adj_from.y - 1.f), ImVec2(adj_to.x + 1.f, adj_to.y + 1.f), IM_COL32(0, 0, 0, static_cast<int>(255 * alpha_factor)));
                            };

                        ImVec2 shield_pos = ImVec2(top_left.x, bottom_right.y + 5.f);
                        ImVec2 shield_size = ImVec2(bottom_right.x - top_left.x, 2.f);

                        shield_bar(200.f, current_armor, shield_pos, shield_size, alpha_factor);
                    }
                }

                if (GlobalVars::name_esp) {
                    ImVec2 boxTopCenter((left + right) / 2, top);
                    const char* playerName = player.name.c_str();

                    ImVec2 textPosition = { boxTopCenter.x - ImGui::CalcTextSize(playerName).x / 2, boxTopCenter.y - 20 };

                    text_shadowed(textPosition, player.name, IM_COL32(255, 255, 255, 255), verdana_12);
                }

                if (GlobalVars::distance_esp) {
                    float distanceMagnitude = (localplayerHead.getPartPosition() - player.rootPart.getPartPosition()).getMagnitude();
                    distanceMagnitude = roundf(distanceMagnitude * 100) / 100;

                    char distanceStr[32];
                    sprintf_s(distanceStr, sizeof(distanceStr), XorStr("%.0fm"), distanceMagnitude);

                    ImVec2 boxBottomCenter((left + right) / 2, bottom);

                    float textWidth = ImGui::CalcTextSize(distanceStr).x;

                    ImVec2 distancePos = { boxBottomCenter.x - textWidth / 2, boxBottomCenter.y + 10 };

                    text_shadowed(distancePos, distanceStr, IM_COL32(255, 255, 255, 255), verdana_12);
                }

                if (GlobalVars::prediction_dot)
                {
                    ImU32 dotColor = GlobalVars::prediction_dot_color.u32();
                    Roblox::Vector3 predictedPosition;

                    if (GlobalVars::aimbot_type == 0) {
                        predictedPosition = rootPartPosition + (humanoidRootPart.getPartVelocity() / Roblox::Vector3{ GlobalVars::mouse_prediction_x, GlobalVars::mouse_prediction_y, GlobalVars::mouse_prediction_x });
                    }
                    else {
                        predictedPosition = rootPartPosition + (humanoidRootPart.getPartVelocity() / Roblox::Vector3{ GlobalVars::camera_prediction_x, GlobalVars::camera_prediction_y, GlobalVars::camera_prediction_x });
                    }

                    Roblox::Vector2 dotPosition = Roblox::worldToScreen(predictedPosition, dimensions, viewMatrix);

                    if (dotPosition.x == -1)
                        continue;

                    draw->AddCircleFilled(ImVec2(dotPosition.x, dotPosition.y), 5, dotColor, 32);
                }

                if (GlobalVars::snapline_esp)
                {
                    if (headPosition2D.x == -1)
                        continue;

                    ImVec2 snaplineStart;
                    ImVec2 screenCenter = ImVec2(dimensions.x / 2, dimensions.y);

                    switch (GlobalVars::snapline_position)
                    {
                    case 0:
                        snaplineStart = ImVec2(dimensions.x / 2, dimensions.y);
                        break;
                    case 1:
                        snaplineStart = ImVec2(dimensions.x / 2, 0);
                        break;
                    case 2:
                        snaplineStart = ImVec2(cursor_pos.x, cursor_pos.y);
                        break;
                    default:
                        snaplineStart = ImVec2(dimensions.x / 2, 0);
                        break;
                    }

                    draw->AddLine(snaplineStart, ImVec2(headPosition2D.x, headPosition2D.y), GlobalVars::snapline_color.u32());
                }


                if (GlobalVars::skeleton_esp)
                {

                    ImU32 skeleton_color = GlobalVars::skeleton_color.u32();

                    if (headPosition2D.x == -1)
                        continue;

                    Roblox::Vector2 playerUpperTorsoPosition = Roblox::worldToScreen(player.upperTorso.getPartPosition(), dimensions, viewMatrix);
                    Roblox::Vector2 playerLowerTorsoPosition = Roblox::worldToScreen(player.lowerTorso.getPartPosition(), dimensions, viewMatrix);

                    if (playerUpperTorsoPosition.x == -1 || playerLowerTorsoPosition.x == -1)
                        continue;

                    Roblox::Vector2 playerLeftUpperLegPosition = Roblox::worldToScreen(player.leftUpperLeg.getPartPosition(), dimensions, viewMatrix);
                    Roblox::Vector2 playerLeftFootPosition = Roblox::worldToScreen(player.leftFoot.getPartPosition(), dimensions, viewMatrix);

                    if (playerLeftUpperLegPosition.x == -1 || playerLeftFootPosition.x == -1)
                        continue;

                    Roblox::Vector2 playerRightUpperLegPosition = Roblox::worldToScreen(player.rightUpperLeg.getPartPosition(), dimensions, viewMatrix);
                    Roblox::Vector2 playerRightFootPosition = Roblox::worldToScreen(player.rightFoot.getPartPosition(), dimensions, viewMatrix);

                    if (playerRightUpperLegPosition.x == -1 || playerRightFootPosition.x == -1)
                        continue;

                    Roblox::Vector2 playerLeftUpperArmPosition = Roblox::worldToScreen(player.leftUpperArm.getPartPosition(), dimensions, viewMatrix);
                    Roblox::Vector2 playerLeftHandPosition = Roblox::worldToScreen(player.leftHand.getPartPosition(), dimensions, viewMatrix);

                    if (playerLeftUpperArmPosition.x == -1 || playerLeftHandPosition.x == -1)
                        continue;

                    Roblox::Vector2 playerRightUpperArmPosition = Roblox::worldToScreen(player.rightUpperArm.getPartPosition(), dimensions, viewMatrix);
                    Roblox::Vector2 playerRightHandPosition = Roblox::worldToScreen(player.rightHand.getPartPosition(), dimensions, viewMatrix);

                    if (playerRightUpperArmPosition.x == -1 || playerRightHandPosition.x == -1)
                        continue;

                    if (player.r15 == 1) {
                        draw->AddLine(ImVec2(headPosition2D.x, headPosition2D.y), ImVec2(playerUpperTorsoPosition.x, playerUpperTorsoPosition.y), skeleton_color);
                        draw->AddLine(ImVec2(playerUpperTorsoPosition.x, playerUpperTorsoPosition.y), ImVec2(playerLowerTorsoPosition.x, playerLowerTorsoPosition.y), skeleton_color);

                        draw->AddLine(ImVec2(playerLowerTorsoPosition.x, playerLowerTorsoPosition.y), ImVec2(playerLeftUpperLegPosition.x, playerLeftUpperLegPosition.y), skeleton_color);
                        draw->AddLine(ImVec2(playerLeftUpperLegPosition.x, playerLeftUpperLegPosition.y), ImVec2(playerLeftFootPosition.x, playerLeftFootPosition.y), skeleton_color);

                        draw->AddLine(ImVec2(playerLowerTorsoPosition.x, playerLowerTorsoPosition.y), ImVec2(playerRightUpperLegPosition.x, playerRightUpperLegPosition.y), skeleton_color);
                        draw->AddLine(ImVec2(playerRightUpperLegPosition.x, playerRightUpperLegPosition.y), ImVec2(playerRightFootPosition.x, playerRightFootPosition.y), skeleton_color);

                        draw->AddLine(ImVec2(playerUpperTorsoPosition.x, playerUpperTorsoPosition.y), ImVec2(playerLeftUpperArmPosition.x, playerLeftUpperArmPosition.y), skeleton_color);
                        draw->AddLine(ImVec2(playerLeftUpperArmPosition.x, playerLeftUpperArmPosition.y), ImVec2(playerLeftHandPosition.x, playerLeftHandPosition.y), skeleton_color);

                        draw->AddLine(ImVec2(playerUpperTorsoPosition.x, playerUpperTorsoPosition.y), ImVec2(playerRightUpperArmPosition.x, playerRightUpperArmPosition.y), skeleton_color);
                        draw->AddLine(ImVec2(playerRightUpperArmPosition.x, playerRightUpperArmPosition.y), ImVec2(playerRightHandPosition.x, playerRightHandPosition.y), skeleton_color);

                    }
                    else
                    {
                        draw->AddLine(ImVec2(headPosition2D.x, headPosition2D.y), ImVec2(playerUpperTorsoPosition.x, playerUpperTorsoPosition.y), skeleton_color);
                        draw->AddLine(ImVec2(playerUpperTorsoPosition.x, playerUpperTorsoPosition.y), ImVec2(playerRightHandPosition.x, playerRightHandPosition.y), skeleton_color);
                        draw->AddLine(ImVec2(playerUpperTorsoPosition.x, playerUpperTorsoPosition.y), ImVec2(playerLeftHandPosition.x, playerLeftHandPosition.y), skeleton_color);
                        draw->AddLine(ImVec2(playerUpperTorsoPosition.x, playerUpperTorsoPosition.y), ImVec2(playerRightFootPosition.x, playerRightFootPosition.y), skeleton_color);
                        draw->AddLine(ImVec2(playerUpperTorsoPosition.x, playerUpperTorsoPosition.y), ImVec2(playerLeftFootPosition.x, playerLeftFootPosition.y), skeleton_color);
                    }
                }

                if (GlobalVars::undercircle_esp) {

                    ImU32 circle_color = GlobalVars::undercircle_color.u32();

                    bool filled = GlobalVars::undercircle_filled;

                    Roblox::Vector3 pos = { rootPartPosition.x, rootPartPosition.y - 3, rootPartPosition.z };

                    const float radius = GlobalVars::undercircle_radius;
                    const int num_segments = GlobalVars::undercircle_segments;

                    std::vector<ImVec2> points(num_segments + 1);
                    float step = 6.2831f / num_segments;
                    float theta = 0.f;

                    int wrong = 0;

                    for (int i = 0; i <= num_segments; i++, theta += step)
                    {
                        Roblox::Vector3 worldSpace = { pos.x + radius * cos(theta), pos.y, pos.z - radius * sin(theta) };
                        Roblox::Vector2 screenSpace = Roblox::worldToScreen(worldSpace, dimensions, viewMatrix);
                        if (screenSpace.x == -1)
                        {
                            wrong = wrong + 1;
                            continue;
                        }
                        points[static_cast<std::vector<ImVec2, std::allocator<ImVec2>>::size_type>(i) - wrong].x = screenSpace.x;
                        points[static_cast<std::vector<ImVec2, std::allocator<ImVec2>>::size_type>(i) - wrong].y = screenSpace.y;
                    }

                    if (filled)
                        draw->AddConvexPolyFilled(points.data(), points.size() - wrong - 1, circle_color);
                    else
                        draw->AddPolyline(points.data(), points.size() - wrong, circle_color, true, 1.f);
                }

                if (GlobalVars::looktracer) {
                    Roblox::Matrix3x3 headRotation = head.getPartRotation();
                    Roblox::Vector3 headLookVector = { -headRotation.data[2] * 2, -headRotation.data[5] * 2, -headRotation.data[8] * 2 };

                    Roblox::Vector3 headLookVectorPosition = headPosition + headLookVector;
                    Roblox::Vector2 look_position_2D = Roblox::worldToScreen(headLookVectorPosition, dimensions, viewMatrix);

                    if (look_position_2D.x == -1 || headPosition2D.x == -1)
                        continue;

                    draw->AddLine(ImVec2(headPosition2D.x, headPosition2D.y), ImVec2(look_position_2D.x, look_position_2D.y), GlobalVars::looktracer_color.u32());
                }

                if (GlobalVars::aimviewer) {

                    if (player.hasGunEquipped == false)
                        continue;

                    ImU32 aimviewer_color = GlobalVars::aimviewer_color.u32();
                    Roblox::Vector3 to = player.mousePosition.getVec3FromValue();
                    if (to.x == 0 && to.y == 0 && to.z == 0)
                        continue;

                    Roblox::Vector3 from = player.head.getPartPosition();

                    Roblox::Vector2 from_2D = Roblox::worldToScreen(from, dimensions, viewMatrix);
                    Roblox::Vector2 to_2D = Roblox::worldToScreen(to, dimensions, viewMatrix);

                    if (from_2D.x == -1 || to_2D.x == -1)
                        continue;

                    draw->AddLine(ImVec2(from_2D.x, from_2D.y), ImVec2(to_2D.x, to_2D.y), aimviewer_color);
                }
            }
        }
    }
}