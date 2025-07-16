#ifndef _DATA_UTILS_H
#define _DATA_UTILS_H

#include "Globals.h"

u32 Make(u16 high, u16 low);
u16 High(u32 value);
u16 Low(u32 value);

u64 LongMake(u32 high, u32 low);
u32 LongHigh(u64 value);
u32 LongLow(u64 value);

#endif // _DATA_UTILS_H