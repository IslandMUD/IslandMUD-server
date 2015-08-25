﻿/* Jim Viebke
Feb 14, 2015 */

#include "constants.h"

// general
const int C::GROUND_INDEX = 3; // there are three levels below this
const int C::VIEW_DISTANCE = 10; // 5+1+5 to a side == 11*11 total area

const int C::WORLD_X_DIMENSION = 1000;
const int C::WORLD_Y_DIMENSION = 1000;
const int C::WORLD_Z_DIMENSION = 8;

const int C::DEFAULT_SPAWN_X = C::WORLD_X_DIMENSION / 2; // N/S center
const int C::DEFAULT_SPAWN_Y = C::WORLD_Y_DIMENSION / 2; // E/W center
const int C::DEFAULT_SPAWN_Z = C::GROUND_INDEX; // ground level spawn

// game data locations
const string C::game_directory = "C:/IslandMUD";
const string C::world_terrain_file_location = C::game_directory + "/world_terrain.txt";
const string C::world_biome_file_location = C::game_directory + "/biome_map.txt";
const string C::room_directory = C::game_directory + "/rooms";
const string C::user_data_directory = C::game_directory + "/user_data";

// faction IDs
const string C::PC_FACTION_ID = "player";
const string C::NPC_NEUTRAL_FACTION_ID = "neutral_NPC";
const string C::NPC_HOSTILE_FACTION_ID = "hostile_NPC";

// map symbols
const char C::PLAYER_CHAR = '@';
const char C::NPC_NEUTRAL_CHAR = '+';
const char C::WATER_CHAR = '~';
const char C::LAND_CHAR = ' ';
const char C::FOREST_CHAR = '%';
const char C::ITEM_CHAR = '?';
const char C::CHEST_CHAR = char(220); // ascii 220
const char C::RUBBLE_CHAR = '#';

// engine signals
const string C::GOOD_SIGNAL = "good";
const string C::BAD_SIGNAL = "bad";

// item IDs
const string C::BAD_ITEM_ID = "bad_item_id";

const string C::SMELTER_ID = "smelter";
const string C::FORGE_ID = "forge";
const string C::ANVIL_ID = "anvil";
const string C::DEBRIS_ID = "debris";

// item IDs -> materials
const string C::WOOD_ID = "wood";
const string C::STONE_ID = "stone";
const string C::VINE_ID = "vine";
const string C::STICK_ID = "stick";
const string C::BRANCH_ID = "branch";
const string C::ARROW_ID = "arrow";
const string C::ARROWHEAD_ID = "arrowhead";
const string C::BOARD_ID = "board";

// item IDs -> other
const string C::TREE_ID = "tree";
const string C::CHEST_ID = "chest";

// item IDs -> equipment
const string C::AXE_ID = "axe";
const string C::STAFF_ID = "staff";
const string C::SWORD_ID = "sword";
const string C::BOW_ID = "bow";
const string C::TORCH_ID = "torch";
const string C::HAMMER_ID = "hammer";

// commands
const string C::BAD_COMMAND = "unknown_command";

// action commands
const string C::LOGIN_COMMAND = "login";
const string C::LOGOUT_COMMAND = "logout";
const string C::HELP_COMMAND = "help";
const string C::TAKE_COMMAND = "take";
const string C::EQUIP_COMMAND = "equip";
const string C::ITEM_COMMAND = "item"; // view equipped item
const string C::DEQUIP_COMMAND = "dequip";
const string C::CRAFT_COMMAND = "craft";
const string C::MOVE_COMMAND = "move";
const string C::DROP_COMMAND = "drop";
const string C::INSERT_COMMAND = "insert";
const string C::FROM_COMMAND = "from";
const string C::CONSTRUCT_COMMAND = "construct";
const string C::WAIT_COMMAND = "wait"; // for debugging (see constants.h)
const string C::WITH_COMMAND = "with";
const string C::PRINT_RECIPES_COMMAND = "recipes"; // for debugging (see constants.h)
const string C::ATTACK_COMMAND = "attack";

// direction commands/IDs
const string C::NORTH = "north";
const string C::NORTH_EAST = "northeast";
const string C::EAST = "east";
const string C::SOUTH_EAST = "southeast";
const string C::SOUTH = "south";
const string C::SOUTH_WEST = "southwest";
const string C::WEST = "west";
const string C::NORTH_WEST = "northwest";

// floor and ceiling IDs + directions
const string C::FLOOR = "floor";
const string C::CEILING = "ceiling";

const string C::UP = "up";
const string C::DOWN = "down";

// doors and walls
const string C::SURFACE = "surface";
const string C::WALL = "wall";
const string C::DOOR = "door";

// room xml
const string C::XML_ROOM = "room";
const string C::XML_IS_WATER = "is_water";
const string C::XML_ITEM = "item";
const string C::XML_ITEM_HEALTH = "health";
const string C::XML_SURFACE = "surface";
const string C::XML_SURFACE_HEALTH = "health";
const string C::XML_SURFACE_DIRECTION = "direction";
const string C::XML_SURFACE_MATERIAL = "material";
const string C::XML_DOOR = "door";
const string C::XML_DOOR_HEALTH = "health";
const string C::XML_DOOR_MATERIAL = "material";
const string C::XML_DOOR_FACTION = "faction";
const string C::XML_CHEST = "chest";
const string C::XML_CHEST_HEALTH = "health";
const string C::XML_CHEST_EQUIPMENT = "equipment_contents";
const string C::XML_CHEST_MATERIALS = "material_contents";
const string C::XML_CHEST_MATERIALS_COUNT = "count";

// user data xml
const string C::XML_USER_STATUS = "status";
const string C::XML_USER_STATUS_CURRENT_HEALTH = "current_health";
const string C::XML_USER_LOCATION = "location";
const string C::XML_USER_LEVELS = "levels";
const string C::XML_USER_EQUIPMENT = "equipment";
const string C::XML_USER_MATERIALS = "materials";
const string C::XML_USER_MATERIAL_COUNT = "count";

// user level xml
const string C::XML_LEVEL_SWORDSMANSHIP = "swordsmanship";
const string C::XML_LEVEL_ARCHERY = "archery";
const string C::XML_LEVEL_FOREST_VISIBILITY = "forest_visibility";
const string C::XML_LEVEL_HEALTH_MAX = "max_health";

// user health xml
const string C::XML_CURRENT_HEALTH = "current_health";

// item health/integrity
const int C::DEFAULT_ITEM_MIN_HEALTH = 0;
const int C::DEFAULT_ITEM_MAX_HEALTH = 100;

// store a vector of all valid surface ids
const vector<string> C::surface_ids = { C::NORTH, C::EAST, C::SOUTH, C::WEST, C::FLOOR, C::CEILING };

// store a vector of all valid movement directions
const vector<string> C::direction_ids = {
	C::NORTH, C::EAST, C::SOUTH, C::WEST,
	C::NORTH_EAST, C::NORTH_WEST,
	C::SOUTH_EAST, C::SOUTH_WEST,
	C::UP, C::DOWN };

const vector<string> C::primary_direction_ids = {
	C::NORTH, C::EAST, C::SOUTH, C::WEST };

// map surface ids to their opposite surface
const map<string, string> C::opposite_surface_id = {
	{ C::NORTH, C::SOUTH },
	{ C::SOUTH, C::NORTH },
	{ C::EAST, C::WEST },
	{ C::WEST, C::EAST },
	{ C::CEILING, C::FLOOR },
	{ C::FLOOR, C::CEILING },
};

const map<string, string> C::opposite_direction_id = {
	{ C::NORTH, C::SOUTH },
	{ C::SOUTH, C::NORTH },
	{ C::EAST, C::WEST },
	{ C::WEST, C::EAST },
	{ C::NORTH_EAST, C::SOUTH_WEST },
	{ C::NORTH_WEST, C::SOUTH_EAST },
	{ C::SOUTH_EAST, C::NORTH_WEST },
	{ C::SOUTH_WEST, C::NORTH_EAST }
};

// Amount of a resource required to construct a surface (wall/ceiling/floor) of that type
const map<string, unsigned> C::SURFACE_REQUIREMENTS =
{
	{ C::WOOD_ID, 5 }, // {material, count required}
	{ C::STONE_ID, 5 },
	{ C::STICK_ID, 5 },
	{ C::BRANCH_ID, 5 }
};

// Amount of a resource required to construct a door of that type
const map<string, unsigned> C::DOOR_REQUIREMENTS =
{
	{ C::WOOD_ID, 5 }, // {material, count required}
	{ C::STONE_ID, 5 },
	{ C::STICK_ID, 5 },
	{ C::BRANCH_ID, 5 }
};

// box drawing

const char C::NW_CORNER = char(201); // ╔
const char C::NE_CORNER = char(187); // ╗
const char C::SW_CORNER = char(200); // ╚
const char C::SE_CORNER = char(188); // ╝

const char C::NES_CORNER = char(204); // ╠
const char C::ESW_CORNER = char(203); // ­╦
const char C::NSW_CORNER = char(185); // ╣
const char C::NEW_CORNER = char(202); // ╩

const char C::NESW = char(206); // ╬

const char C::NS_WALL = char(186); // ║
const char C::WE_WALL = char(205); // ═

const char C::NS_DOOR = char(179); // │
const char C::WE_DOOR = char(196); // ─

// more box drawing

const vector<char> C::CORNERS = {
	// wsen
	// 0000 land_char
	C::LAND_CHAR,

	// 0001 ns_wall
	C::NS_WALL,

	// 0010 we_wall
	C::WE_WALL,

	// 0011 sw_corner
	C::SW_CORNER,

	// 0100 ns_wall
	C::NS_WALL,

	// 0101 ns_wall
	C::NS_WALL,

	// 0110 nw_corner
	C::NW_CORNER,

	// 0111 nes_corner
	C::NES_CORNER,

	// 1000 we_wall
	C::WE_WALL,

	// 1001 se_corner
	C::SE_CORNER,

	// 1010 we_wall
	C::WE_WALL,

	// 1011 new_corner
	C::NEW_CORNER,

	// 1100 ne_corner
	C::NE_CORNER,

	// 1101 nsw_corner
	C::NSW_CORNER,

	// 1110 esw_corner
	C::ESW_CORNER,

	// 1111 nesw
	C::NESW
};

// walls, ceiling, and floor - min and max health

const int C::MIN_SURFACE_HEALTH = 0;
const int C::MAX_SURFACE_HEALTH = 100;
const int C::MAX_DOOR_HEALTH = 100;

/* Create a two-dimensional map to calculate damamge values.
Associate each attacking implement with a map of target, value pairs. */

const map<string, map<string, int>> C::damage_tables =
{
	// unarmed attacks are deliberately set high for testing

	{ string(C::ATTACK_COMMAND), { // attack command is also used to represent an unarmed attack
		{ C::STICK_ID, 49 }, // formerly 6
		{ C::WOOD_ID, 49 }, // formerly 3
		{ C::STONE_ID, 49, }, // formerly 1
		{ C::BRANCH_ID, 49 }
	} },

	{ string(C::STAFF_ID), {
		{ C::STICK_ID, 8 },
		{ C::WOOD_ID, 5 },
		{ C::STONE_ID, 1 }
	} },

	{ string(C::AXE_ID), {
		{ C::STICK_ID, 10 },
		{ C::WOOD_ID, 10 },
		{ C::STONE_ID, 5 }
	} },

	{ string(C::SWORD_ID), {
		{ C::STICK_ID, 8 },
		{ C::WOOD_ID, 7 },
		{ C::STONE_ID, 2 }
	} }
};

// AI movement costs
const int C::AI_MOVEMENT_COST = 10; // 1 * 10
const int C::AI_MOVEMENT_COST_DIAGONAL = 14; // sqrt(2) * 10
const int C::AI_MOVEMENT_COST_FOREST = C::AI_MOVEMENT_COST * 2;
const int C::AI_MOVEMENT_COST_FOREST_DIAGONAL = C::AI_MOVEMENT_COST_DIAGONAL * 2;

// levels
const int C::SWORDSMANSHIP_LEVEL_MIN = 10; // damage inflicted per strike
const int C::SWORDSMANSHIP_LEVEL_MAX = 50;
const int C::ARCHERY_LEVEL_MIN = 50; // odds of a shot making contact
const int C::ARCHERY_LEVEL_MAX = 90;
const int C::FOREST_VISIBILITY_LEVEL_MIN = 10; // odds of seeing a hostile that is in a forest room (redrawn for every move of either player
const int C::FOREST_VISIBILITY_LEVEL_MAX = 90;
const int C::HEALTH_MIN = 0;
const int C::FULL_HEALTH_MIN = 100; // this is full health for new players
const int C::FULL_HEALTH_MAX = 200; // this is full health for fully leveled health

// AI objective keywords
const string C::AI_OBJECTIVE_ACQUIRE = "acquire";
const string C::AI_OBJECTIVE_GOTO = "goto";
const string C::AI_OBJECTIVE_CONSTRUCT = "construct";

// chests
const int C::MAX_CHEST_HEALTH = 100;

// NPC types
const string C::AI_TYPE_FIGHTER = "fighter";
const string C::AI_TYPE_WORKER = "worker";
const string C::AI_TYPE_BLACKSMITH = "blacksmith";
const string C::AI_TYPE_WATCH_GUARD = "watchguard";
const string C::AI_TYPE_PATROL_GUARD = "partrolguard";
const string C::AI_TYPE_MINER = "miner";

// fortress generation

const int C::FORTRESS_PARTITION_MIN_SIZE = 3;
const int C::FORTRESS_MIN_X = 20;
const int C::FORTRESS_MAX_X = 20;
const int C::FORTRESS_MIN_Y = 20;
const int C::FORTRESS_MAX_Y = 20;
