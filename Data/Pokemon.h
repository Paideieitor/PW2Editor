#ifndef _POKEMON_H
#define _POKEMON_H

#include <vector>

#include "Globals.h"

struct Pokemon
{
	// PokéDex index
	u32 idx = 0;
	// PML index
	u32 dataIdx = 0;
	// Text index (certain text files exclude PokéStudio Pokémon)
	u32 textIdx = 0;

	bool pokeStudio = false;

	vector<Pokemon> forms = vector<Pokemon>();
};
bool PokemonIsForm(const Pokemon& pkm);

bool PokemonFormHasData(const Pokemon& pkm);
bool PokemonFormHasData(const Pokemon* pkm);


#endif // _POKEMON_H