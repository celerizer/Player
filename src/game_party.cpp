/*
 * This file is part of EasyRPG Player.
 *
 * EasyRPG Player is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * EasyRPG Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
 */

// Headers
#include <algorithm>
#include "system.h"
#include "game_party.h"
#include "game_actors.h"
#include "game_map.h"
#include "game_player.h"
#include "game_battle.h"
#include "output.h"
#include "util_macro.h"

static RPG::SaveInventory& data = Main_Data::game_data.inventory;

Game_Party_Class::Game_Party_Class() {
	data.Setup();
}

Game_Battler* Game_Party_Class::GetBattler(int index) {
	std::vector<Game_Actor*> actors = GetActors();

	if (index < 0 || index >= actors.size()) {
		return NULL;
	}

	return actors[index];
}

int Game_Party_Class::GetBattlerCount() const {
	return GetActors().size();
}

void Game_Party_Class::SetupBattleTestMembers() {
	data.party.clear();
	
	std::vector<RPG::TestBattler>::const_iterator it;
	for (it = Data::system.battletest_data.begin();
		it != Data::system.battletest_data.end(); ++it) {
		AddActor(it->ID);
		Game_Actor* actor = Game_Actors::GetActor(it->ID);
		actor->SetEquipment(0, it->weapon_id);
		actor->SetEquipment(1, it->shield_id);
		actor->SetEquipment(1, it->armor_id);
		actor->SetEquipment(1, it->helmet_id);
		actor->SetEquipment(1, it->accessory_id);
		actor->SetLevel(it->level);
	}

	Main_Data::game_player->Refresh();
}

void Game_Party_Class::GetItems(std::vector<int>& item_list) {
	item_list.clear();

	std::vector<int16_t>::iterator it;
	for (it = data.item_ids.begin(); it != data.item_ids.end(); it++)
		item_list.push_back(*it);
}

int Game_Party_Class::ItemNumber(int item_id, bool get_equipped) {
	if (get_equipped && item_id > 0) {
		int number = 0;
		for (int i = 0; i < (int) data.party.size(); i++) {
			Game_Actor* actor = Game_Actors::GetActor(data.party[i]);
			if (actor->GetWeaponId() == item_id) {
				++number;
			}
			if (actor->GetShieldId() == item_id) {
				++number;
			}
			if (actor->GetArmorId() == item_id) {
				++number;
			}
			if (actor->GetHelmetId() == item_id) {
				++number;
			}
			if (actor->GetAccessoryId() == item_id) {
				++number;
			}
		}
		return number;
	} else {
		for (int i = 0; i < (int) data.item_ids.size(); i++)
			if (data.item_ids[i] == item_id)
				return data.item_counts[i];
	}
	
	return 0;
}

void Game_Party_Class::GainGold(int n) {
	data.gold += n;
	data.gold = std::min(std::max(data.gold, 0), 999999);
}

void Game_Party_Class::LoseGold(int n) {
	data.gold -= n;
	data.gold = std::min(std::max(data.gold, 0), 999999);
}

void Game_Party_Class::GainItem(int item_id, int amount) {
	if (item_id < 1 || item_id > (int) Data::items.size()) {
		Output::Warning("Can't add item to party.\n%04d is not a valid item ID.",
						item_id);
		return;
	}

	for (int i = 0; i < (int) data.item_ids.size(); i++) {
		if (data.item_ids[i] != item_id)
			continue;

		int total_items = data.item_counts[i] + amount;

		if (total_items <= 0) {
			data.item_ids.erase(data.item_ids.begin() + i);
			data.item_counts.erase(data.item_counts.begin() + i);
			data.item_usage.erase(data.item_usage.begin() + i);
			return;
		}

		data.item_counts[i] = std::min(total_items, 99);
		return;
	}

	// Item isn't in the inventory yet
	
	if (amount <= 0) {
		return;
	}

	data.item_ids.push_back(item_id);
	data.item_counts.push_back(std::min(amount, 99));
	data.item_usage.push_back(Data::items[item_id - 1].uses);
}

void Game_Party_Class::LoseItem(int item_id, int amount) {
	GainItem(item_id, -amount);
}

bool Game_Party_Class::IsItemUsable(int item_id) {
	if (item_id > 0 && item_id <= (int)Data::items.size()) {
		//TODO: if (Game_Temp::IsInBattle()) {
		//if (Data::items[item_id - 1].type == RPG::Item::Type_medicine) {
		//	return !Data::items[item_id - 1].ocassion_field;
		//} else if (Data::items[item_id - 1].type == RPG::Item::Type_switch) {
		//	return Data::items[item_id - 1].ocassion_battle;
		//} else {
		if (data.party.size() > 0 &&
			(Data::items[item_id - 1].type == RPG::Item::Type_medicine ||
			Data::items[item_id - 1].type == RPG::Item::Type_material ||
			Data::items[item_id - 1].type == RPG::Item::Type_book)) {
			return true;
		} else if (Data::items[item_id - 1].type == RPG::Item::Type_switch) {
			return Data::items[item_id - 1].occasion_field2;
		}
	}

	return false;
}

void Game_Party_Class::AddActor(int actor_id) {
	if (IsActorInParty(actor_id))
		return;
	if (data.party.size() >= 4)
		return;
	data.party.push_back(actor_id);
	Main_Data::game_player->Refresh();
}

void Game_Party_Class::RemoveActor(int actor_id) {
	if (!IsActorInParty(actor_id))
		return;
	data.party.erase(std::find(data.party.begin(), data.party.end(), actor_id));
	Main_Data::game_player->Refresh();
}

bool Game_Party_Class::IsActorInParty(int actor_id) {	
	return std::find(data.party.begin(), data.party.end(), actor_id) != data.party.end();
}

int Game_Party_Class::GetGold() {
	return data.gold;
}

int Game_Party_Class::GetSteps() {
	return data.steps;
}

std::vector<Game_Actor*> Game_Party_Class::GetActors() const {
	std::vector<Game_Actor*> actors;
	std::vector<int16_t>::const_iterator it;
	for (it = data.party.begin(); it != data.party.end(); it++)
		actors.push_back(Game_Actors::GetActor(*it));
	return actors;
}

int Game_Party_Class::GetBattleCount() {
	return data.battles;
}

int Game_Party_Class::GetWinCount() {
	return data.victories;
}

int Game_Party_Class::GetDefeatCount() {
	return data.defeats;
}

int Game_Party_Class::GetRunCount() {
	return data.escapes;
}


void Game_Party::ApplyDamage(int damage) {
	if (damage <= 0) {
		return;
	}

	std::vector<Game_Actor*> actors = GetActors();

	for (std::vector<Game_Actor*>::iterator i = actors.begin(); i != actors.end(); i++) {
		Game_Actor* actor = *i;
		actor->SetHp(actor->GetHp() - damage);
	}
}

void Game_Party_Class::SetTimer(int which, int seconds) {
	switch (which) {
		case Timer1:
			data.timer1_secs = seconds * DEFAULT_FPS;
			Game_Map::SetNeedRefresh(true);
			break;
		case Timer2:
			data.timer2_secs = seconds * DEFAULT_FPS;
			Game_Map::SetNeedRefresh(true);
			break;
	}
}

void Game_Party_Class::StartTimer(int which, bool visible, bool battle) {
	switch (which) {
		case Timer1:
			data.timer1_active = true;
			data.timer1_visible = visible;
			data.timer1_battle = battle;
			break;
		case Timer2:
			data.timer2_active = true;
			data.timer2_visible = visible;
			data.timer2_battle = battle;
			break;
	}
}

void Game_Party_Class::StopTimer(int which) {
	switch (which) {
		case Timer1:
			data.timer1_active = false;
			data.timer1_visible = false;
			break;
		case Timer2:
			data.timer2_active = false;
			data.timer2_visible = false;
			break;
	}
}

void Game_Party_Class::UpdateTimers() {
	bool battle = Game_Battle::GetScene() != NULL;
	if (data.timer1_active && (!data.timer1_battle || !battle) && data.timer1_secs > 0) {
		data.timer1_secs--;
		Game_Map::SetNeedRefresh(true);
	} 
	if (data.timer2_active && (!data.timer2_battle || !battle) && data.timer2_secs > 0) {
		data.timer2_secs--;
		Game_Map::SetNeedRefresh(true);
	}
}

int Game_Party_Class::ReadTimer(int which) {
	switch (which) {
		case Timer1:
			return data.timer1_secs;
		case Timer2:
			return data.timer2_secs;
		default:
			return 0;
	}
}

Game_Party_Class& Game_Party() {
	static bool init = false;
	static boost::scoped_ptr<Game_Party_Class> instance;
	if (!init) {
		instance.reset(new Game_Party_Class());
		init = true;
	}
	return *instance;
}
