#include "Globals.h"

#include "Data/TrainerData.h"

void TrainerReset(TrainerData& trainer)
{
	for (u32 idx = 0; idx < (u32)trainer.size(); ++idx)
		trainer[idx] = TRAINER_NULL;
}

void ResetTrainerPokemon(TrainerTeamData& team, u32 slot)
{
	if (slot >= MAX_TEAM_SIZE)
		return;

	for (u32 field = TRAINER_DIFFICULTY; field < TRAINERPOKEMONDATA_MAX; ++field)
		team[TEAM_SLOT(slot, field)] = TRAINER_NULL;
}

void DefaultTrainerPokemon(TrainerTeamData& team, u32 slot)
{
	if (slot >= MAX_TEAM_SIZE)
		return;

	for (u32 field = TRAINER_DIFFICULTY; field < TRAINERPOKEMONDATA_MAX; ++field)
		team[TEAM_SLOT(slot, field)] = 0;
}

void SwapTrainerPokemon(TrainerTeamData& team, u32 slot1, u32 slot2)
{
	if (slot1 >= MAX_TEAM_SIZE || slot2 >= MAX_TEAM_SIZE)
		return;

	for (u32 field = TRAINER_DIFFICULTY; field < TRAINERPOKEMONDATA_MAX; ++field)
	{
		int temp = team[TEAM_SLOT(slot1, field)];
		team[TEAM_SLOT(slot1, field)] = team[TEAM_SLOT(slot2, field)];
		team[TEAM_SLOT(slot2, field)] = temp;
	}
}

void RemoveTrainerPokemon(TrainerTeamData& team, u32 slot)
{
	if (slot >= MAX_TEAM_SIZE)
		return;

	ResetTrainerPokemon(team, slot);
	for (u32 idx = slot; idx < MAX_TEAM_SIZE - 1; ++idx)
		SwapTrainerPokemon(team, idx, idx + 1);
}

void InsertTrainerPokemon(TrainerTeamData& team, u32 slot)
{
	if (slot >= MAX_TEAM_SIZE)
		return;

	DefaultTrainerPokemon(team, MAX_TEAM_SIZE - 1);
	for (int idx = MAX_TEAM_SIZE - 2; idx >= (int)slot; --idx)
		SwapTrainerPokemon(team, idx, idx + 1);
}
