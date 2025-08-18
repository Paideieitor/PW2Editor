#include "Windows/Engine.h"

#include "Modules/Item.h"

#define ITEM_INPUT_INT(field, label, max, min) engine->InputInt(engine->items, engine->project->item, field, &engine->project->item, engine->project->item, group, label, max, min)
#define ITEM_COMBO_BOX(field, label, _list) engine->ComboBox(engine->items, engine->project->item, field, &engine->project->item, engine->project->item, group, label, _list)
#define ITEM_CHECK_BOX(field, label) engine->CheckBox(engine->items, engine->project->item, field, &engine->project->item, engine->project->item, group, label)

#define ITEM_GET_VALUE(field) engine->GetDataValue(engine->items, engine->project->item, field)
#define ITEM_SET_VALUE(field, value) engine->SetDataValue(engine->items, engine->project->item, field, value)
#define ITEM_START_ACTION engine->StartAction(&engine->project->item, engine->project->item, group)
#define ITEM_END_ACTION engine->EndAction()

Item::Item(Engine* const engine, u32 group) : Module(engine, group),
types(engine->GetTextFile(TYPE_NAME_FILE_ID))
{
}

ReturnState Item::RenderGUI()
{
	ImGui::Begin("Item");
	ImGui::Text("Index: " + to_string(engine->project->item));
	ImGui::Separator();

	ImGui::BeginGroup();
	{
		ImGui::Text("General");
		ITEM_INPUT_INT(PRICE, "Price", 0xFFFF, 0);

		ITEM_INPUT_INT(HELD_EFFECT, "Held Effect", 255, 0);
		ITEM_INPUT_INT(HELD_PARAM, "Held Param", 255, 0);

		ImGui::Text("Fling");
		ITEM_INPUT_INT(FLING_EFFECT, "Effect", 255, 0);
		ITEM_INPUT_INT(FLING_POWER, "Power", 255, 0);

		ImGui::Text("Natural Gift");
		ITEM_COMBO_BOX(NATURAL_GIFT_TYPE, "Type", types);
		if (ITEM_GET_VALUE(ITEM_FIELD(NATURAL_GIFT_TYPE)) != 31)
		{
			ImGui::SameLine();
			if (ImGui::Button("X##NatGift"))
			{
				ITEM_START_ACTION;
				ITEM_SET_VALUE(ITEM_FIELD(NATURAL_GIFT_TYPE), 31);
				ITEM_END_ACTION;
			}
		}
		ITEM_INPUT_INT(NATURAL_GIFT_POWER, "Power", 255, 0);
		ITEM_INPUT_INT(NATURAL_GIFT_EFFECT, "Effect", 255, 0);

		ImGui::Text("Bag");
		ITEM_CHECK_BOX(USABLE_FLAG, "Usable");
		ITEM_CHECK_BOX(REGISTRABLE_FLAG, "Registrable");
		ITEM_COMBO_BOX(POCKET_FIELD, "Pocket Field", &pockets);
		ITEM_INPUT_INT(SORT_IDX, "Sort Index", 255, 0);

		ImGui::Text("Battle");
		ITEM_CHECK_BOX(HAS_BATTLE_STATS, "Has Battle Stats");
		ITEM_CHECK_BOX(CONSUMABLE, "Consumable");
		ITEM_INPUT_INT(EFFECT_FIELD, "Effect Field", 255, 0);
		ITEM_INPUT_INT(EFFECT_BATTLE, "Effect Battle", 255, 0);
		ITEM_INPUT_INT(BATTLE_POCKET, "Battle Pocket", 255, 0);
		
		ImGui::Text("Unused");
		ITEM_INPUT_INT(ITEM_UNUSED_1, "Unused 1", 255, 0);
		ITEM_INPUT_INT(ITEM_UNUSED_2, "Unused 2", 255, 0);
	}
	ImGui::EndGroup();

	ImGui::SameLine();

	ImGui::BeginGroup();
	{
		ImGui::Text("Heal Conditions");
		ImGui::BeginGroup();
		{
			ITEM_CHECK_BOX(CURE_SLEEP, "Cure Sleep");
			ITEM_CHECK_BOX(CURE_POISON, "Cure Poison");
			ITEM_CHECK_BOX(CURE_BURN, "Cure Burn");
			ITEM_CHECK_BOX(CURE_FREEZE, "Cure Freeze");
		}
		ImGui::EndGroup();
		ImGui::SameLine();
		ImGui::BeginGroup();
		{
			ITEM_CHECK_BOX(CURE_PARALYSIS, "Cure Paralysis");
			ITEM_CHECK_BOX(CURE_CONFUSION, "Cure Confusion");
			ITEM_CHECK_BOX(CURE_INFATUATION, "Cure Infatuation");
			ITEM_CHECK_BOX(CURE_GSP, "Cure Stat Drops");
		}
		ImGui::EndGroup();
	}
	ImGui::EndGroup();

	ImGui::SameLine();
	
	ImGui::BeginGroup();
	{
		ImGui::BeginGroup();
		{
			ImGui::Text("Boost Effects");
			ITEM_CHECK_BOX(BOOST_REVIVE, "Revive");
			ITEM_CHECK_BOX(BOOST_SACRED_ASH, "Sacred Ash");
			ITEM_CHECK_BOX(BOOST_RARECANDY, "Rarecandy");
			ITEM_CHECK_BOX(BOOST_EVOSTONE, "Evostone");

			ImGui::BeginGroup();
			{
				ITEM_INPUT_INT(BOOST_ATK, "Boost Attack", 15, 0);
				ITEM_INPUT_INT(BOOST_DEF, "Boost Defense", 15, 0);
				ITEM_INPUT_INT(BOOST_SPE, "Boost Speed", 15, 0);
			}
			ImGui::EndGroup();
			ImGui::SameLine();
			ImGui::BeginGroup();
			{
				ITEM_INPUT_INT(BOOST_SPA, "Boost Sp.Attack", 15, 0);
				ITEM_INPUT_INT(BOOST_SPD, "Boost Sp.Defense", 15, 0);
				ITEM_INPUT_INT(BOOST_ACC, "Boost Accuracy", 15, 0);
			}
			ImGui::EndGroup();

			ITEM_INPUT_INT(BOOST_CRIT, "Boost Crit", 3, 0);
			ITEM_CHECK_BOX(BOOST_PP, "Boost PP");
			ITEM_CHECK_BOX(BOOST_PP_MAX, "Boost PP Max");
		}
		ImGui::EndGroup();

		ImGui::BeginGroup();
		{
			ImGui::Text("Boost Values");
			ImGui::BeginGroup();
			{
				ITEM_INPUT_INT(ITEM_EV_HP, "HP EVs", 255, 0);
				ITEM_INPUT_INT(ITEM_EV_ATK, "Attack EVs", 255, 0);
				ITEM_INPUT_INT(ITEM_EV_DEF, "Defense EVs", 255, 0);
			}
			ImGui::EndGroup();
			ImGui::SameLine();
			ImGui::BeginGroup();
			{
				ITEM_INPUT_INT(ITEM_EV_SPE, "Speed EVs", 255, 0);
				ITEM_INPUT_INT(ITEM_EV_SPA, "Sp.Attack EVs", 255, 0);
				ITEM_INPUT_INT(ITEM_EV_SPD, "Sp.Defense EVs", 255, 0);
			}
			ImGui::EndGroup();

			ITEM_INPUT_INT(HEAL_AMOUNT, "Heal Amount", 255, 0);

			ITEM_INPUT_INT(PP_GAIN, "PP Gain", 255, 0);

			ITEM_INPUT_INT(FRIENDSHIP_1, "Friendship 1", 255, 0);
			ITEM_INPUT_INT(FRIENDSHIP_2, "Friendship 2", 255, 0);
			ITEM_INPUT_INT(FRIENDSHIP_3, "Friendship 3", 255, 0);
		}
		ImGui::EndGroup();
	}
	ImGui::EndGroup();

	ImGui::SameLine();
	
	ImGui::BeginGroup();
	{
		ImGui::Text("Add Stats");
		ITEM_CHECK_BOX(PP_REPLENISH, "PP Replenish");
		ITEM_CHECK_BOX(PP_REPLENISH_FULL, "PP Replenish Full");
		ITEM_CHECK_BOX(HP_REPLENISH, "HP Replenish");
		ImGui::BeginGroup();
		{
			ITEM_CHECK_BOX(EV_ADD_HP, "Add HP EVs");
			ITEM_CHECK_BOX(EV_ADD_ATK, "Add Attack EVs");
			ITEM_CHECK_BOX(EV_ADD_DEF, "Add Defense EVs");
		}
		ImGui::EndGroup();
		ImGui::SameLine();
		ImGui::BeginGroup();
		{
			ITEM_CHECK_BOX(EV_ADD_SPE, "Add Speed EVs");
			ITEM_CHECK_BOX(EV_ADD_SPA, "Add Sp.Attack EVs");
			ITEM_CHECK_BOX(EV_ADD_SPD, "Add Sp.Defense EVs");
		}
		ImGui::EndGroup();
		ITEM_CHECK_BOX(EV_ADD_ABOVE_100, "Add EVs above 100");
		ITEM_CHECK_BOX(FRIENDSHIP_ADD_1, "Add Friendship 1");
		ITEM_CHECK_BOX(FRIENDSHIP_ADD_2, "Add Friendship 2");
		ITEM_CHECK_BOX(FRIENDSHIP_ADD_3, "Add Friendship 3");
	}
	ImGui::EndGroup();

	ImGui::End();
	return OK;
}
