#pragma once
#include <string>
#include <vector>
#include <cmath>
#include <cstdint>

inline std::vector<std::string> g_whitelisted_players{};

namespace Silence {
	bool Initialize();
}

struct PartState {
	std::string name;
	bool ignore;
};

namespace Roblox
{
	struct Vector2 final { 
		const Vector2 operator-(const Vector2& other) const noexcept
		{
			return Vector2{ x - other.x, y - other.y};
		}

		const Vector2 operator+(const Vector2& other) const noexcept
		{
			return Vector2{ x + other.x, y + other.y};
		}

		const Vector2 operator/(const float factor) const noexcept
		{
			return Vector2{ x / factor, y / factor};
		}

		const Vector2 operator/(const Vector2& factor) const noexcept
		{
			return Vector2{ x / factor.x, y / factor.y};
		}

		const Vector2 operator*(const float factor) const noexcept
		{
			return Vector2{ x * factor, y * factor};
		}

		const Vector2 operator*(const Vector2& factor) const noexcept
		{
			return Vector2{ x * factor.x, y * factor.y};
		}

		const float getMagnitude() const noexcept
		{
			return sqrtf((x * x) + (y * y));
		}
		
		float x, y; 
	};
	struct Vector3 final {
		void setPartVelocity(Roblox::Vector3 Velocity);

		// -

		const Vector3 operator-(const Vector3& other) const noexcept {
			return Vector3{ x - other.x, y - other.y, z - other.z };
		}

		const Vector3 operator-() const noexcept {
			return Vector3{ -x, -y, -z };
		}

		const Vector3 operator-(const float factor) const noexcept
		{
			return Vector3{ x - factor, y - factor, z - factor };
		}

		// +

		const Vector3 operator+(const Vector3& other) const noexcept
		{
			return Vector3{ x + other.x, y + other.y, z + other.z };
		}

		const Vector3 operator+(const float factor) const noexcept
		{
			return Vector3{ x + factor, y + factor, z + factor };
		}

		// /

		const Vector3 operator/(const Vector3& factor) const noexcept
		{
			return Vector3{ x / factor.x, y / factor.y, z / factor.z };
		}

		const Vector3 operator/(const float factor) const noexcept
		{
			return factor != 0 ? Vector3{ x / factor, y / factor, z / factor } : Vector3{ 0, 0, 0 };
		}

		// *

		const Vector3 operator*(const Vector3& factor) const noexcept
		{
			return Vector3{ x * factor.x, y * factor.y, z * factor.z };
		}

		friend Vector3 operator*(float factor, const Vector3& vec) noexcept {
			return Vector3{ vec.x * factor, vec.y * factor, vec.z * factor };
		}

		// Scalar multiplication (Vector3 * float)
		const Vector3 operator*(float factor) const noexcept {
			return Vector3{ x * factor, y * factor, z * factor };
		}

		const bool operator==(const Vector3& other) const noexcept
		{
			return x == other.x && y == other.y && z == other.z;
		}

		const bool operator!=(const Vector3& other) const noexcept
		{
			return x != other.x || y != other.y || z != other.z;
		}

		// Functions

		const Vector3 negate() const noexcept
		{
			return Vector3{ -x, -y, -z };
		}

		const float getMagnitude() const noexcept
		{
			return sqrtf(x * x + y * y + z * z);
		}

		const Vector3 normalize() const noexcept
		{
			float lengthSquared = x * x + y * y + z * z;
			if (lengthSquared > 0) {
				float invLength = 1.0f / sqrtf(lengthSquared);
				return { x * invLength, y * invLength, z * invLength };
			}
			return *this; // Return original vector if length is zero
		}

		auto cross(Vector3 vec) const
		{
			Vector3 ret;
			ret.x = y * vec.z - z * vec.y;
			ret.y = -(x * vec.z - z * vec.x);
			ret.z = x * vec.y - y * vec.x;
			return ret;
		}

		float x, y, z;
		
	};

	struct Vector4 final { float x, y, z, w; };
	struct Matrix4x4 final { float data[16]; };
	struct Matrix3x3 final { float data[9]; };

	struct CFrame
	{
		Vector3 right_vector = { 1, 0, 0 };
		Vector3 up_vector = { 0, 1, 0 };
		Vector3 back_vector = { 0, 0, 1 };
		Vector3 position = { 0, 0, 0 };

		CFrame() = default;
		CFrame(Vector3 position) : position{ position } {}
		CFrame(Vector3 right_vector, Vector3 up_vector, Vector3 back_vector, Vector3 position)
			: right_vector{ right_vector }, up_vector{ up_vector }, back_vector{ back_vector }, position{ position } {}

		CFrame look_at(Vector3 point) noexcept
		{
			Vector3 look_vector = (position - point).normalize();
			Vector3 right_vector = Vector3(0, 1, 0).cross(look_vector).normalize();
			Vector3 up_vector = look_vector.cross(right_vector).normalize();

			return CFrame{ right_vector, up_vector, look_vector, position };
		}

		CFrame operator*(CFrame cframe) const noexcept
		{
			CFrame ret;

			ret.right_vector = Vector3{
				right_vector.x * cframe.right_vector.x + right_vector.y * cframe.up_vector.x + right_vector.z * cframe.back_vector.x,
				right_vector.x * cframe.right_vector.y + right_vector.y * cframe.up_vector.y + right_vector.z * cframe.back_vector.y,
				right_vector.x * cframe.right_vector.z + right_vector.y * cframe.up_vector.z + right_vector.z * cframe.back_vector.z
			};
			ret.up_vector = Vector3{
				up_vector.x * cframe.right_vector.x + up_vector.y * cframe.up_vector.x + up_vector.z * cframe.back_vector.x,
				up_vector.x * cframe.right_vector.y + up_vector.y * cframe.up_vector.y + up_vector.z * cframe.back_vector.y,
				up_vector.x * cframe.right_vector.z + up_vector.y * cframe.up_vector.z + up_vector.z * cframe.back_vector.z
			};
			ret.back_vector = Vector3{
				back_vector.x * cframe.right_vector.x + back_vector.y * cframe.up_vector.x + back_vector.z * cframe.back_vector.x,
				back_vector.x * cframe.right_vector.y + back_vector.y * cframe.up_vector.y + back_vector.z * cframe.back_vector.y,
				back_vector.x * cframe.right_vector.z + back_vector.y * cframe.up_vector.z + back_vector.z * cframe.back_vector.z
			};
			ret.position = Vector3{
				right_vector.x * cframe.position.x + right_vector.y * cframe.position.y + right_vector.z * cframe.position.z + position.x,
				up_vector.x * cframe.position.x + up_vector.y * cframe.position.y + up_vector.z * cframe.position.z + position.y,
				back_vector.x * cframe.position.x + back_vector.y * cframe.position.y + back_vector.z * cframe.position.z + position.z
			};

			return ret;
		}

		Vector3 operator*(Vector3 vec) const noexcept
		{
			return Vector3{
				right_vector.x * vec.x + right_vector.y * vec.y + right_vector.z * vec.z + position.x,
				up_vector.x * vec.x + up_vector.y * vec.y + up_vector.z * vec.z + position.y,
				back_vector.x * vec.x + back_vector.y * vec.y + back_vector.z * vec.z + position.z
			};
		}
	};

	class Instance final
	{
	public:

		std::uint64_t address = 0;

		// Informations
		std::string getInstanceName() const;
		std::string getInstanceDisplayName();
		std::string getInstanceClass();

		// Children / Parents
		std::vector<Roblox::Instance> getInstancesChildren(bool isHumanoid);
		Roblox::Instance getInstanceParent();
		Roblox::Instance findFirstChild(std::string child, bool isCharChild);
		Roblox::Instance findFirstChildOfClass(std::string child);
		int ReadPlayer(std::uint64_t address);

		// Screen
		Roblox::Vector2 getScreenDimensions();
		Roblox::Matrix4x4 getViewMatrix();

		// Entity
		Roblox::Instance getLocalEntity();
		Roblox::Instance getEntityModelInstance();
		Roblox::Instance getModelInstancePrimaryPart();
		std::int32_t getHumanoidRigType();
		Roblox::Instance getEntityTeam();

		float getHumanoidHealth();
		float getHumanoidMaxHealth();

		void writeEntityHipHeight(float HipHeight);
		void writeEntityJumpPower(float JumpPower);
		void writeEntityWalkSpeed(float WalkSpeed);

		void setPartCFrame(const CFrame& cf);

		// Camera
		Roblox::Instance getCamera();
		Roblox::Vector3 getCameraPosition();
		Roblox::Matrix3x3 getCameraRotation();
		float getCameraFOV();

		void writeCameraRotation(Roblox::Matrix3x3 Rotation);
		void writeCameraFieldOfView(float fieldOfView);

		static void initializeMouseService(uint64_t address);
		static void writeMousePosition(float x, float y);

		// Part
		void setAnchored(bool value);
		bool setCanCollide(bool value);
		std::uint64_t getPartPrimitive();
		Roblox::Vector3 getPartPosition() const;
		Roblox::Vector3 getPartVelocity();
		Roblox::Matrix3x3 getPartRotation();
		Roblox::Vector3 getPartSize() const;
		std::string getAnchor();

		Roblox::CFrame getPartCFrame();

		void setPartPosition(Roblox::Vector3 Position);

		// Value
		bool getBoolFromValue();
		Roblox::Vector3 getVec3FromValue();

	private:
		static std::uint64_t cached_input_object;
	};

	class Entity final
	{
	public:
		bool operator==(const Entity& other) const {
			return address == other.address;
		}



		std::uint64_t address;

		// Informations
		std::string name;
		std::string displayName;
		Roblox::Instance team;
		Roblox::Instance character;
		int r15;

		// Da Hood Exclusive
		Roblox::Instance bodyEffects;
		Roblox::Instance knockedOut;
		Roblox::Instance mousePosition;
		Roblox::Instance currentTool;
		Roblox::Instance armor_obj;
		Roblox::Instance grabbing_constraint;
		bool hasGunEquipped = false;

		// Parts
		Roblox::Instance humanoid;
		Roblox::Instance hrpInstance;
		Roblox::Instance head;
		Roblox::Instance rootPart;
		Roblox::Instance upperTorso;
		Roblox::Instance lowerTorso;
		Roblox::Instance leftUpperLeg;
		Roblox::Instance leftFoot;
		Roblox::Instance rightUpperLeg;
		Roblox::Instance rightFoot;
		Roblox::Instance leftUpperArm;
		Roblox::Instance leftHand;
		Roblox::Instance rightUpperArm;
		Roblox::Instance rightHand;

	};

	Roblox::Vector2 worldToScreen(Roblox::Vector3 world, Roblox::Vector2 dimensions, Roblox::Matrix4x4 viewmatrix);

}