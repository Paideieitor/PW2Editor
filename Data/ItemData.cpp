#include "Globals.h"

#include "Data/ItemData.h"

void ItemDataReset(ItemData& itemData)
{
	for (u32 idx = 0; idx < (u32)itemData.size(); ++idx)
		itemData[idx] = ITEMDATA_NULL;
}
