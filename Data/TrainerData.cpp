#include "Globals.h"

#include "Data/TrainerData.h"

void TrainerReset(TrainerData& trainer)
{
	for (u32 idx = 0; idx < (u32)trainer.size(); ++idx)
		trainer[idx] = TRAINER_NULL;
}