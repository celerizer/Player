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

#ifndef EP_SCENE_BATTLE_RPG2K_H
#define EP_SCENE_BATTLE_RPG2K_H

// Headers
#include "scene_battle.h"
#include "game_enemy.h"

#include "window_command.h"
#include "window_battlemessage.h"

/**
 * Scene_Battle class.
 * Manages the battles.
 */
class Scene_Battle_Rpg2k : public Scene_Battle {
public:
	enum BattleActionState {
		/**
		 * 1st action, called repeatedly.
		 * Handles healing of conditions that get auto removed after X turns.
		 */
		BattleActionState_ConditionHeal,
		/**
		 * 2nd action, called once.
		 * Used to execute the algorithm and print the first start line.
		 */
		BattleActionState_Execute,
		/**
		 * 3rd action, called once.
		 * Used to apply the new conditions, play an optional battle animation and sound, and print the second line of a technique.
		 */
		BattleActionState_Apply,
		/**
		* 4th action, called repeatedly.
		* Used for the results, concretely wait a few frames and pop the messages.
		*/
		BattleActionState_ResultPop,
		/**
		 * 5th action, called repeatedly.
		 * Used to push the message results, effects and advances the messages. If it finishes, it calls Death. If not, it calls ResultPop
		 */
		BattleActionState_ResultPush,
		/**
		 * 6th action, called once.
		 * Action treating whether the enemy died or not.
		 */
		BattleActionState_Death,
		/**
		 * 7th action, called once.
		 * It finishes the action and checks whether to repeat it if there is another target to hit.
		 */
		BattleActionState_Finished
	};



public:
	Scene_Battle_Rpg2k();
	~Scene_Battle_Rpg2k() override;

	void Update() override;

protected:
	void SetState(State new_state) override;

	void NextTurn();

	void CreateUi() override;

	void CreateBattleTargetWindow();
	void CreateBattleCommandWindow();

	bool CheckWin();
	bool CheckLose();
	bool CheckResultConditions();

	void RefreshCommandWindow();

	void ProcessActions() override;

	bool ProcessBattleAction(Game_BattleAlgorithm::AlgorithmBase* action);
	void ProcessInput() override;

	/**
	 * Adds a message about the gold received into
	 * Game_Message::texts.
	 *
	 * @param money Number of gold to display.
	 */
	void PushGoldReceivedMessage(int money);

	/**
	 * Adds a message about the experience received into
	 * Game_Message::texts.
	 *
	 * @param exp Number of experience to display.
	 */
	void PushExperienceGainedMessage(int exp);

	/**
	 * Adds messages about the items obtained after the battle
	 * into Game_Message::texts.
	 *
	 * @param drops Vector of item IDs
	 */
	void PushItemRecievedMessages(std::vector<int> drops);

	void OptionSelected();
	void CommandSelected();

	void Escape();

	void SelectNextActor();
	void SelectPreviousActor();

	/**
	 * Gets the time during before hiding a windowful of
	 * text.
	 *
	 * @return int seconds to wait
	 */
	int GetDelayForWindow();

	/**
	 * Gets delay between showing two lines of text.
	 *
	 * @return int seconds to wait
	 */
	int GetDelayForLine();

	void CreateExecutionOrder();
	void CreateEnemyActions();

	bool DisplayMonstersInMessageWindow();

	bool ProcessActionConditionHeal(Game_BattleAlgorithm::AlgorithmBase* action);

	std::unique_ptr<Window_BattleMessage> battle_message_window;
	std::vector<std::string> battle_result_messages;
	std::vector<std::string>::iterator battle_result_messages_it;
	int battle_action_wait = 0;
	int battle_action_state = BattleActionState_ConditionHeal;

	int select_target_flash_count = 0;
	bool encounter_message_first_monster = true;
	int encounter_message_wait = 0;
	bool encounter_message_first_strike = false;

	bool battle_action_pending = false;
	bool begin_escape = true;
	bool escape_success = false;
	int escape_counter = 0;

	bool message_box_got_visible = false;
	bool move_screen = false;

	int last_turn_check = -1;
};

#endif
