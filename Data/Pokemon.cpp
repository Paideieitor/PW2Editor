#include "Data/Pokemon.h"

bool PokemonIsForm(const Pokemon& pkm)
{
	if (pkm.pokeStudio)
		return false;
	if (pkm.idx == pkm.dataIdx && pkm.idx == pkm.textIdx)
		return false;
	return true;
}

bool PokemonFormHasData(const Pokemon& pkm)
{
	if (pkm.idx == pkm.dataIdx)
		return false;
	return true;
}
bool PokemonFormHasData(const Pokemon* pkm)
{
	if (pkm->idx == pkm->dataIdx)
		return false;
	return true;
}
