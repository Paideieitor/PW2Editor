#ifndef _POKEMON_TEXT_H
#define _POKEMON_TEXT_H

#include <vector>
#include <string>

#include "Globals.h"

#include "Modules/Module.h"

class PokemonText : public Module
{
public:
	PokemonText() = delete;
	PokemonText(Engine* const engine, u32 group);
	~PokemonText();

	virtual ReturnState RenderGUI() override;

	virtual void HandleReverseEvent(const Event* reverseEvent) override;
	virtual void HandleSaveEvent(Event* saveEvent) override;

private:

	void TextInput(const char* label, string* text, u32 fileID, const vector<string>& list);
	void InputTextMultiline(const char* label, string* text, u32 fileID, const vector<string>& list);
};

#endif // _POKEMON_TEXT_H