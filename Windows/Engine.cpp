#include <algorithm>

#include "System.h"

#include "Utils/FileUtils.h"
#include "Utils/StringUtils.h"
#include "Utils/NDS/NarcUtils.h"
#include "Utils/NDS/Alle5Format.h"
#include "Utils/NDS/NCLRFormat.h"
#include "Utils/NDS/NCGRFormat.h"

#include "Windows/Engine.h"

#include "Modules/Module.h"
#include "Modules/List.h"
#include "Modules/Pokemon.h"
#include "Modules/Item.h"
#include "Modules/Move.h"
#include "Modules/Trainer.h"
#include "Modules/Encounter.h"

#define CACHE_SIZE 64

enum DataNarc
{
	POKEMON_NARCS,
	ITEM_NARCS,
	MOVE_NARCS,
	TRAINER_NARCS,
	ZONE_NARCS,
	ENCOUNTER_NARCS,
};
struct NarcPath
{
	vector<string> paths[2];
};
const vector<NarcPath> narcPaths = {
	// POKEMONS -> PERSONAL, LEARNSET, EVOLUTION, CHILD
	{ vector<string>({NARC_PATH_TEMPLATE("0", "1", "6"), NARC_PATH_TEMPLATE("0", "1", "8"), NARC_PATH_TEMPLATE("0", "1", "9"), NARC_PATH_TEMPLATE("0", "2", "0"), NARC_PATH_TEMPLATE("0", "2", "0")}), vector<string>({NARC_PATH_TEMPLATE("0", "1", "6"), NARC_PATH_TEMPLATE("0", "1", "8"), NARC_PATH_TEMPLATE("0", "1", "9"), NARC_PATH_TEMPLATE("0", "2", "0"), NARC_PATH_TEMPLATE("0", "2", "0")}) },
	// ITEMS
	{ vector<string>({NARC_PATH_TEMPLATE("0", "2", "4")}) , vector<string>({NARC_PATH_TEMPLATE("0", "2", "4")}) },
	// MOVES -> DATA, ANIMATIONS
	{ vector<string>({NARC_PATH_TEMPLATE("0", "2", "1"), NARC_PATH_TEMPLATE("0", "6", "6")}) , vector<string>({NARC_PATH_TEMPLATE("0", "2", "1"), NARC_PATH_TEMPLATE("0", "6", "5")}) },
	// TRAINERS -> DATA, TEAMS
	{ vector<string>({NARC_PATH_TEMPLATE("0", "9", "2"), NARC_PATH_TEMPLATE("0", "9", "3")}) , vector<string>({NARC_PATH_TEMPLATE("0", "9", "1"), NARC_PATH_TEMPLATE("0", "9", "2")}) },
	// ZONES
	{ vector<string>({NARC_PATH_TEMPLATE("0", "1", "2")}) , vector<string>({NARC_PATH_TEMPLATE("0", "1", "2")}) },
	// ENCOUNTERS
	{ vector<string>({NARC_PATH_TEMPLATE("1", "2", "6")}) , vector<string>({NARC_PATH_TEMPLATE("1", "2", "7")}) },
};

const vector<string> alternateVersions = {
	"POKEMON B2",
	"POKEMON W2",
};

const vector<string> validGameCodes = {
	"IRDO",
};

#define PKM_ICON_PALETTE_FILE 0
u16 PokemonIconPaletteIndices[] = {
    0, 0x11, 0x11, 0x11, 0, 0, 0, 0, 0x22, 0x22, 0x11, 0x11, 0, 0x11, 0x22, 0x22, 0,
    0, 0, 0x22, 0x11, 0, 0, 0x22, 0x22, 0x22, 0, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22,
    0x22, 0x22, 0, 0, 0x22, 0x11, 0, 0, 0x22, 0x22, 0x11, 0, 0, 0, 0, 0, 0x22, 0x22,
    0x22, 0x11, 0x11, 0x11, 0x22, 0x11, 0x22, 0, 0, 0, 0, 0, 0x22, 0x22, 0x22, 0, 0x22,
    0, 0x11, 0x11, 0x11, 0x22, 0x22, 0x11, 0x11, 0x11, 0, 0, 0, 0, 0, 0, 0x11, 0x22,
    0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x11, 0x22,
    0x22, 0, 0, 0, 0x11, 0x11, 0x11, 0x22, 0x22, 0x11, 0x22, 0x22, 0x11, 0x11, 0, 0,
    0x11, 0, 0, 0, 0, 0x22, 0x22, 0, 0x11, 0x22, 0x11, 0, 0x22, 0x22, 0, 0, 0x22, 0x22,
    0x22, 0, 0, 0, 0, 0, 0, 0x22, 0x22, 0, 0x11, 0, 0, 0, 0, 0, 0x22, 0x22, 0, 0x11,
    0x11, 0x11, 0x11, 0x11, 0x11, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0, 0, 0x11,
    0, 0x22, 0x22, 0, 0, 0, 0x11, 0x22, 0x22, 0, 0, 0x22, 0, 0, 0x11, 0x22, 0x22, 0x11,
    0x11, 0x11, 0x11, 0x22, 0x22, 0x11, 0x11, 0x11, 0, 0, 0x22, 0x22, 0x22, 0, 0, 0,
    0, 0x11, 0, 0x22, 0x22, 0x22, 0, 0, 0x22, 0, 0, 0x11, 0x22, 0, 0, 0x22, 0, 0, 0x22,
    0x22, 0, 0, 0, 0, 0x22, 0, 0, 0, 0, 0, 0, 0, 0x22, 0x11, 0x22, 0x22, 0x11, 0x11,
    0x11, 0x11, 0x11, 0, 0x22, 0, 0x11, 0, 0x11, 0, 0x11, 0x11, 0x11, 0, 0x11, 0, 0,
    0, 0, 0, 0, 0x22, 0x22, 0x22, 0x22, 0, 0x22, 0, 0x22, 0x11, 0x11, 0x11, 0x11, 0x11,
    0x11, 0, 0x22, 0x22, 0, 0, 0x11, 0x11, 0x11, 0x22, 0, 0x11, 0x11, 0x22, 0x22, 0x11,
    0x11, 0x11, 0x11, 0, 0x22, 0x22, 0x22, 0x11, 0x22, 0, 0, 0x22, 0x22, 0x22, 0x22,
    0x22, 0x22, 0, 0, 0x11, 0, 0, 0, 0, 0x22, 0, 0x11, 0x22, 0, 0, 0x22, 0, 0x11, 0,
    0x11, 0, 0x22, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0, 0, 0, 0x22, 0x11, 0, 0, 0,
    0, 0, 0x11, 0, 0x22, 0, 0, 0, 0x22, 0, 0, 0x11, 0, 0, 0, 0, 0x11, 0, 0, 0, 0x22,
    0, 0x22, 0x22, 0, 0, 0, 0, 0x11, 0, 0x22, 0x22, 0, 0, 0, 0, 0x22, 0x22, 0, 0, 0x22,
    0x22, 0, 0x11, 0, 0, 0x11, 0x11, 0x11, 0x11, 0, 0, 0x22, 0x22, 0, 0, 0, 0, 0x22,
    0x22, 0, 0, 0, 0, 0, 0x11, 0, 0, 0, 0x11, 0x11, 0x11, 0x11, 0, 0, 0, 0, 0, 0, 0x11,
    0, 0, 0, 0x22, 0x22, 0x22, 0x22, 0x22, 0, 0x11, 0, 0x22, 0, 0x22, 0x22, 0, 0, 0x11,
    0, 0, 0, 0, 0, 0, 0, 0x22, 0x22, 0x22, 0x11, 0x11, 0, 0x22, 0, 0, 0x11, 0, 0, 0,
    0x11, 0x11, 0, 0, 0x11, 0, 0x22, 0x11, 0, 0x22, 0x11, 0x11, 0, 0x22, 0x22, 0, 0x11,
    0, 0x22, 0x22, 0, 0, 0, 0, 0x22, 0x22, 0, 0, 0, 0, 0, 0, 0, 0x11, 0, 0, 0x11, 0x11,
    0x11, 0, 0, 0, 0, 0, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0, 0, 0x11, 0x11, 0x22,
    0x22, 0, 0, 0, 0, 0, 0, 0, 0x22, 0x22, 0, 0, 0, 0, 0, 0, 0, 0, 0x11, 0x11, 0x11,
    0x22, 0x22, 0, 0x11, 0, 0x11, 0x11, 0x11, 0x11, 0, 0, 0x11, 0x11, 0x11, 0x11, 0,
    0x11, 0x11, 0, 0, 0, 0x11, 0, 0x22, 0x11, 0, 0, 0x11, 0, 0x22, 0x22, 0, 0, 0x11,
    0x11, 0, 0, 0, 0, 0x22, 0x22, 0x22, 0x11, 0x11, 0x11, 0, 0x22, 0, 0, 0, 0x11, 0x11,
    0, 0, 0, 0x22, 0, 0x10, 0x10, 0, 0, 0, 0x11, 0x11, 0, 0, 0, 0, 0, 0, 0, 0x22, 0x22,
    0x22, 0x22, 0x11, 0x11, 0x22, 0, 0, 0, 0, 0x11, 0x22, 0, 0x22, 0, 0, 0, 0, 0, 0,
    0x11, 0, 0, 0x11, 0x22, 0, 0x22, 0x22, 0x22, 0, 0, 0, 0x22, 0x11, 0x11, 0, 0, 0,
    0, 0, 0, 0x11, 0x22, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x11, 0x22, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x11,
    0, 0x11, 0, 0x11, 0, 0, 0, 0, 0, 0, 0, 0, 0x11, 0, 0, 0x11, 0, 0x22, 0x11, 0, 0x11,
    0x11, 0, 0, 0, 0, 0, 0, 0x22, 0x22, 0x22, 0x22, 0, 0, 0,
};

u16 ItemIconGraphicIndices[] = {
    0x3FD, 0x3FE, 2, 3, 4, 5, 6, 7, 8, 9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, 0x10, 0x11, 0x12,
    0x13, 0x14, 0x13, 0x15, 0x16, 0x17, 0x16, 0x297, 0x298, 0x299, 0x29A, 0x29B, 0x29C,
    0x29D, 0x29E, 0x18, 0x19, 0x1A, 0x1B, 0x1E, 0x1C, 0x1E, 0x1D, 0x1E, 0x1F, 0x1E, 0x20,
    0x21, 0x22, 0x21, 0x23, 0x18, 0x24, 0x18, 0x25, 0x26, 0x27, 0x28, 0x2A, 0x29, 0x2A,
    0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x33, 0x37,
    0x38, 0x39, 0x3A, 0x3B, 0x3A, 0x3C, 0x3A, 0x3D, 0x3A, 0x3E, 0x3F, 0x40, 0x47, 0x48,
    0x49, 0x4A, 0x53, 0x54, 0x55, 0x56, 0x55, 0x57, 0x55, 0x58, 0x55, 0x59, 0x5A, 0x5B,
    0x5C, 0x5D, 0x55, 0x5E, 0x5F, 0x60, 0x1D2, 0x1D3, 0x64, 0x61, 0x64, 0x62, 0x64, 0x63,
    0x64, 0x65, 0x64, 0x66, 0x64, 0x67, 0x64, 0x68, 0x64, 0x1D5, 0x69, 0x6A, 0x6B, 0x6C,
    0x41, 0x42, 0x41, 0x43, 0x41, 0x44, 0x41, 0x45, 0x41, 0x46, 0x33, 0x4B, 0x4C, 0x4D,
    0x4E, 0x4F, 0x4E, 0x50, 0x4E, 0x51, 0x4E, 0x52, 0x6D, 0x6E, 0x6D, 0x6F, 0x70, 0x71,
    0x6D, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E,
    0x7F, 0x81, 0x80, 0x81, 0x82, 0x83, 0x84, 0x83, 0x85, 0x86, 0x87, 0x86, 0x88, 0x89,
    0x8A, 0x8B, 0x1D6, 0x1D7, 0x1D8, 0x1D9, 0x1DA, 0x1DB, 0x1DC, 0x1DD, 0x1DE, 0x1DF,
    0x188, 0x189, 0x18A, 0x189, 0x1AF, 0x1B0, 0x1B1, 0x1B0, 0x1A9, 0x1AA, 0x269, 0x26A,
    0x267, 0x268, 0x151, 0x152, 0x1E0, 0x1E1, 0x1E2, 0x1E3, 0x1E4, 0x1E5, 0x1E6, 0x1E7,
    0x1E8, 0x1E9, 0x2BB, 0x2BC, 0x3FD, 0x3FE, 0x3FD, 0x3FE, 0x3FD, 0x3FE, 0x389, 0x38A,
    0x389, 0x38B, 0x389, 0x38C, 0x389, 0x38D, 0x3FD, 0x3FE, 0x3FD, 0x3FE, 0x3FD, 0x3FE,
    0x3FD, 0x3FE, 0x3FD, 0x3FE, 0x3FD, 0x3FE, 0x3FD, 0x3FE, 0x3FD, 0x3FE, 0x3FD, 0x3FE,
    0x3FD, 0x3FE, 0x3FD, 0x3FE, 0x3FD, 0x3FE, 0x3FD, 0x3FE, 0x3FD, 0x3FE, 0x38E, 0x38F,
    0x288, 0x289, 0x286, 0x287, 0x319, 0x31A, 0x31B, 0x31C, 0x31D, 0x31E, 0x31F, 0x320,
    0x321, 0x322, 0x323, 0x324, 0x325, 0x326, 0x327, 0x328, 0x329, 0x32A, 0x32B, 0x32C,
    0x32D, 0x32E, 0x32F, 0x330, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC,
    0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA,
    0xBB, 0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8,
    0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6,
    0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF, 0xE0, 0xE1, 0xE2, 0xE3, 0xE4,
    0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0x22A, 0x22B, 0x22C, 0x22D, 0x22E, 0x22F, 0x230, 0x231,
    0x232, 0x233, 0x234, 0x235, 0x236, 0x237, 0x238, 0x239, 0x23A, 0x23B, 0x23C, 0x23D,
    0x23E, 0x23F, 0x240, 0x241, 0x242, 0x243, 0x244, 0x245, 0x246, 0x247, 0x248, 0x249,
    0x24A, 0x24B, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5,
    0xF6, 0xF7, 0xF8, 0xF9, 0x24C, 0x24D, 0x24E, 0x24F, 0x250, 0x251, 0x252, 0x253, 0xFA,
    0xFB, 0xFC, 0xFD, 0xFE, 0xFF, 0x100, 0x101, 0x102, 0x103, 0x104, 0x105, 0xFC, 0x106,
    0x107, 0x108, 0x109, 0x10A, 0x10B, 0x10C, 0x10D, 0x10E, 0x10F, 0x110, 0x111, 0x112,
    0x113, 0x114, 0x115, 0x116, 0x117, 0x118, 0x119, 0x11A, 0x11B, 0x11C, 0x11D, 0x11E,
    0x11F, 0x120, 0x121, 0x122, 0x123, 0x124, 0x125, 0x126, 0x127, 0x128, 0x129, 0x12A,
    0x12B, 0x12C, 0x12D, 0x12E, 0x12F, 0x130, 0x131, 0x130, 0x132, 0x133, 0x134, 0x135,
    0x136, 0x137, 0x138, 0x139, 0x13A, 0x13B, 0x13C, 0x13D, 0x13E, 0x13F, 0x140, 0x141,
    0x142, 0x143, 0x144, 0x145, 0x146, 0x147, 0x148, 0x4D, 0x149, 0x14A, 0x14B, 0x14C,
    0x14D, 0x14E, 0x14F, 0x150, 0x151, 0x152, 0x153, 0x154, 0x155, 0x156, 0x155, 0x157,
    0x155, 0x158, 0x155, 0x159, 0x155, 0x15A, 0x1EA, 0x1EB, 0x1EC, 0x1ED, 0x1EE, 0x1EF,
    0x1F0, 0x1F1, 0x263, 0x264, 0x1F2, 0x1F3, 0x1F4, 0x1F5, 0x1F6, 0x1F7, 0x1F8, 0x1F9,
    0x14F, 0x292, 0x293, 0x294, 0x1FA, 0x1FB, 0x1FC, 0x1FD, 0x265, 0x266, 0x28A, 0x28B,
    0x28E, 0x28F, 0x1FE, 0x1FF, 0x27C, 0x27D, 0x27E, 0x27F, 0x280, 0x281, 0x282, 0x283,
    0x28C, 0x28D, 0x200, 0x201, 0x202, 0x203, 0x2AB, 0x2AC, 0x2B3, 0x2B4, 0x2B5, 0x2B6,
    0x2AD, 0x2AE, 0x2AF, 0x2B0, 0x2B1, 0x2B2, 0x204, 0x205, 0x206, 0x207, 0x208, 0x209,
    0x26B, 0x26C, 0x26B, 0x26D, 0x26B, 0x26E, 0x26B, 0x26F, 0x26B, 0x270, 0x26B, 0x271,
    0x26B, 0x272, 0x26B, 0x273, 0x26B, 0x274, 0x26B, 0x275, 0x26B, 0x276, 0x26B, 0x277,
    0x26B, 0x278, 0x26B, 0x279, 0x26B, 0x27A, 0x26B, 0x27B, 0x20A, 0x20B, 0x20C, 0x20D,
    0x20E, 0x20F, 0x210, 0x211, 0x212, 0x213, 0x214, 0x215, 0x216, 0x217, 0x2B7, 0x2B8,
    0x218, 0x219, 0x21A, 0x21B, 0x21C, 0x21D, 0x21E, 0x21F, 0x220, 0x221, 0x222, 0x223,
    0x18D, 0x197, 0x18D, 0x18F, 0x18D, 0x191, 0x18D, 0x191, 0x18D, 0x192, 0x18D, 0x193,
    0x18D, 0x194, 0x18D, 0x18E, 0x18D, 0x193, 0x18D, 0x192, 0x18D, 0x196, 0x18D, 0x197,
    0x18D, 0x194, 0x18D, 0x194, 0x18D, 0x192, 0x18D, 0x191, 0x18D, 0x192, 0x18D, 0x190,
    0x18D, 0x191, 0x18D, 0x192, 0x18D, 0x192, 0x18D, 0x195, 0x18D, 0x19C, 0x18D, 0x199,
    0x18D, 0x199, 0x18D, 0x19A, 0x18D, 0x192, 0x18D, 0x19A, 0x18D, 0x191, 0x18D, 0x19B,
    0x18D, 0x18E, 0x18D, 0x192, 0x18D, 0x191, 0x18D, 0x193, 0x18D, 0x196, 0x18D, 0x193,
    0x18D, 0x19C, 0x18D, 0x196, 0x18D, 0x19C, 0x18D, 0x19D, 0x18D, 0x197, 0x18D, 0x192,
    0x18D, 0x196, 0x18D, 0x191, 0x18D, 0x192, 0x18D, 0x197, 0x18D, 0x18E, 0x18D, 0x192,
    0x18D, 0x192, 0x18D, 0x196, 0x18D, 0x191, 0x18D, 0x18E, 0x18D, 0x195, 0x18D, 0x192,
    0x18D, 0x190, 0x18D, 0x197, 0x18D, 0x199, 0x18D, 0x19D, 0x18D, 0x196, 0x18D, 0x197,
    0x18D, 0x196, 0x18D, 0x19D, 0x18D, 0x197, 0x18D, 0x192, 0x18D, 0x19B, 0x18D, 0x197,
    0x18D, 0x192, 0x18D, 0x192, 0x18D, 0x19C, 0x18D, 0x192, 0x18D, 0x19C, 0x18D, 0x199,
    0x18D, 0x199, 0x18D, 0x198, 0x18D, 0x192, 0x18D, 0x262, 0x18D, 0x192, 0x18D, 0x19A,
    0x18D, 0x194, 0x18D, 0x19C, 0x18D, 0x262, 0x18D, 0x18F, 0x18D, 0x192, 0x18D, 0x193,
    0x18D, 0x191, 0x18D, 0x195, 0x18D, 0x192, 0x18D, 0x19D, 0x18D, 0x262, 0x18D, 0x192,
    0x18D, 0x198, 0x18D, 0x191, 0x19E, 0x192, 0x19E, 0x19D, 0x19E, 0x190, 0x19E, 0x192,
    0x19E, 0x190, 0x19E, 0x190, 0x3FD, 0x3FE, 0x3FD, 0x3FE, 0x224, 0x225, 0x226, 0x227,
    0x228, 0x229, 0x2B9, 0x2BA, 0x2A9, 0x2AA, 0x25E, 0x25F, 0x29F, 0x2A0, 0x2A3, 0x2A4,
    0x2A1, 0x2A2, 0x295, 0x296, 0x284, 0x285, 0x25C, 0x25D, 0x290, 0x291, 0x2A7, 0x2A8,
    0x1B7, 0x1B8, 0x1B9, 0x1BA, 0x15D, 0x15E, 0x161, 0x162, 0x163, 0x164, 0x165, 0x166,
    0x16B, 0x16C, 0x175, 0x176, 0x1B5, 0x1B6, 0x1AD, 0x1AE, 0x177, 0x40, 0x25A, 0x25B,
    0x260, 0x261, 0x2A5, 0x2A6, 0x167, 0x168, 0x169, 0x16A, 0x254, 0x255, 0x19F, 0x1A0,
    0x1A1, 0x1A2, 0x1A3, 0x1A4, 0x1A5, 0x1A6, 0x256, 0x257, 0x258, 0x259, 0x2BD, 0x2BE,
    0x2BF, 0x2C0, 0x2C1, 0x2C2, 0x2C5, 0x2C6, 0x2F7, 0x2F8, 0x2C7, 0x2C8, 0x3E5, 0x3E6,
    0x2F9, 0x2FA, 0x2FB, 0x2FC, 0x2FD, 0x2FE, 0x2FF, 0x300, 0x301, 0x302, 0x2C9, 0x2CA,
    0x303, 0x304, 0x305, 0x306, 0x307, 0x308, 0x309, 0x30A, 0x30B, 0x30C, 0x30D, 0x30E,
    0x30F, 0x310, 0x2DD, 0x2DE, 0x2DF, 0x2E0, 0x2E1, 0x2E2, 0x2E3, 0x2E4, 0x2E5, 0x2E6,
    0x2E7, 0x2E8, 0x2E9, 0x2EA, 0x2D3, 0x2D4, 0x2CD, 0x2CE, 0x2CB, 0x2CC, 0x2D1, 0x2D2,
    0x2D7, 0x2D8, 0x2D5, 0x2D6, 0x2CF, 0x2D0, 0x2DB, 0x2DC, 0x2D9, 0x2DA, 0x25E, 0x25F,
    0x311, 0x312, 0x313, 0x314, 0x2ED, 0x2EE, 0x3FD, 0x3FE, 0x3FD, 0x3FE, 0x3FD, 0x3FE,
    0x3FD, 0x3FE, 0x3FD, 0x3FE, 0x3FD, 0x3FE, 0x3FD, 0x3FE, 0x3FD, 0x3FE, 0x3FD, 0x3FE,
    0x3FD, 0x3FE, 0x3FD, 0x3FE, 0x3FD, 0x3FE, 0x3FD, 0x3FE, 0x3FD, 0x3FE, 0x3FD, 0x3FE,
    0x3FD, 0x3FE, 0x3FD, 0x3FE, 0x3FD, 0x3FE, 0x3FD, 0x3FE, 0x3FD, 0x3FE, 0x3FD, 0x3FE,
    0x3FD, 0x3FE, 0x3FD, 0x3FE, 0x3FD, 0x3FE, 0x3FD, 0x3FE, 0x3FD, 0x3FE, 0x3FD, 0x3FE,
    0x2F3, 0x2F4, 0x317, 0x318, 0x2EF, 0x2F0, 0x2F1, 0x2F2, 0x2F5, 0x2F6, 0x390, 0x391,
    0x392, 0x393, 0x394, 0x395, 0x396, 0x397, 0x398, 0x399, 0x39A, 0x39B, 0x39C, 0x39D,
    0x39E, 0x39F, 0x3A0, 0x3A1, 0x3A2, 0x3A3, 0x3A4, 0x3A5, 0x3A6, 0x3A7, 0x3A6, 0x3A8,
    0x3A6, 0x3A9, 0x3A6, 0x3AA, 0x3A6, 0x3AB, 0x3A6, 0x3AC, 0x3A6, 0x3AD, 0x3A6, 0x3AE,
    0x3A6, 0x3AF, 0x3A6, 0x3B0, 0x3A6, 0x3B1, 0x3A6, 0x3B2, 0x3A6, 0x3B3, 0x3A6, 0x3B4,
    0x3A6, 0x3B5, 0x3A6, 0x3B6, 0x3A6, 0x3B7, 0x3B8, 0x3B9, 0x3B8, 0x3BA, 0x3B8, 0x3BB,
    0x3B8, 0x3BC, 0x3B8, 0x3BD, 0x3B8, 0x3BE, 0x3BF, 0x3C0, 0x3C1, 0x3C2, 0x3C3, 0x3C4,
    0x3C5, 0x3C6, 0x3C7, 0x3C8, 0x3C9, 0x3CA, 0x3CB, 0x3CC, 0x331, 0x332, 0x333, 0x334,
    0x3CD, 0x3CE, 0x3CF, 0x3D0, 0x3D1, 0x3D2, 0x3D3, 0x3D4, 0x3D5, 0x3D6, 0x3D5, 0x3D8,
    0x3D5, 0x3DA, 0x3DB, 0x3DC, 0x3DD, 0x3DE, 0x3DF, 0x3E0, 0x3E1, 0x3E2, 0x3E3, 0x3E4,
    0x348, 0x349, 0x348, 0x34A, 0x348, 0x34B, 0x348, 0x34C, 0x348, 0x34D, 0x348, 0x34E,
    0x348, 0x34F, 0x350, 0x352, 0x350, 0x353, 0x350, 0x354, 0x350, 0x355, 0x350, 0x356,
    0x350, 0x357, 0x358, 0x359, 0x358, 0x35A, 0x358, 0x35B, 0x358, 0x35C, 0x358, 0x35D,
    0x358, 0x35E, 0x35F, 0x360, 0x361, 0x362, 0x363, 0x364, 0x365, 0x366, 0x350, 0x351,
    0x335, 0x336, 0x337, 0x338, 0x18D, 0x199, 0x18D, 0x19C, 0x18D, 0x197, 0x339, 0x3E7,
    0x33B, 0x33C, 0x33D, 0x33E, 0x33D, 0x33E, 0x33D, 0x33E, 0x339, 0x33A, 0x3EA, 0x3E9,
    0x3EC, 0x3EB, 0x3EC, 0x3EB, 0x3F0, 0x3EF, 0x3F2, 0x3F1, 0x3F4, 0x3F3, 0x3F6, 0x3F5,
    0x3F8, 0x3F7, 0x3FA, 0x3F9, 0x339, 0x3E7, 0x339, 0x33A, 0x3FC, 0x3FB,
};

Engine::Engine(Project* const project) : project(project), textureCache(CACHE_SIZE)
{
	Log(INFO, "Constructing engine");

	if (!LoadEngine())
	{
		Quit(CONSTRUCTOR_LOAD_QUIT);
	}

	Log(INFO, "    Constructing success");
}

Engine::~Engine()
{
	Log(INFO, "Destroying engine");

	ClearEngine();

	Log(INFO, "    Destroying success");
}

bool Engine::UsavedData()
{
	for (u32 idx = 0; idx < (u32)datas.size(); ++idx)
	{
		Data* data = datas[idx];
		if (data->HasSaveEvents())
			return true;
	}
	return false;
}

void Engine::Save()
{
	for (u32 idx = 0; idx < (u32)datas.size(); ++idx)
	{
		Data* data = datas[idx];
		data->Save(project);
	}

	SaveProjectSettings(*project);
}

bool Engine::PMCCheck()
{
	if (!enableBuilder)
		return false;
	return PathExists(PathConcat(project->ctrMapProjectDir, PMC_CHECK_PATH));
}

bool Engine::PatchIsInstalled()
{
	if (!PMCCheck())
		return false;
	return PathExists(PathConcat(project->path, PATCH_INSTALLED_FILE));
}

bool Engine::PW2CodeCheck()
{
#if _DEBUG
	return true;
#endif
	string patchPath = DEPLOY_PATCH_DIR;
	if (!PathExists(patchPath))
		return false;

	return PathExists(PathConcat(patchPath, ".git"));
}

void Engine::Quit(const string& msg)
{
	quit = true;
	quitMessage = msg;
}

vector<string>* Engine::GetTextFile(u32 idx)
{
	unordered_map<u32, vector<string>>::iterator itr = textFiles.find(idx);
	if (itr == textFiles.end())
	{
		Log(CRITICAL, "Text file %d is not loaded", idx);
		return nullptr;
	}
	return &(itr->second);
}

u32 Engine::GetConcatDataCount(Data* data, u32 idx)
{
	return data->GetCount(project, idx);
}

u32 Engine::GetConcatDataIdx(Data* data, u32 idx, u32 concatIdx)
{
	return data->GetConcat(idx, concatIdx);
}

bool Engine::StartAction(u32* selectable, u32 selected, u32 group)
{
	if (actionIdx != -1)
	{
		Log(CRITICAL, "Can't start an action if there is already one in progress");
		return false;
	}

	actions.emplace_back(selectable, selected, group);
	actionIdx = (int)actions.size() - 1;
	return true;
}

bool Engine::EndAction()
{
	if (actionIdx == -1)
	{
		Log(CRITICAL, "Can't end an action if there is none in progress");
		return false;
	}
	actionIdx = -1;
	return true;
}

int Engine::GetDataValue(Data* data, u32 idx, u32 field)
{
	return data->GetValue(project, idx, field);
}

u32 Engine::SetDataValue(Data* data, u32 idx, u32 field, int value)
{
	if (actionIdx == -1)
	{
		Log(CRITICAL, "Can't set a data value outside of an action");
		return field;
	}

	return data->SetValue(idx, field, value, (u32)actionIdx);
}

void Engine::InputInt(Data* data, u32 idx, u32 field, u32* selectable, u32 selected, u32 group, const string& label, int max, int min)
{
	int value = data->GetValue(project, idx, field);
	if (value == data->nullValue)
	{
		if (ImGui::Button(LABEL(label + "Null", (int)field)))
		{
			StartAction(selectable, selected, group);
			SetDataValue(data, idx, field, min);
			EndAction();
		}
		return;
	}

	if (value > max || value < min)
	{
		ImGui::Text(label + "(Error): " + to_string(value));
		ImGui::SameLine();
		if (ImGui::Button(LABEL("Fix", (int)field)))
		{
			StartAction(selectable, selected, group);
			SetDataValue(data, idx, field, min);
			EndAction();
		}
		return;
	}

	ImGui::SetNextItemWidth(40.0f);

	if (ImGui::InputInt(LABEL(label, (int)field), &value, 0) && ImGui::IsItemDeactivatedAfterEdit())
	{
		if (value > max)
			value = max;
		if (value < min)
			value = min;

		StartAction(selectable, selected, group);
		SetDataValue(data, idx, field, value);
		EndAction();
	}
}

void Engine::ComboBox(Data* data, u32 idx, u32 field, u32* selectable, u32 selected, u32 group, const string& label, const std::vector<std::string>* const list)
{
	int value = data->GetValue(project, idx, field);
	if (value == data->nullValue)
	{
		if (list && !list->empty())
		{
			if (ImGui::Button(LABEL(label + "Null", (int)field)))
			{
				StartAction(selectable, selected, group);
				SetDataValue(data, idx, field, 0);
				EndAction();
			}
		}
		else
		{
			ImGui::Text("No values");
		}
		return;
	}

	if (!list || value < 0 || value >= (int)list->size())
	{
		ImGui::Text(label + "(Error): " + to_string(value));
		ImGui::SameLine();
		if (list && !list->empty())
		{
			if (ImGui::Button(LABEL("Fix", (int)field)))
			{
				StartAction(selectable, selected, group);
				SetDataValue(data, idx, field, 0);
				EndAction();
			}
		}
		else
		{
			ImGui::Text("No values");
		}
		
		return;
	}

	ImGui::SetNextItemWidth(150.0f);
	if (ImGui::TextInputComboBox(LABEL(label, (int)field), list, &value))
	{
		StartAction(selectable, selected, group);
		SetDataValue(data, idx, field, value);
		EndAction();
	}
}

void Engine::CheckBox(Data* data, u32 idx, u32 field, u32* selectable, u32 selected, u32 group, const string& label)
{
	int value = data->GetValue(project, idx, field);
	if (value == data->nullValue)
	{
		if (ImGui::Button(LABEL(label + "Null", (int)field)))
		{
			StartAction(selectable, selected, group);
			SetDataValue(data, idx, field, 0);
			EndAction();
		}
		return;
	}

	if (value > 1 || value < 0)
	{
		ImGui::Text(label + "(Error): " + to_string(value));
		ImGui::SameLine();
		if (ImGui::Button(LABEL("Fix", (int)field)))
		{
			StartAction(selectable, selected, group);
			SetDataValue(data, idx, field, 0);
			EndAction();
		}
		return;
	}

	bool baalue = (bool)value; // Get it?
	if (ImGui::Checkbox(LABEL(label, (int)field), &baalue))
	{
		StartAction(selectable, selected, group);
		SetDataValue(data, idx, field, (int)baalue);
		EndAction();
	}
}

void Engine::ListBox(Data* data, u32 idx, u32 firstField, u32* selectable, u32 selected, u32 group, const string& label, const vector<string>* const list)
{
	ImGui::Text(label);
	if (ImGui::BeginListBox(string("##" + label).c_str(), ImVec2(150.0f, 200.0f)))
	{
		if (!list)
		{
			ImGui::Text("(Error): No values");
		}
		else
		{
			for (u32 listIdx = 0; listIdx < (u32)list->size(); ++listIdx)
				CheckBox(data, idx, firstField + listIdx, selectable, selected, group, list->at(listIdx));
		}
		ImGui::EndListBox();
	}
}

void Engine::DisplayImage(const string& narcDir, u32 imgFileIdx, u32 flags, u32 palFileIdx, u32 palIdx)
{
    u64 keyVar = imgFileIdx;
    keyVar |= ((u64)palFileIdx << 32);
    Texture& texture = textureCache.Get(narcDir, keyVar, project, TEXTURE_FULL, flags, palIdx);
    if (texture.texture == 0 || texture.width == 0 || texture.height == 0)
        return;

    ImGui::Image(texture.texture, ImVec2(texture.width, texture.height));
    if (ImGui::Button(string("Edit##" + to_string(keyVar)).c_str()))
    {
        string cmd = "./PW2Paint";
        cmd += string(" -i ") + PathConcat("tmp", Cache<Image>::GenerateDataStr(narcDir, imgFileIdx));
        cmd += string(" -p ") + PathConcat("tmp", Cache<Palette>::GenerateDataStr(narcDir, palFileIdx));

        string ctrPath = PathConcat(project->ctrMapProjectDir, CTRMAP_FILESYSTEM_DIR);
        ctrPath = PathConcat(ctrPath, narcDir);
        string imgPath = PathConcat(ctrPath, to_string(imgFileIdx));
        string palPath = PathConcat(ctrPath, to_string(palFileIdx));
        cmd += string(" -si ") + imgPath;
        cmd += string(" -sp ") + palPath;

        cmd += string(" -f ") + to_string(flags);

        system(cmd.c_str());
    }
    ImGui::SameLine();
    if (ImGui::Button(string("Update##" + to_string(keyVar)).c_str()))
    {
        textureCache.Update(narcDir, keyVar, project, TEXTURE_FULL, flags, palIdx);
    }
}

void Engine::DisplayImage(const string& narcDir, u32 imgFileIdx, u32 flags, u32 palFileIdx, u32 palIdx, u32 x, u32 y, u32 width, u32 height)
{
    u64 keyVar = imgFileIdx;
    keyVar |= ((u64)palFileIdx << 32);
    Texture& texture = textureCache.Get(narcDir, keyVar, project, TEXTURE_RECT, flags, palIdx, x, y, width, height);
    if (texture.texture == 0 || texture.width == 0 || texture.height == 0)
        return;

    ImGui::Image(texture.texture, ImVec2(texture.width, texture.height));
    if (ImGui::BeginItemTooltip())
    {
        if (ImGui::Button("Edit Image"))
        {
            string cmd = "./PW2Paint";
            cmd += " -i " + narcDir + to_string(imgFileIdx);
            cmd += " -p " + narcDir + to_string(palFileIdx);
            cmd += " -f " + to_string(flags);
        }
        ImGui::EndTooltip();
    }
}

void Engine::DisplayPokemonIcon(u32 pokemon, u32 form, u32 sex, bool full)
{
    bool spriteForm = (bool)GetDataValue(pokemons, pokemon, FORM_RARE);

    u32 iconFileIdx = 0;
    u32 palIdx = 0;
    if (!spriteForm && form != 0)
    {
        iconFileIdx = 1378;
        u32 formSpriteOffset = (u32)GetDataValue(pokemons, pokemon, FORM_SPRITE_OFFSET);
        iconFileIdx += 2 * (formSpriteOffset + form - 1);

        palIdx = 686;
        palIdx += formSpriteOffset + form;
    }
    else
    {
        iconFileIdx = 8;
        iconFileIdx += 2 * pokemon;

        palIdx = pokemon;
    }

    if (sex)
        palIdx = (PokemonIconPaletteIndices[palIdx] & 0xF0u) >> 4;
    else
        palIdx = PokemonIconPaletteIndices[palIdx] & 0xF;

    u32 flags = IMAGE_FLAG_PKM_ICON;
    if (full)
        DisplayImage(PKM_ICONS_NARC_PATH, iconFileIdx + sex, flags, 0, palIdx);
    else
        DisplayImage(PKM_ICONS_NARC_PATH, iconFileIdx + sex, flags | IMAGE_FLAG_TRANSPARENT_BG, 0, palIdx, 0,0,32,32);
}

void Engine::DisplayItemIcon(u32 item, bool full)
{
    u32 imgFileIdx = ItemIconGraphicIndices[2 * item];
    u32 palFileIdx = ItemIconGraphicIndices[2 * item + 1];

    u32 flags = IMAGE_FLAG_ITEM_ICON;
    if (full)
        DisplayImage(ITEM_ICONS_NARC_PATH, imgFileIdx, flags, palFileIdx, 0);
    else
        DisplayImage(ITEM_ICONS_NARC_PATH, imgFileIdx, flags | IMAGE_FLAG_TRANSPARENT_BG, palFileIdx, 0);
}

ReturnState Engine::RenderGUI()
{
	if (quit)
	{
		if (quitMessage.empty())
			return STOP;

		ReturnState exitState = OK;
		ImGui::Begin("The engine needs to shut down");
		ImGui::Text(quitMessage);
		if (ImGui::Button("OK"))
			exitState = STOP;
		ImGui::End();
		return exitState;
	}

	project->width = (u32)width;
	project->height = (u32)height;

	static bool commandInput = false;
	if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl))
		commandInput = true;
	if (ImGui::IsKeyReleased(ImGuiKey_LeftCtrl))
		commandInput = false;

	if (commandInput)
	{
		if (ImGui::IsKeyPressed(ImGuiKey_S))
			Save();

		if (ImGui::IsKeyPressed(ImGuiKey_R))
		{
			Save();
			ReloadData();
		}

		if (ImGui::IsKeyPressed(ImGuiKey_Z))
			ReverseAction();

		for (u32 group = POKEMON_GROUP; group <= ENCOUNTER_GROUP; ++group)
			if (ImGui::IsKeyPressed((ImGuiKey)(ImGuiKey_0 + group)))
				project->group = group;
	}
	MenuBar();

	ReturnState moduleState = OK;
	for (u32 idx = 0; idx < (u32)modules.size(); ++idx)
	{
		if (modules[idx]->group == ENGINE_GROUP ||
			modules[idx]->group == project->group)
		{
			moduleState = modules[idx]->RenderGUI();
			if (moduleState != OK)
				break;
		}
	}

	return moduleState;
}

bool Engine::LoadEngine()
{
	Log(INFO, "Loading engine");

	// Only enable the Code Injection for sopported game codebases
	for (u32 codeIdx = 0; codeIdx < (u32)validGameCodes.size(); ++codeIdx)
	{
		if (project->gameCode == validGameCodes.at(codeIdx))
		{
			enableBuilder = true;
			break;
		}
	}

	if (PW2CodeCheck())
	{
		LoadPatchSettings();
	}

	const vector<u32> textFileIdexes = {
	ALL_TEXT_FILES
	};
	for (u32 textIdx = 0; textIdx < (u32)textFileIdexes.size(); ++textIdx)
	{
		u32 idx = textFileIdexes[textIdx];
		FileStream stream = LoadFileFromNarc(project->ctrMapProjectDir, project->romDir, TEXT_NARC_PATH, idx);
		if (!stream.data)
		{
			Log(INFO, "    Failed loading text NARC file %d", idx);
			return false;
		}

		vector<string> text;
		LoadAlle5Data(stream, text);
		if (text.empty())
		{
			Log(INFO, "    Failed loading text data %d", idx);
			ReleaseFileStream(stream);
			return false;
		}
		ReleaseFileStream(stream);

		textFiles[idx] = text;
	}

	u32 gameVersion = 0;
	for (u32 versionIdx = 0; versionIdx < (u32)alternateVersions.size(); ++versionIdx)
	{
		if (project->game == alternateVersions.at(versionIdx))
		{
			gameVersion = 1;
			break;
		}
	}

	Log(INFO, "    Loading Data");
	AddData<PokemonData>(pokemons, narcPaths.at(POKEMON_NARCS).paths[gameVersion]);
	AddData<ItemData>(items, narcPaths.at(ITEM_NARCS).paths[gameVersion]);
	AddData<MoveData>(moves, narcPaths.at(MOVE_NARCS).paths[gameVersion]);
	AddData<TrainerData>(trainers, narcPaths.at(TRAINER_NARCS).paths[gameVersion]);
	AddData<ZoneData>(zones, narcPaths.at(ZONE_NARCS).paths[gameVersion]);
	AddData<EncounterData>(encounters, narcPaths.at(ENCOUNTER_NARCS).paths[gameVersion]);

	Log(INFO, "    Setting up PW2Code");
	string buildSettingsPath = PathConcat(project->path, PATCH_SETTINGS_FILE);
	if (!PathExists(buildSettingsPath))
		InstallPatch(buildSettingsPath);

	Log(INFO, "    Creating modules");
	modules.push_back(new List(this, POKEMON_GROUP, &project->pokemon, GetTextFile(PKM_NAME_FILE_ID)));
	modules.push_back(new Pokemon(this, POKEMON_GROUP));

	modules.push_back(new List(this, ITEM_GROUP, &project->item, GetTextFile(ITEM_NAME_FILE_ID)));
	modules.push_back(new Item(this, ITEM_GROUP));

	modules.push_back(new List(this, MOVE_GROUP, &project->move, GetTextFile(MOVE_NAME_FILE_ID)));
	modules.push_back(new Move(this, MOVE_GROUP));

	modules.push_back(new List(this, TRAINER_GROUP, &project->trainer, GetTextFile(TRAINER_NAME_FILE_ID)));
	modules.push_back(new Trainer(this, TRAINER_GROUP));

	modules.push_back(new List(this, ENCOUNTER_GROUP, &project->location, GetTextFile(LOCATION_NAME_FILE_ID)));
	modules.push_back(new Encounter(this, ENCOUNTER_GROUP));

	Log(INFO, "    Loading success");
	return true;
}

bool Engine::ClearEngine()
{
	Log(INFO, "Clearing engine");

	for (u32 idx = 0; idx < (u32)datas.size(); ++idx)
		delete datas[idx];
	datas.clear();

	for (u32 idx = 0; idx < (u32)modules.size(); ++idx)
		delete modules[idx];
	modules.clear();

    textureCache.Release();

	Log(INFO, "    Clearing success");
	return true;
}

bool Engine::SaveData()
{
	for (u32 idx = 0; idx < (u32)datas.size(); ++idx)
		datas.at(idx)->Save(project);

	return true;
}

bool Engine::ReverseAction()
{
	if (actions.empty())
		return false;

	const Action& action = actions.back();
	*action.selectable = action.selected;
	project->group = action.group;

	const u32 actionIdx = (u32)actions.size() - 1;
	for (u32 idx = 0; idx < (u32)datas.size(); ++idx)
		datas.at(idx)->ReverseAction(actionIdx);
	actions.erase(actions.begin() + actionIdx);

	return true;
}

bool Engine::ReloadData()
{
	if (!ClearEngine())
	{
		Quit(RELOAD_CLEAR_QUIT);
		return false;
	}
	if (!LoadEngine())
	{
		Quit(RELOAD_LOAD_QUIT);
		return false;
	}
	return true;
}

void Engine::LoadPatchSettings()
{
	string patchSettingsPath = DEPLOY_PATCH_DIR;
	if (!PathExists(patchSettingsPath))
		patchSettingsPath = DEV_PATCH_DIR;

	patchSettingsPath = PathConcat(patchSettingsPath, "settings.h");
	LoadKlang(patchSettings, patchSettingsPath);
}

void Engine::InstallPatch(string settingsPath)
{
	string toolsDir = DEPLOY_TOOLS_DIR;
	if (!PathExists(toolsDir))
		toolsDir = DEV_TOOLS_DIR;

	string buildSettings = "SET PROJECT_DIR=";
	buildSettings += project->ctrMapProjectDir + PATH_SEPARATOR;
	buildSettings += '\n';

	buildSettings += "SET CTRMAP_DIR=";
	buildSettings += toolsDir + PATH_SEPARATOR;
	buildSettings += '\n';

	buildSettings += "SET ARM_NONE_EABI_DIR=";
	buildSettings += PathConcat(toolsDir, string("Arm-None-Eabi") + PATH_SEPARATOR + "bin" + PATH_SEPARATOR);
	buildSettings += '\n';

	buildSettings += "SET JAVA_DIR=";

	FileStream file;
	LoadEmptyFileStream(file);

	FileStreamBufferWriteBack(file, (u8*)buildSettings.c_str(), (u32)buildSettings.size());

	SaveFileStream(file, settingsPath);
}

string GetBuilderPath()
{
	string builderPath = DEPLOY_BUILDER_DIR;
	if (PathExists(builderPath))
		return builderPath;

	builderPath = DEV_BUILDER_DIR;
	builderPath = PathConcat(builderPath, "x64");
	builderPath = PathConcat(builderPath, "Release");
	return builderPath;
}

void Engine::BuildPatch()
{
	string builderPath = GetBuilderPath();
	string patchPath = DEPLOY_PATCH_DIR;
	if (!PathExists(patchPath))
		patchPath = DEV_PATCH_DIR;

	string command = PATH_FORMAT(PathConcat(builderPath, PATCH_BUILDER_FILE)) + " ";
	command += "-rebuild ";
	command += PATH_FORMAT(patchPath) + " ";
	command += "-whitelist-all ";
	command += "-custom-build ";
	command += PATH_FORMAT(project->path) + " ";
	command += "-pause";

	system(PATH_FORMAT(command).c_str());
}

void Engine::UninstallPatch()
{
	string builderPath = GetBuilderPath();
	string patchPath = DEPLOY_PATCH_DIR;
	if (!PathExists(patchPath))
		patchPath = DEV_PATCH_DIR;

	string command = PATH_FORMAT(PathConcat(builderPath, "PW2Builder.exe")) + " ";
	command += "-uninstall ";
	command += PATH_FORMAT(patchPath) + " ";
	command += "-custom-build ";
	command += PATH_FORMAT(project->path) + " ";
	command += "-keep-settings ";
	command += "-pause";

	system(PATH_FORMAT(command).c_str());
}

void LaunchCommand(const string& command)
{
	string buildScript = "@ECHO OFF\n";
	buildScript += command;
	buildScript += "\nPAUSE";

	FileStream fileStream;
	LoadEmptyFileStream(fileStream);
	FileStreamBufferWriteBack(fileStream, (u8*)buildScript.c_str(), (u32)buildScript.size());
	SaveFileStream(fileStream, TEMP_CMD);
	ReleaseFileStream(fileStream);

	system(TEMP_CMD);
}

void Engine::DownloadPW2Code()
{
	string patchPath = DEPLOY_PATCH_DIR;

	string command = "git clone --recursive ";
	command += PW2CODE_LINK;
	command += " " + PATH_FORMAT(patchPath);

	LaunchCommand(command);

	LoadPatchSettings();
}

void Engine::UpdatePW2Code()
{
	string patchPath = DEPLOY_PATCH_DIR;
	if (!PathExists(patchPath))
		return;

	string command = "cd ";
	command += PATH_FORMAT(patchPath) + '\n';
	command += "git fetch origin";
	command += '\n';
	command += "git reset --hard origin/main";

	LaunchCommand(command);

	LoadPatchSettings();
}

void PatchSetting(KlangVar& var, bool isChild)
{
	string text;
	if (isChild)
		text += "    ";
	text += var.name;

	ImGui::Text(text);
	ImGui::SameLine();

	bool value = (bool)var.Value();
	if (ImGui::Checkbox((string("##") + var.name).c_str(), &value))
		var.SetValue((int)value);
}

void Engine::MenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Project"))
		{
			ImGui::BeginDisabled(!UsavedData());
			if (ImGui::MenuItem("Save", "Ctrl+S"))
				Save();
			ImGui::EndDisabled();

			if (ImGui::MenuItem("Save & Reload", "Ctrl+R"))
			{
				Save();
				ReloadData();
			}

			if (ImGui::MenuItem("Save & Exit"))
			{
				Save();
				Quit();
			}

			if (ImGui::MenuItem("Undo", "Ctrl+Z"))
				ReverseAction();

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Editors"))
		{
			if (ImGui::MenuItem(u8"Pokémon", "Ctrl+1"))
				project->group = POKEMON_GROUP;
			if (ImGui::MenuItem("Items", "Ctrl+2"))
				project->group = ITEM_GROUP;
			if (ImGui::MenuItem("Moves", "Ctrl+3"))
				project->group = MOVE_GROUP;
			if (ImGui::MenuItem("Trainers", "Ctrl+4"))
				project->group = TRAINER_GROUP;
			if (ImGui::MenuItem("Encounters", "Ctrl+5"))
				project->group = ENCOUNTER_GROUP;
			ImGui::EndMenu();
		}

		ImGui::BeginDisabled(!enableBuilder || !PMCCheck());
		if (ImGui::BeginMenu("Patcher"))
		{
			if (!PW2CodeCheck())
			{
				if (ImGui::MenuItem("Download PW2Code"))
					DownloadPW2Code();
			}
			else
			{

				if (ImGui::MenuItem("Update PW2Code"))
					UpdatePW2Code();

				if (ImGui::MenuItem("Options"))
					patcherOptions = !patcherOptions;

				bool patchInstalled = PatchIsInstalled();

				string buildText = "Save + Build";
				if (patchInstalled)
					buildText = "Save + Rebuild";
				if (ImGui::MenuItem(buildText.c_str()))
				{
					Save();
					BuildPatch();
					ReloadData();
				}

				ImGui::BeginDisabled(!patchInstalled);
				if (ImGui::MenuItem("Save + Uninstall"))
				{
					Save();
					UninstallPatch();
					ReloadData();
				}
				ImGui::EndDisabled();
			}
			ImGui::EndMenu();
		}
		ImGui::EndDisabled();

		ImGui::EndMainMenuBar();
	}

	if (patcherOptions)
	{
		ImGui::Begin("Patcher Options", &patcherOptions);

		for (u32 setting = 0; setting < (u32)patchSettings.vars.size(); ++setting)
		{
			KlangVar& parent = patchSettings.vars[setting];
			PatchSetting(parent, false);

			ImGui::BeginDisabled(parent.Value() == 0);
			for (u32 child = 0; child < (u32)parent.dependentVars.size(); ++child)
			{
				KlangVar& dependent = parent.dependentVars[child];
				PatchSetting(dependent, true);
			}
			ImGui::EndDisabled();

			ImGui::Separator();
		}

		ImGui::End();
	}
}
