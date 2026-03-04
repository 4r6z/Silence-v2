#include <iostream>
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>
#include <ShlObj.h>
#include "../rbx/class/Classes.hpp"

#include "overlay/Overlay.hpp"
#include "rbx/class/Classes.hpp"
#include "rbx/globals/GlobalVars.hpp"

#include "rbx/aimbot/Aimbot.hpp"
#include "rbx/esp/ESP.hpp"
#include "rbx/exploits/Exploits.hpp"
#include "rbx/triggerbot/Triggerbot.hpp"

#include "Headers/XorStr.hpp"
#include "rbx/Offsets.hpp"
#include "driver/DriverImplementation.hpp"
#include "into.hpp"

int main() {
    std::thread(Roblox::hook_aimbot).detach();
    std::thread(Roblox::hook_exploits).detach();
    std::thread(Roblox::hook_triggerbot).detach();
    std::thread(rbxcache::cache_entities).detach();
    Silence::Initialize();
    return 0;
}