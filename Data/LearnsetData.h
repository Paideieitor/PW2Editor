#ifndef _LEARNSET_DATA_H
#define _LEARNSET_DATA_H

#include <array>

#define LEARNSET_DATA_MAX 64

enum LearnsetField
{
	LEARN_MOVE_ID,
	LEARN_LEVEL,
	LEARNSET_MAX,
};
typedef std::array<int, LEARNSET_MAX> LearnMove;
typedef std::array<LearnMove, LEARNSET_DATA_MAX> LearnsetData;

void LearnsetReset(LearnsetData& learnset);

#define LEARNSET_NULL 0xFFFF
#define LEARNSET_SKIP 0xFFFE

#endif // _LEARNSET_DATA_H