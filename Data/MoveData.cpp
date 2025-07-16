#include "Globals.h"

#include "Data/MoveData.h"

void MoveDataReset(MoveData& moveData)
{
	for (u32 idx = 0; idx < (u32)moveData.size(); ++idx)
		moveData[idx] = MOVEDATA_NULL;
}
