#include "Globals.h"

#include "Data/LearnsetData.h"

void LearnsetReset(LearnsetData& learnset)
{
	for (u32 moveIdx = 0; moveIdx < (u32)learnset.size(); ++moveIdx)
		for (u32 idx = 0; idx < (u32)learnset[idx].size(); ++idx)
			learnset[moveIdx][idx] = LEARNSET_NULL;
}
