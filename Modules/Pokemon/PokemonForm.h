#ifndef _POKEMON_FORM_H
#define _POKEMON_FORM_H

#include "Globals.h"

#include "Modules/Module.h"

class PokemonForm : public Module
{
public:
	PokemonForm() = delete;
	PokemonForm(Engine* const engine, u32 group);
	~PokemonForm();

	virtual ReturnState RenderGUI() override;

	virtual void HandleReverseEvent(const Event* reverseEvent) override;
	virtual void HandleSaveEvent(Event* saveEvent) override {}
};

#endif // _POKEMON_FORM_H