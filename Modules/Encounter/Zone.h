#ifndef _ZONE_H
#define _ZONE_H

#include <vector>
#include <string>

#include "Globals.h"

#include "Modules/Module.h"

#include "Data/ZoneData.h"

class Zone : public Module
{
public:
	Zone() = delete;
	Zone(Engine* const engine, u32 group);
	~Zone();

	virtual ReturnState RenderGUI() override;

	virtual void HandleReverseEvent(const Event* reverseEvent) override;
	void HandleSaveEvent(Event* saveEvent) override;

private:

	void SwapLocations(int previous, int next);

	void LocationComboBox(ZoneData& zoneData);

	int selectedIdx = -1;

	u32 prevLocation = 0;
};

#endif // _ZONE_H
