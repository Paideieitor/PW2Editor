#include "Globals.h"

#include "Data/ZoneData.h"

void ZoneReset(ZoneData& zoneData)
{
	for (u32 idx = 0; idx < (u32)zoneData.size(); ++idx)
		zoneData[idx] = ZONEDATA_NULL;
}