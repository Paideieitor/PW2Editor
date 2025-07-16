#include "Utils/DataUtils.h"

u32 Make(u16 high, u16 low)
{
	u32 output = (u32)high << 16;
	return output + low;
}

u16 High(u32 value)
{
	return (u16)((value & 0xFFFF0000) >> 16);
}

u16 Low(u32 value)
{
	return (u16)(value & 0x0000FFFF);
}

u64 LongMake(u32 high, u32 low)
{
	u64 output = (u64)high << 32;
	return output + low;
}

u32 LongHigh(u64 value)
{
	return (u32)((value & 0xFFFFFFFF00000000) >> 32);
}

u32 LongLow(u64 value)
{
	return (u32)(value & 0x00000000FFFFFFFF);
}