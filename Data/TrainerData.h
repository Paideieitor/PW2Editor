#ifndef _TRAINER_DATA_H
#define _TRAINER_DATA_H

#include <array>
#include <vector>

enum TrainerField
{
	TRAINER_TYPE,
	TRAINER_CLASS,
	BATTLE_TYPE,
	POKEMON_COUNT,
	ITEM_1,
	ITEM_2,
	ITEM_3,
	ITEM_4,
	AI_NO_EFFECT_CHECK,
	AI_EVALUATE_MOVES,
	AI_EXPERT,
	AI_SET_UP,
	AI_FIRST_RIVAL,
	AI_N_S_FINAL_BATTLE,
	AI_BATON_PASS,
	AI_DOUBLE_TRIPLE_BATTLE,
	AI_HP_BASED,
	AI_WEATHER,
	AI_DISRUPTION,
	AI_ROAMING,
	AI_SAFARI,
	AI_TUTORIAL,
	AI_UNUSED_14,
	AI_UNUSED_15,
	AI_UNUSED_16,
	AI_UNUSED_17,
	AI_UNUSED_18,
	AI_UNUSED_19,
	AI_UNUSED_20,
	AI_UNUSED_21,
	AI_UNUSED_22,
	AI_UNUSED_23,
	AI_UNUSED_24,
	AI_UNUSED_25,
	AI_UNUSED_26,
	AI_UNUSED_27,
	AI_UNUSED_28,
	AI_UNUSED_29,
	AI_UNUSED_30,
	AI_UNUSED_31,
	IS_HEALER,
	CASH,
	POST_BATTLE_ITEM,
	TRAINERDATA_MAX,
};

typedef std::array<int, TRAINERDATA_MAX> TrainerData;

void TrainerReset(TrainerData& trainer);

enum TrainerPokemonField
{
	TRAINER_DIFFICULTY,
	TRAINER_SEX,
	TRAINER_ABILITY,
	TRAINER_LEVEL,
	TRAINER_SPECIES,
	TRAINER_FORM,
	TRAINER_HELD_ITEM,
	TRAINER_MOVE_1,
	TRAINER_MOVE_2,
	TRAINER_MOVE_3,
	TRAINER_MOVE_4,
	TRAINERPOKEMONDATA_MAX,
};

enum TrainerType
{
	SIMPLE_TRAINER,
	MOVE_TRAINER,
	ITEM_TRAINER,
	GOOD_TRAINER,
	PERFECT_TRAINER,
};

#define MAX_TEAM_SIZE 6
typedef std::array<int, TRAINERPOKEMONDATA_MAX * MAX_TEAM_SIZE> TrainerTeamData;

void ResetTrainerPokemon(TrainerTeamData& team, u32 slot);
void DefaultTrainerPokemon(TrainerTeamData& team, u32 slot);

void SwapTrainerPokemon(TrainerTeamData& team, u32 slot1, u32 slot2);
void RemoveTrainerPokemon(TrainerTeamData& team, u32 slot);
void InsertTrainerPokemon(TrainerTeamData& team, u32 slot);

#define TRAINER_NULL -1
#define TEAM_SLOT(slot, field) (slot * TRAINERPOKEMONDATA_MAX + field)

#endif // _TRAINER_DATA_H