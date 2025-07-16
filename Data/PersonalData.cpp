#include "Globals.h"

#include "Data/PersonalData.h"

void PersonalReset(PersonalData& personal)
{
	for (u32 idx = 0; idx < (u32)personal.size(); ++idx)
		personal[idx] = PERSONAL_NULL;
}
