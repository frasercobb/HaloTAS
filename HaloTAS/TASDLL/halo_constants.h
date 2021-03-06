#pragma once

#define HALO_VANILLA
//#define HALO_CUSTOMED

#define SyncBin PlaySound

#if defined(HALO_VANILLA) && defined(HALO_CUSTOMED)
#error "Don't define HALO_VANILLA and HALO_CUSTOMED at the same time."
#endif

#if !defined(HALO_VANILLA) && !defined(HALO_CUSTOMED)
#error "Either HALO_VANILLA or HALO_CUSTOMED has to be defined."
#endif

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <boost/functional/hash.hpp>
#include <type_traits>

struct object_pool_entry
{
	uint16_t id;
	uint16_t unk1;
	uint16_t unk2;
	uint16_t size;
	uint32_t object_address;
};

inline extern int8_t MAGIC_DATAPOOLHEADER[] = {/* 0x00, 0x08, 0x0C, 0x00, 0x01, 0x00, 0x00, 0x00,*/ 0x40, 0x74, 0x40, 0x64 };
struct object_pool_header {
	char name[32];
	uint16_t max_objects; // 0x800
	uint16_t object_table_size; // 0xC
	uint32_t unknown1; // 00000001
	uint32_t signature; // 0x64407440 @t@d
	int16_t objects;
	int16_t max_object_count;
	int16_t current_objects;
	int16_t next_object_index;
	object_pool_entry* object_data_begin;
};

struct Tag {
	uint32_t id;
	glm::vec3 displayColor;
	std::string displayName;
};

// Utility to get an enum value as the underlying type
template <typename E>
constexpr auto to_underlying(E e) noexcept
{
	return static_cast<std::underlying_type_t<E>>(e);
}

// Patch DirectInput code to allow for editing of mouse x/y values while the game is not in focus
// TODO: Replace this with Detours hook when possible
static uint8_t PATCH_DINPUT_MOUSE_BYTES[] = { 0x90,0x90,0x90,0x90,0x90,0x90,0x90 };
static uint8_t PATCH_DINPUT_MOUSE_ORIGINAL[] = { 0x52,0x6A,0x14,0x50,0xFF,0x51,0x24 };
inline extern uint8_t* PATCH_DINPUT_MOUSE_FUNC = reinterpret_cast<uint8_t*>(0x00490910);

enum class HUD_LOCATION : int16_t
{
	TOP_LEFT = 0,
	TOP_RIGHT = 1,
	BOTTOM_LEFT = 2,
	BOTTOM_RIGHT = 3
};

namespace halo::constants {
	static const float CAMERA_PITCH_MIN = -1.492f;
	static const float CAMERA_PITCH_MAX = 1.492f;
	static const float CAMERA_YAW_MIN = 0.0f;
	static const float CAMERA_YAW_MAX = glm::pi<float>() * 2.0f;

	static uint8_t PATCH_CUTSCENE_FPS_CAP[]= { 0xEB };
	static uint8_t PATCH_CUTSCENE_FPS_CAP_ORIGINAL[] = {0x74};

	static uint8_t PATCH_LOOK_CENTERING[] = { 0x90, 0x90 };
	static uint8_t PATCH_LOOK_CENTERING_ORIGINAL[] = {0x7A, 0x0B};

	static size_t CORE_SAVE_SIZE = 0x440000;
}

#if defined(HALO_VANILLA)

namespace halo::function {
	inline extern uintptr_t PRINT_HUD = 0x004AE180; // (int playerIndex, wchar_t* string)
	inline extern uintptr_t GET_TAG_INDEX = 0x00442550; // (int tagTypeIdentifier, char* pathString)
	inline extern uintptr_t EXECUTE_COMMAND = 0x004C6A80; // (const char* command)

	inline extern uintptr_t PLAY_SOUND_PTR = 0x00549af0;
	typedef uint32_t play_sound_actual_func(uint32_t sndIndex, int16_t*, int, int, int*, int, int);
	inline extern play_sound_actual_func* PLAY_SOUND = (play_sound_actual_func*)PLAY_SOUND_PTR;
}


namespace halo::addr {
	inline extern uint32_t* RUNTIME_DATA_BEGIN = reinterpret_cast<uint32_t*>(0x40000000);
	inline extern uint32_t* TAGS_BEGIN = reinterpret_cast<uint32_t*>(0x40440000);
	inline extern object_pool_header* OBJ_POOL_HEADER = reinterpret_cast<object_pool_header*>(0x400506B4);

	inline extern int32_t* FRAMES_SINCE_LEVEL_START_ANIMATION = reinterpret_cast<int32_t*>(0x00746F88);
	inline extern int32_t* FRAMES_SINCE_LEVEL_START = reinterpret_cast<int32_t*>(0x008603CC);
	inline extern int32_t* FRAMES_ABSOLUTE = reinterpret_cast<int32_t*>(0x007C3100);
	inline extern int32_t* FRAMES_ABSOLUTE_ALTERNATE = reinterpret_cast<int32_t*>(0x007C3104);
	inline volatile extern int32_t* SIMULATION_TICK = reinterpret_cast<int32_t*>(0x400002F4);
	inline volatile extern int32_t* SIMULATION_TICK_ALTERNATE = reinterpret_cast<int32_t*>(0x400002FC);

	inline extern uint8_t* LOAD_CHECKPOINT = reinterpret_cast<uint8_t*>(0x71973A);
	inline extern uint8_t* SAVE_CHECKPOINT = reinterpret_cast<uint8_t*>(0x71973F);
	inline extern uint8_t* MAP_RESET = reinterpret_cast<uint8_t*>(0x719738);
	inline extern uint8_t* RESTART_WITH_INITIAL_LOAD = reinterpret_cast<uint8_t*>(0x719739);
	inline extern uint8_t* CORE_SAVE = reinterpret_cast<uint8_t*>(0x719751);
	inline extern uint8_t* CORE_LOAD = reinterpret_cast<uint8_t*>(0x719752);
	inline extern uint8_t* GAME_WON = reinterpret_cast<uint8_t*>(0x71974E);

	inline extern uint8_t* GAME_IS_RUNNING = reinterpret_cast<uint8_t*>(0x400002E9);
	inline extern uint8_t* GAME_IS_PAUSED = reinterpret_cast<uint8_t*>(0x400002EA);
	inline extern float* LEFTRIGHTVIEW = reinterpret_cast<float*>(0x402AD4B8);
	inline extern float* UPDOWNVIEW = reinterpret_cast<float*>(0x402AD4BC);
	inline extern char* MAP_STRING = reinterpret_cast<char*>(0x40000004);
	inline extern uint8_t* CURRENT_BSP_INDEX = reinterpret_cast<uint8_t*>(0x69E8D8);
	inline extern uint32_t* CHECKPOINT_INDICATOR = reinterpret_cast<uint32_t*>(0x00746F90);
	inline extern uint8_t* KEYBOARD_INPUT = reinterpret_cast<uint8_t*>(0x006B1620);
	inline extern uint8_t* LEFTMOUSE = reinterpret_cast<uint8_t*>(0x006B1818);
	inline extern uint8_t* MIDDLEMOUSE = reinterpret_cast<uint8_t*>(0x006B1819);
	inline extern uint8_t* RIGHTMOUSE = reinterpret_cast<uint8_t*>(0x006B181A);
	inline extern bool* ALLOW_SIMULATE = reinterpret_cast<bool*>(0x00721E8C);
	inline extern bool* ALLOW_INPUT = reinterpret_cast<bool*>(0x006B15F8);
	inline extern uint8_t* ENGINE_RENDER_ENABLE = reinterpret_cast<uint8_t*>(0x00719AA8);
	inline extern int32_t* DINPUT_MOUSEX = reinterpret_cast<int32_t*>(0x006B180C);
	inline extern int32_t* DINPUT_MOUSEY = reinterpret_cast<int32_t*>(0x006B1810);
	inline extern int32_t* DINPUT_MOUSEZ = reinterpret_cast<int32_t*>(0x006B1814); // Scroll
	inline extern char* INPUT_SLOT = reinterpret_cast<char*>(0x400003C4);
	inline extern uint8_t* SOUND_ENABLED = reinterpret_cast<uint8_t*>(0x00725201);
	inline extern uint8_t* CUTSCENE_FPS_CAP_PATCH = reinterpret_cast<uint8_t*>(0x004C9FB3); // 0x74 Default, 0xEB No Cap
	inline extern uint8_t* LOOK_CENTERING_PATCH = reinterpret_cast<uint8_t*>(0x0047254A);

	// HUD Stuff
	inline extern bool* HUD_TIMER_PAUSED = reinterpret_cast<bool*>(0x40000846);
	inline extern bool* HUD_TIMER_VISIBLE = reinterpret_cast<bool*>(0x40000847);
	inline extern int32_t* HUD_TIMER_START_TICK = reinterpret_cast<int32_t*>(0x40000838);
	inline extern int32_t* HUD_TIMER_TOTAL_TIME_TICKS = reinterpret_cast<int32_t*>(0x4000083C);
	inline extern int16_t* HUD_TIMER_OFFSET_X = reinterpret_cast<int16_t*>(0x40000840);
	inline extern int16_t* HUD_TIMER_OFFSET_Y = reinterpret_cast<int16_t*>(0x40000842);
	inline extern HUD_LOCATION* HUD_TIMER_LOCATION = reinterpret_cast<HUD_LOCATION*>(0x40000844);
	inline extern float* HUD_PLAYER_SHIELD = reinterpret_cast<float*>(0x40000848);
	inline extern float* HUD_PLAYER_HEALTH = reinterpret_cast<float*>(0x4000084C);
	inline extern uint8_t* HUD_FLAGS = reinterpret_cast<uint8_t*>(0x400008A0);
	inline extern bool* HUD_ENABLED = reinterpret_cast<bool*>(0x400003BC);

	inline extern int32_t* FAST_FORWARD_POINTER = reinterpret_cast<int32_t*>(0x00470c03);

	inline extern glm::vec3* CAMERA_POSITION = reinterpret_cast<glm::vec3*>(0x006AC6D0);
	inline extern float* CAMERA_LOOK_VECTOR = reinterpret_cast<float*>(0x006AC72C);
	inline extern float** PTR_TO_CAMERA_HORIZONTAL_FIELD_OF_VIEW_IN_RADIANS = reinterpret_cast<float**>(0x00445920);
	inline extern float* PLAYER_YAW_ROTATION_RADIANS = reinterpret_cast<float*>(0x402AD4B8);
	inline extern float* PLAYER_PITCH_ROTATION_RADIANS = reinterpret_cast<float*>(0x402AD4BC);
	inline extern uint8_t* DEBUG_CAMERA_ENABLE = reinterpret_cast<uint8_t*>(0x006AC568);
	//inline extern uint8_t* debugB = reinterpret_cast<uint8_t*>(0x006AC569);

	inline extern float* GAME_SPEED = reinterpret_cast<float*>(0x40000300);
	inline extern int32_t* RNG = reinterpret_cast<int32_t*>(0x00719CD0);
	inline extern char* GAME_VERSION = reinterpret_cast<char*>(0x40000104);
	inline extern uint8_t* GAME_DIFFICULTY_ACTUAL = reinterpret_cast<uint8_t*>(0x40000126);
	inline extern uint8_t* GAME_DIFFICULTY_ACTUAL_ALTERNATE = reinterpret_cast<uint8_t*>(0x400001E2);
	inline extern uint8_t* GAME_DIFFICULTY_ON_NEXT_MAP_LOAD = reinterpret_cast<uint8_t*>(0x696564);
}

#elif defined(HALO_CUSTOMED)

inline extern uint32_t* ADDR_TAGS_ARRAY = (uint32_t*)0x40000000;
inline extern uint32_t TAG_ARRAY_LENGTH_BYTES = 0x440000;

inline extern int32_t* ADDR_FRAMES_SINCE_LEVEL_START_ANIMATION = (int32_t*)0x00746F88;
inline extern int32_t* ADDR_INPUT_TICK = (int32_t*)0x006F1D8C;
inline extern float* ADDR_LEFTRIGHTVIEW = (float*)0x402AD4B8;
inline extern float* ADDR_UPDOWNVIEW = (float*)0x402AD4BC;
inline extern char* ADDR_MAP_STRING = (char*)0x006A8174;
inline extern uint32_t* ADDR_CHECKPOINT_INDICATOR = (uint32_t*)0x00746F90;
inline extern uint8_t* ADDR_KEYBOARD_INPUT = (uint8_t*)0x006B1620;
inline extern uint8_t* ADDR_LEFTMOUSE = (uint8_t*)0x006B1818;
inline extern uint8_t* ADDR_MIDDLEMOUSE = (uint8_t*)0x006B1819;
inline extern uint8_t* ADDR_RIGHTMOUSE = (uint8_t*)0x006B181A;
inline extern bool* ADDR_SIMULATE = (bool*)0x006BD15C;
inline extern bool* ADDR_ALLOW_INPUT = (bool*)0x0064C528;

inline extern int32_t* ADDR_DINPUT_MOUSEX = (int32_t*)0x006B180C;
inline extern int32_t* ADDR_DINPUT_MOUSEY = (int32_t*)0x006B1810;
inline extern int32_t* ADDR_DINPUT_MOUSEZ = (int32_t*)0x006B1814; // Scroll

// Patch point for allowing external directinput mouse movement
inline extern uint8_t* ADDR_PATCH_DINPUT_MOUSE = (uint8_t*)0x00490910;
inline extern float* ADDR_CAMERA_POSITION = (float*)0x00647600;
inline extern float* ADDR_CAMERA_LOOK_VECTOR = (float*)0x0064765C;
inline extern float** ADDR_PTR_TO_CAMERA_HORIZONTAL_FIELD_OF_VIEW_IN_RADIANS = (float**)0x00446280;

inline extern float* ADDR_GAME_SPEED = (float*)0x40000300;

#endif

namespace halo {
	enum class MAP {
		PILLAR_OF_AUTUMN,
		HALO,
		TRUTH_AND_RECONCILIATION,
		SILENT_CARTOGRAPHER,
		ASSAULT_ON_THE_CONTROL_ROOM,
		_343_GUILTY_SPARK,
		LIBRARY,
		TWO_BETRAYALS,
		KEYES,
		MAW,
		UI_MAIN_MENU,
		UNKNOWN_MAP
	};

	enum class DIFFICULTY : uint8_t {
		EASY,
		NORMAL,
		HEROIC,
		LEGENDARY,
		INVALID_DIFFICULTY
	};

	// KEYS is a layout of the keyboard keys in Halo's memory (one unsigned byte per key)
	// When held, each key increments by 1 for each frame up to the max of 255
	enum class KEYS : uint8_t {
		ESC = 0,
		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,
		PrntScrn,
		ScrollLock,
		PauseBreak,
		Tilde,
		N_1,
		N_2,
		N_3,
		N_4,
		N_5,
		N_6,
		N_7,
		N_8,
		N_9,
		N_0,
		Minus,
		Equal,
		Backspace,
		Tab,
		Q,
		W,
		E,
		R,
		T,
		Y,
		U,
		I,
		O,
		P,
		LeftBracket,
		RightBracket,
		Pipe,
		CapsLock,
		A,
		S,
		D,
		F,
		G,
		H,
		J,
		K,
		L,
		Colon,
		Quote,
		Enter,
		LShift,
		Z,
		X,
		C,
		V,
		B,
		N,
		M,
		Comma,
		Period,
		ForwardSlash,
		RShift,
		LCtrl,
		LWin,
		LAlt,
		Space,
		RAlt,
		RWin,
		KeyThatLiterallyNoOneHasEverUsed,
		RCtrl,
		Up,
		Down,
		Left,
		Right,
		Insert,
		Home,
		PageUp,
		Delete,
		End,
		PageDown,
		NumLock,
		NUMPAD_Div,
		NUMPAD_Mul,
		NUMPAD_0,
		NUMPAD_1,
		NUMPAD_2,
		NUMPAD_3,
		NUMPAD_4,
		NUMPAD_5,
		NUMPAD_6,
		NUMPAD_7,
		NUMPAD_8,
		NUMPAD_9,
		NUMPAD_Minus,
		NUMPAD_Plus,
		NUMPAD_Enter,
		NUMPAD_Period,

		KEY_COUNT
	};

	// A subset of KEYS that only contains the most commonly used keys

	const std::vector<KEYS> COMMON_KEYS {
		KEYS::W,
		KEYS::A,
		KEYS::S,
		KEYS::D,
		KEYS::E,
		KEYS::R,
		KEYS::F,
		KEYS::G,
		KEYS::Z,
		KEYS::Tab,
		KEYS::Space,
		KEYS::LCtrl,
		KEYS::LShift,
		KEYS::Q
	};

	enum class COMMON_KEYS : uint8_t {
		W = to_underlying(KEYS::W),
		A = to_underlying(KEYS::A),
		S = to_underlying(KEYS::S),
		D = to_underlying(KEYS::D),

		E = to_underlying(KEYS::E),
		R = to_underlying(KEYS::R),
		F = to_underlying(KEYS::F),
		G = to_underlying(KEYS::G),
		Z = to_underlying(KEYS::Z),
		Tab = to_underlying(KEYS::Tab),
		Space = to_underlying(KEYS::Space),
		LCtrl = to_underlying(KEYS::LCtrl),
		LShift = to_underlying(KEYS::LShift),
		Q = to_underlying(KEYS::Q),
	};

	// Text to display for KEYS codes
	inline extern const std::string KEYS_TO_STRING[] = {
		"ESC",
		"F1",
		"F2",
		"F3",
		"F4",
		"F5",
		"F6",
		"F7",
		"F8",
		"F9",
		"F10",
		"F11",
		"F12",
		"PrntScrn",
		"ScrollLock",
		"PauseBreak",
		"Tilde",
		"1",
		"2",
		"3",
		"4",
		"5",
		"6",
		"7",
		"8",
		"9",
		"0",
		"Minus",
		"Equal",
		"Backspace",
		"Tab",
		"Q",
		"W",
		"E",
		"R",
		"T",
		"Y",
		"U",
		"I",
		"O",
		"P",
		"[",
		"]",
		"|",
		"CapsLock",
		"A",
		"S",
		"D",
		"F",
		"G",
		"H",
		"J",
		"K",
		"L",
		"Colon",
		"Quote",
		"Enter",
		"LShift",
		"Z",
		"X",
		"C",
		"V",
		"B",
		"N",
		"M",
		",",
		".",
		"/",
		"RShift",
		"LCtrl",
		"LWin",
		"LAlt",
		"Space",
		"RAlt",
		"RWin",
		"Menu",
		"RCtrl",
		"Up",
		"Down",
		"Left",
		"Right",
		"Insert",
		"Home",
		"PageUp",
		"Delete",
		"End",
		"PageDown",
		"NumLock",
		"NUM_/",
		"NUM_*",
		"NUM_0",
		"NUM_1",
		"NUM_2",
		"NUM_3",
		"NUM_4",
		"NUM_5",
		"NUM_6",
		"NUM_7",
		"NUM_8",
		"NUM_9",
		"NUM_-",
		"NUM_+",
		"NUM_ENTER",
		"NUM_."
	};

	const std::unordered_map<std::pair<std::string, uint8_t>, std::string, boost::hash<std::pair<std::string, uint8_t>>> LEVEL_BSP_NAME{
			{{"levels\\a10\\a10", 0}, "a10a"},
			{{"levels\\a10\\a10", 1}, "a10b"},
			{{"levels\\a10\\a10", 2}, "a10c"},
			{{"levels\\a10\\a10", 3}, "a10d"},
			{{"levels\\a10\\a10", 4}, "a10e"},
			{{"levels\\a10\\a10", 5}, "a10f"},
			{{"levels\\a10\\a10", 6}, "a10g"},
			{{"levels\\a10\\a10", 7}, "a10_space"},
			{{"levels\\a10\\a10", 8}, "x10hangar"},

			{{"levels\\a30\\a30", 0}, "a30_a"},
			{{"levels\\a30\\a30", 1}, "a30_b"},

			{{"levels\\a50\\a50", 0}, "a50_exterior"},
			{{"levels\\a50\\a50", 1}, "a50_muster"},
			{{"levels\\a50\\a50", 2}, "a50_hangar"},
			{{"levels\\a50\\a50", 3}, "a50_control"},

			{{"levels\\b30\\b30", 0}, "b30a"},
			{{"levels\\b30\\b30", 1}, "b30b"},

			{{"levels\\c20\\c20", 0}, "c20_1"},
			{{"levels\\c20\\c20", 1}, "c20_2"},
			{{"levels\\c20\\c20", 2}, "c20_3"},
			{{"levels\\c20\\c20", 3}, "c20_4"},

			{{"levels\\d20\\d20", 0}, "d20_start"},
			{{"levels\\d20\\d20", 1}, "d20_exterior"},
			{{"levels\\d20\\d20", 2}, "d20_muster"},
			{{"levels\\d20\\d20", 3}, "d20_hangar"},
			{{"levels\\d20\\d20", 4}, "d20_control"},

			{{"levels\\d40\\d40", 0}, "d40a"},
			{{"levels\\d40\\d40", 1}, "d40b"},
			{{"levels\\d40\\d40", 2}, "d40c"},
			{{"levels\\d40\\d40", 3}, "d40d"},
			{{"levels\\d40\\d40", 4}, "d40e"},
			{{"levels\\d40\\d40", 5}, "d40f"},
			{{"levels\\d40\\d40", 6}, "d40g"},
			{{"levels\\d40\\d40", 7}, "d40h"},
			{{"levels\\d40\\d40", 8}, "d40_terrain"},


	};

	const std::unordered_map<uint32_t, Tag> KNOWN_TAGS = {
	{ 580, Tag{ 580, glm::vec3(0,1,0), "static prop" } },
	{ 616, Tag{ 616, glm::vec3(0,1,0), "Terminal" }},
	{ 628, Tag{ 628, glm::vec3(0,1,0), "bulkhead?"   }},
	{ 632, Tag{ 632, glm::vec3(0,1,0), "tree"   }},
	{ 680, Tag{ 680, glm::vec3(0,1,0), "animated light"   }},
	{ 720, Tag{ 720, glm::vec3(0,1,0), "debris" } },
	{ 732, Tag{ 732, glm::vec3(0,1,0), "door"   }},
	{ 736, Tag{ 736, glm::vec3(0,1,0), "tree"   }},
	{ 764, Tag{ 764, glm::vec3(0,1,0), "projectile"   }},
	{ 800, Tag{ 800, glm::vec3(0,1,0), "Ammo/Health/Consumable"   }},
	{ 836, Tag{ 836, glm::vec3(0,1,0), "bulkhead?"   }},
	{ 892, Tag{ 892, glm::vec3(0,1,0), "trigger?"   }},
	{ 940, Tag{ 940, glm::vec3(0,1,0), "Door" } },
	{ 972, Tag{ 972, glm::vec3(0,1,0), "Needler" } },
	{ 1088,Tag{ 1088, glm::vec3(0,1,0), "Shotgun"   }},
	{ 1204,Tag{ 1204, glm::vec3(0,1,0), "MA5B"  }},
	{ 1320,Tag{ 1320, glm::vec3(0,1,0), "Plasma Pistol"  }},
	{ 1356,Tag{ 1356, glm::vec3(0,1,0), "POA Terminal"  }},
	{ 1436,Tag{ 1436, glm::vec3(0,1,0), "Plasma Rifle/Rockets"  }},
	{ 1612,Tag{ 1612, glm::vec3(0,1,0), "POA Bridge Chair"  }},
	{ 1616,Tag{ 1616, glm::vec3(0,1,0), "343 GS"  }},
	{ 1668,Tag{ 1668, glm::vec3(0,1,0), "Pistol"  }},
	{ 1728,Tag{ 1728, glm::vec3(0,1,0), "???"  }},
	{ 1960,Tag{ 1960, glm::vec3(0,1,0), "Banshee" } },
	{ 2076,Tag{ 2076, glm::vec3(0,1,0), "Ghost" } },
	{ 2424,Tag{ 2424, glm::vec3(0,1,0), "Wraith" } },
	{ 2888,Tag{ 2888, glm::vec3(0,1,0), "Pelican" } },
	{ 2892,Tag{ 2892, glm::vec3(0,1,0), "Popcorn"  }},
	{ 3356,Tag{ 3356, glm::vec3(0,1,0), "Grunt" } },
	{ 3584,Tag{ 3584, glm::vec3(0,1,0), "Warthog?"  }},
	{ 3588,Tag{ 3588, glm::vec3(0,1,0), "You!"  }},
	{ 3704,Tag{ 3704, glm::vec3(0,1,0), "Marine" } },
	{ 3936,Tag{ 3936, glm::vec3(0,1,0), "Infection Form"  }},
	{ 4400,Tag{ 4400, glm::vec3(0,1,0), "Elite"  }},
	{ 4516,Tag{ 4516, glm::vec3(0,1,0), "Jackal"  }},
	{ 4864,Tag{ 4864, glm::vec3(1,0,1), "Hunter"  }},
	{ 5328,Tag{ 5328, glm::vec3(0,1,0), "Cpt. Keyes" } },
	{ 5792,Tag{ 5792, glm::vec3(0,1,0), "Flood (human)" } },
	{ 6024,Tag{ 6024, glm::vec3(0,1,0), "Flood (elite)" }},
	};
}
