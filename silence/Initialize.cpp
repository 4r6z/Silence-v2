#include <thread>
#include <filesystem>
#include <fstream>
#include <ShlObj.h>
#include <algorithm>
#include <cctype>
#include <unordered_set>
#include <unordered_map>
#include <typeinfo>

const int DEBOUNCE_THRESHOLD = 5;
const int LEAVE_GAME_DEBOUNCE_THRESHOLD = 5;

#define DEBOUNCE_THRESHOLD 5

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

static std::string readstring(std::uint64_t address);

static uint64_t get_datamodel() {
	uintptr_t base = mem::find_image();
	uintptr_t scheduler = 0;
	while (!scheduler) {
		scheduler = read<uintptr_t>(base + Offsets::TaskScheduler::Pointer);
		if (!scheduler);
	}

	uintptr_t jobStart = 0;
	uintptr_t jobEnd = 0;
	while (!jobStart || !jobEnd || jobStart >= jobEnd) {
		jobStart = read<uintptr_t>(scheduler + Offsets::TaskScheduler::JobStart);
		jobEnd = read<uintptr_t>(scheduler + Offsets::TaskScheduler::JobEnd);
		if (!jobStart || !jobEnd || jobStart >= jobEnd);
	}

	for (uintptr_t job = jobStart; job < jobEnd; job += 0x10) {
		uintptr_t jobAddress = read<uintptr_t>(job);
		if (!jobAddress) continue;

		std::string jobName = readstring(jobAddress + Offsets::TaskScheduler::JobName);

		if (jobName == "RenderJob") {
			auto RenderView = read<uintptr_t>(jobAddress + Offsets::RenderJob::RenderView);
			if (!RenderView) continue;
            
            virtualaddy = RenderView;

			uintptr_t FakeDataModel = 0;
			while (!FakeDataModel) {
				FakeDataModel = read<uintptr_t>(base + Offsets::FakeDataModel::Pointer);
				if (!FakeDataModel);
			}

			uintptr_t realDataModel = 0;
			while (!realDataModel) {
				realDataModel = read<uintptr_t>(FakeDataModel + Offsets::FakeDataModel::RealDataModel);
				if (!realDataModel);
			}
			return realDataModel;
		}
	}
	return 0;
}

int Roblox::Instance::ReadPlayer(std::uint64_t address) {
    return read<int>(address);
}


static std::string readstring(std::uint64_t address)
{
    std::string string;
    char character = 0;
    int char_size = sizeof(character);
    int offset = 0;

    string.reserve(204);
    const auto stringLength = read<int>(address + 0x18);
    std::uint64_t stringAddress = stringLength >= 16u ? read<std::uint64_t>(address) : address;

    while (offset < 200)
    {
        character = read<char>(stringAddress + offset);

        if (character == 0)
            break;

        offset += char_size;
        string.push_back(character);
    }

    return string;
}

// Informations

std::string Roblox::Instance::getInstanceName() const
{
    return readstring(read<std::uint64_t>(this->address + Offsets::Instance::Name));
}

std::string Roblox::Instance::getInstanceDisplayName()
{
    return readstring(read<std::uint64_t>(this->address + Offsets::Humanoid::DisplayName));
}

std::string Roblox::Instance::getInstanceClass()
{
    std::uint64_t descriptor = read<std::uint64_t>(this->address + Offsets::Instance::ClassDescriptor);
    if (!descriptor) return "";
    return readstring(read<std::uint64_t>(descriptor + Offsets::Instance::ClassName));
}

// Children / Parents

std::vector<Roblox::Instance> Roblox::Instance::getInstancesChildren(bool isHumanoid)
{
    std::vector<Roblox::Instance> container;

    if (!this->address)
        return container;

    std::uint64_t children_list_ptr = read<std::uint64_t>(this->address + Offsets::Instance::ChildrenStart);
    if (!children_list_ptr)
        return container;

    std::uint64_t start = read<std::uint64_t>(children_list_ptr);
    std::uint64_t end = read<std::uint64_t>(children_list_ptr + 8);

    if (!start || !end || start >= end || (end - start) > 0x100000)
        return container;

    if (isHumanoid == true) {
        auto startTime = std::chrono::steady_clock::now();

        for (auto instances = start; instances != end; instances += 16u)
        {
            auto currentTime = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed = currentTime - startTime;
            container.emplace_back(read<Roblox::Instance>(instances));

            if (elapsed.count() >= 1.25) {
                return std::vector<Roblox::Instance>();
            }
        }
    }
    else {
        for (auto instances = start; instances != end; instances += 16u)
        {
            container.emplace_back(read<Roblox::Instance>(instances));
        }
    }

    return container;
}

Roblox::Instance Roblox::Instance::getInstanceParent()
{
    return read<Roblox::Instance>(this->address + Offsets::Instance::Parent);;
}

Roblox::Instance Roblox::Instance::findFirstChild(std::string child, bool isCharChild)
{
    Roblox::Instance returnObject;
    std::vector<Roblox::Instance> instancesChildren = this->getInstancesChildren(isCharChild);
    if (isCharChild && instancesChildren.empty() == true) {
        returnObject.address = 0;
        GlobalVars::threadcrash = true;
        return returnObject;
    }
    for (Roblox::Instance& object : instancesChildren)
    {
        if (object.getInstanceName() == child)
        {
            return static_cast<Roblox::Instance>(object);
        }
    }
    return returnObject;
}

Roblox::Instance Roblox::Instance::findFirstChildOfClass(std::string child)
{
    Roblox::Instance returnObject;
    for (Roblox::Instance& object : this->getInstancesChildren(false))
    {
        if (object.getInstanceClass() == child)
        {
            return static_cast<Roblox::Instance>(object);
        }
    }
    return returnObject;
}

Roblox::Vector2 Roblox::Instance::getScreenDimensions()
{
    return read<Roblox::Vector2>(this->address + Offsets::VisualEngine::Dimensions);
}

Roblox::Matrix4x4 Roblox::Instance::getViewMatrix()
{
    return read<Roblox::Matrix4x4>(this->address + Offsets::VisualEngine::ViewMatrix);
}

// Entity

Roblox::Instance Roblox::Instance::getLocalEntity()
{
    return read<Roblox::Instance>(this->address + Offsets::Player::LocalPlayer);
}

Roblox::Instance Roblox::Instance::getEntityModelInstance()
{
    return read<Roblox::Instance>(this->address + Offsets::Player::ModelInstance);
}

std::int32_t Roblox::Instance::getHumanoidRigType()
{
    return read<std::uint8_t>(this->address + Offsets::Humanoid::RigType);
}

Roblox::Instance Roblox::Instance::getModelInstancePrimaryPart()
{
    return read<Roblox::Instance>(this->address + Offsets::Model::PrimaryPart);
}

Roblox::Instance Roblox::Instance::getEntityTeam()
{
    return read<Roblox::Instance>(this->address + Offsets::Player::Team);
}

void Roblox::Instance::setPartCFrame(const CFrame& cf) {
    write<CFrame>(read<std::uint64_t>(this->address + Offsets::BasePart::Primitive) + Offsets::Camera::Rotation, cf);
}


union convertion
{
    std::uint64_t hex;
    float f;
} conv;

float Roblox::Instance::getHumanoidHealth()
{
    auto one = read<std::uint64_t>(this->address + Offsets::Humanoid::Health);
    auto two = read<std::uint64_t>(one);

    conv.hex = one ^ two;
    return conv.f;
}

float Roblox::Instance::getHumanoidMaxHealth()
{
    auto one = read<std::uint64_t>(this->address + Offsets::Humanoid::MaxHealth);
    auto two = read<std::uint64_t>(one);

    conv.hex = one ^ two;
    return conv.f;
}

void Roblox::Instance::writeEntityHipHeight(float HipHeight)
{
    write<float>(this->address + Offsets::Humanoid::HipHeight, HipHeight);
    return;
}

void Roblox::Instance::writeEntityJumpPower(float JumpPower)
{
    write<float>(this->address + Offsets::Humanoid::JumpPower, JumpPower);
    return;
}

void Roblox::Instance::writeEntityWalkSpeed(float WalkSpeed)
{
    write<float>(this->address + Offsets::Humanoid::Walkspeed, WalkSpeed);

    float WalkspeedCheckValue = read<float>(this->address + Offsets::Humanoid::Walkspeed);
    if (WalkspeedCheckValue == WalkSpeed)
        write<float>(this->address + Offsets::Humanoid::WalkspeedCheck, WalkSpeed);
    return;
}

// Camera

Roblox::Instance Roblox::Instance::getCamera()
{
    return read<Roblox::Instance>(this->address + Offsets::Workspace::CurrentCamera);
}

Roblox::Vector3 Roblox::Instance::getCameraPosition()
{
    return read<Roblox::Vector3>(this->address + Offsets::Camera::Position);
}

Roblox::Matrix3x3 Roblox::Instance::getCameraRotation()
{
    return read<Roblox::Matrix3x3>(this->address + Offsets::Camera::Rotation);
}

float Roblox::Instance::getCameraFOV()
{
    return read<float>(this->address + Offsets::Camera::FieldOfView);
}

void Roblox::Instance::writeCameraRotation(Roblox::Matrix3x3 Rotation)
{
    write<Roblox::Matrix3x3>(this->address + Offsets::Camera::Rotation, Rotation);
    return;
}

void Roblox::Instance::writeCameraFieldOfView(float fieldOfView)
{
    write<float>(this->address + Offsets::Camera::FieldOfView, fieldOfView / GlobalVars::camera_fieldOfViewScaleFactor);
    return;
}

std::uint64_t Roblox::Instance::cached_input_object = 0;

void Roblox::Instance::initializeMouseService(std::uint64_t address) {
    if (cached_input_object == 0) {
        cached_input_object = read<std::uint64_t>(address + 0x100);
        _mm_prefetch(reinterpret_cast<const char*>(cached_input_object + 0x50C), _MM_HINT_T0);
        _mm_prefetch(reinterpret_cast<const char*>(cached_input_object + 0x514), _MM_HINT_T0);
    }
}

void Roblox::Instance::writeMousePosition(float x, float y) {
    if (cached_input_object != 0) {
        write<float>(cached_input_object + 0x50C, x);
        write<float>(cached_input_object + 0x514, y);
    }
}

// Part

std::uint64_t Roblox::Instance::getPartPrimitive()
{
    return read<std::uint64_t>(this->address + Offsets::BasePart::Primitive);
}

bool Roblox::Instance::setCanCollide(bool value)
{
    uint64_t primitive = read<std::uint64_t>(this->address + Offsets::BasePart::Primitive);
    return write<bool>(primitive + Offsets::PrimitiveFlags::CanCollide, value);
}

void Roblox::Instance::setAnchored(bool value)
{
    auto primitive = read<std::uint64_t>(this->address + Offsets::BasePart::Primitive);
    if (!primitive)
        return;

    write<bool>(primitive + Offsets::PrimitiveFlags::Anchored, value);
}

Roblox::Vector3 Roblox::Instance::getPartPosition() const
{
    Roblox::Instance instance;

    instance.address = this->address;

    return read<Roblox::Vector3>(instance.getPartPrimitive() + Offsets::Primitive::Position);
}

Roblox::Vector3 Roblox::Instance::getPartVelocity()
{
    Roblox::Instance instance;

    instance.address = this->address;

    return read<Roblox::Vector3>(instance.getPartPrimitive() + Offsets::Primitive::AssemblyLinearVelocity);
}

Roblox::Matrix3x3 Roblox::Instance::getPartRotation()
{
    Roblox::Instance instance;
    instance.address = this->address;
    return read<Roblox::Matrix3x3>(instance.getPartPrimitive() + Offsets::Primitive::Rotation);
}

Roblox::Vector3 Roblox::Instance::getPartSize() const
{
    Roblox::Instance instance;
    instance.address = this->address;
    return read<Roblox::Vector3>(instance.getPartPrimitive() + Offsets::Primitive::Size);
}

Roblox::CFrame Roblox::Instance::getPartCFrame()
{
    Roblox::CFrame res{};

    auto primitive = read<std::uint64_t>(this->address + Offsets::BasePart::Primitive);

    if (!primitive)
        return res;

    res = read<Roblox::CFrame>(primitive + Offsets::Primitive::Rotation);
    return res;
}

void Roblox::Instance::setPartPosition(Roblox::Vector3 Position)
{
    Roblox::Instance instance;
    instance.address = this->address;
    auto primitive = instance.getPartPrimitive();
    auto part_pos = read<Roblox::Vector3>(primitive + Offsets::Primitive::Position);
    auto ret = write<Roblox::Vector3>(primitive + Offsets::Primitive::Position, Position);
    auto ret2 = write<Roblox::Vector3>(primitive + Offsets::Primitive::Position, Position);
}

void Roblox::Vector3::setPartVelocity(Roblox::Vector3 Velocity)
{
    Roblox::Instance instance;
    std::uint64_t address = 0;

    instance.address = address;
    auto primitive = instance.getPartPrimitive();

    auto success = write<Roblox::Vector3>(primitive + Offsets::Primitive::AssemblyLinearVelocity, Velocity);
    auto confirmedVelocity = read<Roblox::Vector3>(primitive + Offsets::Primitive::AssemblyLinearVelocity);
}

bool Roblox::Instance::getBoolFromValue()
{
    return read<std::uint8_t>(this->address + Offsets::Misc::Value);
}

Roblox::Vector3 Roblox::Instance::getVec3FromValue()
{
    return read<Roblox::Vector3>(this->address + Offsets::Misc::Value);
}

Roblox::Vector2 Roblox::worldToScreen(Roblox::Vector3 world, Roblox::Vector2 dimensions, Roblox::Matrix4x4 viewmatrix)
{
    Roblox::Vector4 clipCoords{};

    float world_x = world.x;
    float world_y = world.y;
    float world_z = world.z;


    clipCoords.x = (world_x * viewmatrix.data[0]) + (world_y * viewmatrix.data[1]) + (world_z * viewmatrix.data[2]) + viewmatrix.data[3];
    clipCoords.y = (world_x * viewmatrix.data[4]) + (world_y * viewmatrix.data[5]) + (world_z * viewmatrix.data[6]) + viewmatrix.data[7];
    clipCoords.z = (world_x * viewmatrix.data[8]) + (world_y * viewmatrix.data[9]) + (world_z * viewmatrix.data[10]) + viewmatrix.data[11];
    clipCoords.w = (world_x * viewmatrix.data[12]) + (world_y * viewmatrix.data[13]) + (world_z * viewmatrix.data[14]) + viewmatrix.data[15];

    if (clipCoords.w <= 0.0f) {
        return Vector2{ -1, -1 };
    }

    float inv_w = 1.0f / clipCoords.w;
    Vector3 ndc{};
    ndc.x = clipCoords.x * inv_w;
    ndc.y = clipCoords.y * inv_w;
    ndc.z = clipCoords.z * inv_w;

    Roblox::Vector2 position = { (dimensions.x / 2 * ndc.x) + (ndc.x + dimensions.x / 2), -(dimensions.y / 2 * ndc.y) + (ndc.y + dimensions.y / 2) };

    Roblox::Vector2 screenPos{};
    screenPos.x = (ndc.x + 1.0f) * 0.5f * dimensions.x;
    screenPos.y = (1.0f - ndc.y) * 0.5f * dimensions.y;

    return screenPos;
}

static void hook_character() {
    HWND rblx = FindWindowA(0, XorStr("Roblox"));

    while (true) {
        if (GetForegroundWindow() != rblx) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }


        if (!GlobalVars::nojump_cooldown && !GlobalVars::walkspeed && !GlobalVars::jumppower && !GlobalVars::hipheight) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        Roblox::Entity localplayer = GlobalVars::localplayer;
        Roblox::Instance character = localplayer.character;
        Roblox::Instance humanoid = localplayer.humanoid;
        Roblox::Instance currentTool = localplayer.currentTool;

        if (GlobalVars::nojump_cooldown && !GlobalVars::jumppower) {
            if (currentTool.getInstanceName() == XorStr("[Knife]")) {
                humanoid.writeEntityJumpPower(59.5);
            }
            else {
                humanoid.writeEntityJumpPower(50.0);
            }
        }


        if (GlobalVars::walkspeed) {
            humanoid.writeEntityWalkSpeed(GlobalVars::walkspeed_val);
        }

        if (GlobalVars::jumppower) {
            humanoid.writeEntityJumpPower(GlobalVars::jumppower_val);
        }

        if (GlobalVars::hipheight) {
            humanoid.writeEntityHipHeight(GlobalVars::hipheight_val);
        }
    }
    return;
}

void synchronizeLists(std::vector<Roblox::Entity>& oldList, std::vector<Roblox::Entity>& newList) {
    if (newList.empty()) return;

    oldList.erase(
        std::remove_if(oldList.begin(), oldList.end(), [&newList](const Roblox::Entity item) {
            return std::find(newList.begin(), newList.end(), item) == newList.end();
            }
        ),
        oldList.end()
    );

    for (auto it = newList.begin(); it != newList.end(); ++it) {
        auto pos = std::distance(newList.begin(), it);
        if (pos >= oldList.size() || *it != oldList[pos]) {
            oldList.insert(oldList.begin() + pos, *it);
        }
    }

    for (auto& player : oldList) {
        for (auto& player1 : newList) {
            if (player.address == player1.address) {
                if (player1.humanoid.address != 0 && player1.rootPart.getPartPosition() != Roblox::Vector3{ 0,0,0 } && player1.upperTorso.getPartPosition() != Roblox::Vector3{ 0,0,0 } /* && playerPosition1 == playerPosition*/) {
                    player = player1;
                }
                else {
                }
               
                break;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void rbxcache::cache_entities() {
    auto players = GlobalVars::players;
    std::vector<Roblox::Entity> cachedPlayers;

    GlobalVars::threadcrash = false;

    if (players.ReadPlayer(players.address) != GlobalVars::mostFreq) {
        return;
    }

    Roblox::Instance player = players.getLocalEntity();
    if (player.address == 0) return;
    
    Roblox::Entity localPlayer;
    localPlayer.address = player.address;
    localPlayer.character = player.getEntityModelInstance();
    localPlayer.head = localPlayer.character.findFirstChild(XorStr("Head"), true);
    localPlayer.team = player.getEntityTeam();
    localPlayer.humanoid = localPlayer.character.findFirstChild(XorStr("Humanoid"), true);
    localPlayer.rootPart = localPlayer.character.findFirstChild(XorStr("HumanoidRootPart"), true);

    if (localPlayer.humanoid.address == 0) {
        GlobalVars::threadcrash = true;
        return;
    }

    GlobalVars::localplayer = localPlayer;
    std::vector<Roblox::Instance> playersInstances = players.getInstancesChildren(true);

    if (playersInstances.empty() == true) {
        GlobalVars::threadcrash = true;
        return;
    }

    for (Roblox::Instance& player : playersInstances) {
        Roblox::Entity cachedPlayer;
        cachedPlayer.name = player.getInstanceName();
        Roblox::Instance character = player.getEntityModelInstance();
        Roblox::Instance humanoid = character.findFirstChild(XorStr("Humanoid"), true);

        if (character.address == 0) {
            continue;
        }

        cachedPlayer.character = character;
        cachedPlayer.team = player.getEntityTeam();
        cachedPlayer.address = player.address;
        cachedPlayer.head = character.findFirstChild(XorStr("Head"), true);
        cachedPlayer.humanoid = humanoid;
        cachedPlayer.rootPart = character.findFirstChild(XorStr("HumanoidRootPart"), true);
        cachedPlayer.r15 = humanoid.getHumanoidRigType();

        if (GlobalVars::shield_bar)
            cachedPlayer.armor_obj = character.findFirstChild("BodyEffects", false).findFirstChild("Armor", false);

        if ((!GlobalVars::aimbot_checks.empty() && GlobalVars::aimbot_checks[0]) || GlobalVars::aimviewer) {
            cachedPlayer.bodyEffects = character.findFirstChild(XorStr("BodyEffects"), true);
            if (!GlobalVars::aimbot_checks.empty() && GlobalVars::aimbot_checks[0]) {
                cachedPlayer.knockedOut = cachedPlayer.bodyEffects.findFirstChild(XorStr("K.O"), true);
            }
            if (GlobalVars::aimviewer) {
                Roblox::Instance currentTool = character.findFirstChildOfClass(XorStr("Tool"));
                if (currentTool.address != 0) {
                    cachedPlayer.currentTool = currentTool;
                    cachedPlayer.mousePosition = cachedPlayer.bodyEffects.findFirstChild(XorStr("MousePos"), true);
                    cachedPlayer.hasGunEquipped = currentTool.findFirstChild(XorStr("GunScript"), true).address != 0;
                }
            }
        }
        switch (cachedPlayer.r15) {
        case 0:
            cachedPlayer.upperTorso = character.findFirstChild(XorStr("Torso"), true);
            if (GlobalVars::esp) {
                cachedPlayer.leftHand = character.findFirstChild(XorStr("Left Arm"), true);
                cachedPlayer.rightHand = character.findFirstChild(XorStr("Right Arm"), true);
                cachedPlayer.leftFoot = character.findFirstChild(XorStr("Left Leg"), true);
                cachedPlayer.rightFoot = character.findFirstChild(XorStr("Right Leg"), true);
            }
            break;
        case 1:
            cachedPlayer.upperTorso = character.findFirstChild(XorStr("UpperTorso"), true);
            cachedPlayer.lowerTorso = character.findFirstChild(XorStr("LowerTorso"), true);
            if (GlobalVars::esp) {
                cachedPlayer.leftUpperArm = character.findFirstChild(XorStr("LeftUpperArm"), true);
                cachedPlayer.leftHand = character.findFirstChild(XorStr("LeftHand"), true);
                cachedPlayer.rightUpperArm = character.findFirstChild(XorStr("RightUpperArm"), true);
                cachedPlayer.rightHand = character.findFirstChild(XorStr("RightHand"), true);
                cachedPlayer.leftUpperLeg = character.findFirstChild(XorStr("LeftUpperLeg"), true);
                cachedPlayer.leftFoot = character.findFirstChild(XorStr("LeftFoot"), true);
                cachedPlayer.rightUpperLeg = character.findFirstChild(XorStr("RightUpperLeg"), true);;
                cachedPlayer.rightFoot = character.findFirstChild(XorStr("RightFoot"), true);
            }
            break;
        }
        cachedPlayers.push_back(cachedPlayer);
    }
    
    static int last_count = -1;
    if (cachedPlayers.size() != last_count) {
        std::cout << XorStr("[ rbxcache ] cached ") << cachedPlayers.size() << XorStr(" players.\n");
        last_count = cachedPlayers.size();
    }
    
    synchronizeLists(GlobalVars::cached_players, cachedPlayers);
}

std::string toLowerCase(const std::string& str) {
    std::string lower_str;
    std::transform(str.begin(), str.end(), std::back_inserter(lower_str), [](unsigned char c) {
        return std::tolower(c);
        });
    return lower_str;
}

bool caseInsensitiveCompare(const std::string& a, const std::string& b) {
    return toLowerCase(a) < toLowerCase(b);
}

bool Silence::Initialize() {
    SetConsoleTitle(XorStr("gg/silence"));

    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    WORD defaultattrib;
    CONSOLE_SCREEN_BUFFER_INFO cons_info;
    GetConsoleScreenBufferInfo(hConsole, &cons_info);
    defaultattrib = cons_info.wAttributes;

    SetConsoleTextAttribute(hConsole, BACKGROUND_GREEN | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    std::cout << XorStr("[ valid@silence ~ ]");
    SetConsoleTextAttribute(hConsole, defaultattrib);
    std::cout << XorStr(" getting process...\n");

    mem::process_id = 0;
    while (!mem::process_id) {
        mem::process_id = mem::find_process(XorStr("RobloxPlayerBeta.exe"));
        if (!mem::process_id) std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    
    if (!mem::find_driver()) {
        std::perror(XorStr("Unable to open usermode handle to process...\n"));
        std::cin.get();
        return -1;
    }

    SetConsoleTextAttribute(hConsole, BACKGROUND_GREEN | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    std::cout << XorStr("[ valid@silence ~ ]");
    SetConsoleTextAttribute(hConsole, defaultattrib);
    std::cout << XorStr(" initializing...\n");

    std::uint64_t datamodel = 0;
    while (!datamodel) {
        datamodel = get_datamodel();
        if (!datamodel) {
            std::cout << XorStr("[ valid@silence ~ ] waiting for datamodel..      \r");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
    std::cout << XorStr("[ valid@silence ~ ] found datamodel.             \n");

    std::uint64_t renderview = virtualaddy;

    SetConsoleTextAttribute(hConsole, BACKGROUND_GREEN | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    std::cout << XorStr("[ valid@silence ~ ]");
    SetConsoleTextAttribute(hConsole, defaultattrib);
    std::cout << XorStr(" loading components..\n");

    GlobalVars::game = static_cast<Roblox::Instance>(datamodel);
    std::cout << XorStr("[ valid@silence ~ ] found game instance.         \n");

    while (GlobalVars::visualengine.address == 0) {
        uintptr_t static_ptr = read<std::uint64_t>(mem::find_image() + Offsets::VisualEngine::Pointer);
        if (static_ptr != 0) {
            GlobalVars::visualengine.address = static_ptr;
        } else if (renderview != 0) {
            GlobalVars::visualengine.address = read<std::uint64_t>(renderview + Offsets::RenderView::VisualEngine);
        }

        if (GlobalVars::visualengine.address == 0) {
            std::cout << XorStr("[ valid@silence ~ ] waiting for visualengine..      \n");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
    std::cout << XorStr("[ valid@silence ~ ] found visualengine: ") << std::hex << GlobalVars::visualengine.address << std::dec << "\n";
    
    while (GlobalVars::workspace.address == 0) {
        GlobalVars::workspace = GlobalVars::game.findFirstChild(XorStr("Workspace"), false);
        if (GlobalVars::workspace.address == 0) {
            std::cout << XorStr("[ valid@silence ~ ] waiting for workspace..      \n");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
    std::cout << XorStr("[ valid@silence ~ ] found workspace.             \n");

    Roblox::Instance camera_inst = GlobalVars::workspace.findFirstChild(XorStr("Camera"), false);
    if (camera_inst.address != 0) {
        GlobalVars::camera_fieldOfView = camera_inst.getCameraFOV() * GlobalVars::camera_fieldOfViewScaleFactor;
    } else {
        std::cout << XorStr("[ valid@silence ~ ] warning: camera not found, using default FOV.\n");
        GlobalVars::camera_fieldOfView = 70.0f;
    }

    GlobalVars::mouse_service = GlobalVars::game.findFirstChild(XorStr("MouseService"), false);

    while (GlobalVars::players.address == 0) {
        GlobalVars::players = GlobalVars::game.findFirstChild(XorStr("Players"), false);
        if (GlobalVars::players.address == 0) {
            std::cout << XorStr("[ valid@silence ~ ] waiting for players service..\r");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    }
    std::cout << XorStr("[ valid@silence ~ ] found players service.       \n");

    auto players = GlobalVars::players;

    SetConsoleTextAttribute(hConsole, BACKGROUND_GREEN | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    std::cout << XorStr("[ valid@silence ~ ]");
    SetConsoleTextAttribute(hConsole, defaultattrib);
    std::cout << XorStr(" getting players. (this might take a second)..\n");

    while (true) {

        std::vector<int> playersVal;
        for (int i = 0; i < 30; i++) {
            int p = players.ReadPlayer(players.address);
            if (p != 0) {
                playersVal.push_back(p);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }

        if (!playersVal.empty()) {
            std::unordered_map<int, int> countMap;
            int maxCount = 0;

            for (int num : playersVal) {
                if (++countMap[num] > maxCount) {
                    maxCount = countMap[num];
                    GlobalVars::mostFreq = num;
                }
            }
            break; // found players, break out of the infinite loop
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Wait before trying again
    }

    SetConsoleTextAttribute(hConsole, BACKGROUND_GREEN | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    std::cout << XorStr("[ valid@silence ~ ]");
    SetConsoleTextAttribute(hConsole, defaultattrib);
    std::cout << XorStr(" almost done..\n");

    std::thread(Roblox::hook_aimbot).detach();
    std::thread(Roblox::hook_exploits).detach();
    std::thread(Roblox::hook_triggerbot).detach();
    std::thread(Overlay::Render).detach();

    Roblox::Instance localplayer_inst;
    while (localplayer_inst.address == 0) {
        localplayer_inst = GlobalVars::players.getLocalEntity();
        if (localplayer_inst.address == 0) {
            std::cout << XorStr("[ valid@silence ~ ] waiting for local player..   \n");
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }

    Roblox::Instance humanoid;
    while (humanoid.address == 0) {
        humanoid = localplayer_inst.getEntityModelInstance().findFirstChild(XorStr("Humanoid"), true);
        if (humanoid.address == 0) {
            std::cout << XorStr("[ valid@silence ~ ] waiting for humanoid..       \n");
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }

    GlobalVars::walkspeed_val = read<float>(humanoid.address + Offsets::Humanoid::Walkspeed);
    GlobalVars::jumppower_val = read<float>(humanoid.address + Offsets::Humanoid::WalkspeedCheck);
    GlobalVars::hipheight_val = read<float>(humanoid.address + Offsets::Humanoid::HipHeight);
    GlobalVars::original_hipheight = GlobalVars::hipheight_val;

    std::cout << XorStr("[ valid@silence ~ ] successfully initialized.\n");

    Roblox::Instance character_l = localplayer_inst.getEntityModelInstance();

    while (true) {
        rbxcache::cache_entities();
        
        Roblox::Instance character = GlobalVars::players.getLocalEntity().getEntityModelInstance();
        if (character.address != 0) {
            auto lplrchildren = character.getInstancesChildren(true);
            for (auto child : lplrchildren) {
                if (child.getInstanceClass() == XorStr("MeshPart") || child.getInstanceClass() == XorStr("Part") || child.getInstanceClass() == XorStr("BasePart")) {
                    child.setCanCollide(false);
                }
            }
        }

        if (GlobalVars::nojump_cooldown && !GlobalVars::jumppower) {
            // Re-find humanoid to ensure it's still valid
            Roblox::Instance current_humanoid = GlobalVars::players.getLocalEntity().getEntityModelInstance().findFirstChild(XorStr("Humanoid"), true);
            if (current_humanoid.address != 0) {
                Roblox::Instance currentTool = GlobalVars::localplayer.currentTool;
                if (currentTool.getInstanceName() == XorStr("[Knife]")) {
                    current_humanoid.writeEntityJumpPower(59.5);
                }
                else {
                    current_humanoid.writeEntityJumpPower(50.0);
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        if (FindWindowA("WINDOWSCLIENT", NULL) == NULL) {
            exit(0);
        }
    }

    return 0;
}


