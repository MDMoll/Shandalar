#include "manalink.h"

/*
 * Bulk battlefield manipulation and damage helpers.
 *
 * This file centralizes effects that apply the same action to many cards:
 * destroying, bouncing, tapping, adding counters, detaining, damaging all
 * creatures, and similar sweep-style operations.
 *
 * The important contract is that target discovery is separated from mutation.
 * First, matching cards are marked.  Then the action is applied to the marked
 * cards.  That two-phase structure protects loops from cards leaving play,
 * changing zones, or changing card indexes while the search is still running.
 */

enum {
	MAX_PLAYERS = 2,
	MAX_CARD_SLOTS = 150,
	MAX_TARGETS = 19
};

static int is_real_player(int player)
{
	return player >= HUMAN && player <= AI;
}

static int is_player_selector(int player)
{
	return is_real_player(player) || player == ANYBODY;
}

static int is_card_slot(int card)
{
	return card >= 0 && card < MAX_CARD_SLOTS;
}

static int is_card_ref(int player, int card)
{
	return is_real_player(player) && is_card_slot(card);
}

static int player_matches_selector(int selector, int player)
{
	return selector == ANYBODY || selector == player;
}

static int bounded_active_cards_count(int player)
{
	return is_real_player(player) ? MIN(active_cards_count[player], MAX_CARD_SLOTS) : 0;
}

/*
 * Applies a cooked, non-parameterized action to one concrete card slot.
 *
 * raw_action must be decoded by cook_action() before reaching this function.
 * Keep validation here intentionally small: some actions are valid for cards in
 * hand as well as cards in play, and callers decide which zones are eligible.
 */
static int action_on_card_impl(int player, int card, int p, int c, actions_t action, counter_t counter_type, int counter_num)
{
	if (!is_card_ref(p, c)) {
		return 0;
	}

	switch (action)
	{
		case ACT_KILL_DESTROY:
		case ACT_KILL_BURY:
			if (!check_for_special_ability(p, c, SP_KEYWORD_INDESTRUCTIBLE)) {
				kill_card(p, c, action);
				return 1;
			}
			return 0;

		case ACT_KILL_SACRIFICE:
		case ACT_KILL_REMOVE:
		case ACT_KILL_STATE_BASED_ACTION:
			kill_card(p, c, action);
			return 1;

		case ACT_BOUNCE:
			bounce_permanent(p, c);
			return 1;

		case ACT_TAP:
			if (!is_tapped(p, c)) {
				tap_card(p, c);
				return 1;
			}
			return 0;

		case ACT_UNTAP:
			if (is_tapped(p, c)) {
				untap_card(p, c);
				return 1;
			}
			return 0;

		case ACT_RFG_UNTIL_EOT:
			return remove_until_eot(player, card, p, c) != -1 ? 1 : 0;

		case ACT_PUT_ON_TOP:
			put_on_top_of_deck(p, c);
			return 1;

		case ACT_PUT_ON_BOTTOM:
			put_on_bottom_of_deck(p, c);
			return 1;

		case ACT_DISABLE_NONMANA_ACTIVATED_ABILITIES:
			disable_nonmana_activated_abilities(p, c, 1);
			return 1;

		case ACT_ENABLE_NONMANA_ACTIVATED_ABILITIES:
			disable_nonmana_activated_abilities(p, c, 0);
			return 1;

		case ACT_HUMILIATE:
			return humiliate(player, card, p, c, 2) != -1 ? 1 : 0;

		case ACT_DE_HUMILIATE:
			return humiliate(player, card, p, c, 0) != -1 ? 1 : 0;

		case ACT_MAKE_UNTARGETTABLE:
			state_untargettable(p, c, 1);
			return 1;

		case ACT_REMOVE_UNTARGETTABLE:
			state_untargettable(p, c, 0);
			return 1;

		case ACT_RESET_ADDED_SUBTYPE:
			reset_subtypes(p, c, 2);
			return 1;

		case ACT_DISABLE_ALL_ACTIVATED_ABILITIES:
			disable_all_activated_abilities(p, c, 1);
			return 1;

		case ACT_ENABLE_ALL_ACTIVATED_ABILITIES:
			disable_all_activated_abilities(p, c, 0);
			return 1;

		case ACT_GET_COUNT:
			return 1;

		case ACT_GAIN_CONTROL:
			return gain_control(player, card, p, c) != -1 ? 1 : 0;

		case ACT_DOES_NOT_UNTAP:
			return does_not_untap_effect(player, card, p, c, 0, 1) != -1 ? 1 : 0;

		case ACT_PHASE_OUT:
			phase_out(p, c);
			return 1;

		case ACT_OF_TREASON:
			return effect_act_of_treason(player, card, p, c) != -1 ? 1 : 0;

		case ACT_DETAIN:
			detain(player, card, p, c);
			return 1;

		case ACT_PARAMETERIZED_ACTIONS_MASK:
			ASSERT(!"Parameterized action reached action_on_card_impl() without being cooked");
			return 0;

		case ACT_ADD_COUNTERS_BASE:
			/*
			 * Return the number of counters requested.  This does not account for
			 * replacement effects such as Melira's Keepers, but no current caller
			 * depends on the exact return value.
			 */
			add_counters_predoubled(p, c, counter_type, counter_num);
			return counter_num;

		case ACT_REMOVE_COUNTERS_BASE:
			{
				int curr = count_counters(p, c, counter_type);
				int amt_removed = MIN(counter_num, curr);
				remove_counters(p, c, counter_type, amt_removed);
				return amt_removed;
			}
	}

	return 0;
}

/*
 * Decodes actions whose parameters are packed into the action bitfield.
 *
 * Return values:
 *   -1: action is invalid for every player; caller can stop immediately.
 *   -2: action is invalid for this player only; caller may try the other player.
 */
static actions_t cook_action(int player, actions_t raw_action, counter_t* counter_type, int* counter_num)
{
	switch (raw_action & ACT_PARAMETERIZED_ACTIONS_MASK)
	{
		case 0:
			return raw_action;

		case ACT_ADD_COUNTERS_BASE:
			*counter_type = BYTE2(raw_action);
			*counter_num = SLOWORD(raw_action);

			if (*counter_num <= 0) {
				return -1;
			}

			*counter_num = get_updated_counters_number(player, -1, *counter_type, *counter_num);
			if (*counter_num <= 0) {
				return -2;
			}

			return ACT_ADD_COUNTERS_BASE;

		case ACT_REMOVE_COUNTERS_BASE:
			*counter_type = BYTE2(raw_action);
			*counter_num = SLOWORD(raw_action);

			if (*counter_num <= 0) {
				return -1;
			}

			return ACT_REMOVE_COUNTERS_BASE;

		default:
			ASSERT(!"Unknown parameterized action");
			return -1;
	}
}

int action_on_card(int player, int card, int t_player, int t_card, actions_t raw_action)
{
	if (!is_card_ref(t_player, t_card)) {
		return 0;
	}

	counter_t counter_type = COUNTER_invalid;
	int counter_num = 0;
	actions_t action = cook_action(t_player, raw_action, &counter_type, &counter_num);

	if (action == (actions_t)-1 || action == (actions_t)-2) {
		return 0;
	}

	return action_on_card_impl(player, card, t_player, t_card, action, counter_type, counter_num);
}

int action_on_target(int player, int card, unsigned int target_number, actions_t raw_action)
{
	if (!is_card_ref(player, card) || target_number >= MAX_TARGETS) {
		return 0;
	}

	card_instance_t* instance = get_card_instance(player, card);

	if (!is_card_ref(instance->targets[target_number].player, instance->targets[target_number].card)) {
		return 0;
	}

	return action_on_card(player, card, instance->targets[target_number].player, instance->targets[target_number].card, raw_action);
}

int new_manipulate_all(int player, int card, int t_player, test_definition_t* this_test, actions_t raw_action)
{
	if (!this_test || !is_player_selector(t_player)) {
		return 0;
	}

	int result = 0;
	int marked[MAX_PLAYERS][MAX_CARD_SLOTS];
	int mc[MAX_PLAYERS] = {0, 0};

	/*
	 * First pass: mark every matching card before mutating anything.  Cards may
	 * leave play or shift slots during the action pass, so the search pass must
	 * not perform the action directly.
	 */
	int i, c;
	for (i = 0; i < MAX_PLAYERS; i++) {
		int p = i == 0 ? current_turn : 1 - current_turn;

		if (player_matches_selector(t_player, p)) {
			counter_t counter_type = COUNTER_invalid;
			int counter_num = 0;
			actions_t action = cook_action(p, raw_action, &counter_type, &counter_num);

			if (action == (actions_t)-1) {
				return result;
			}
			else if (action == (actions_t)-2) {
				continue;
			}

			int active_count = bounded_active_cards_count(p);
			for (c = active_count - 1; c >= 0; --c) {
				if ((((this_test->zone == TARGET_ZONE_IN_PLAY && in_play(p, c) && is_what(p, c, TYPE_PERMANENT) && !check_state(p, c, STATE_CANNOT_TARGET))
					  || (this_test->zone == TARGET_ZONE_HAND && in_hand(p, c)))
					 && new_make_test_in_play(p, c, -1, this_test)
					 && (this_test->not_me == 0 || !(player == p && c == card)))
					&& mc[p] < MAX_CARD_SLOTS) {
					marked[p][mc[p]] = c;
					mc[p]++;
				}
			}
		}
	}

	/*
	 * Second pass: apply the action to marked cards.  Re-cook the action per
	 * player because counter replacement effects can be controller-specific.
	 */
	for (i = 0; i < MAX_PLAYERS; i++) {
		int p = i == 0 ? current_turn : 1 - current_turn;

		if (player_matches_selector(t_player, p)) {
			counter_t counter_type = COUNTER_invalid;
			int counter_num = 0;
			actions_t action = cook_action(p, raw_action, &counter_type, &counter_num);

			if (action == (actions_t)-1) {
				return result;
			}
			else if (action == (actions_t)-2) {
				continue;
			}

			for (c = 0; c < mc[p]; c++) {
				result += action_on_card_impl(player, card, p, marked[p][c], action, counter_type, counter_num);
			}
		}
	}

	return result;
}

int manipulate_all(int player, int card, int t_player, int type, int flag1, int subtype, int flag2, int color, int flag3, int id, int flag4, int cc, int flag5, actions_t action)
{
	test_definition_t this_test;
	new_default_test_definition(&this_test, type, "");
	this_test.type = type;
	this_test.type_flag = flag1;
	this_test.subtype = subtype;
	this_test.subtype_flag = flag2;
	this_test.color = color;
	this_test.color_flag = flag3;
	this_test.id = id;
	this_test.id_flag = flag4;
	this_test.cmc = cc;
	this_test.cmc_flag = flag5;

	return new_manipulate_all(player, card, t_player, &this_test, action);
}

int manipulate_type(int player, int card, int t_player, type_t type, actions_t action)
{
	test_definition_t this_test;
	new_default_test_definition(&this_test, type, "");

	return new_manipulate_all(player, card, t_player, &this_test, action);
}

int manipulate_auras_enchanting_target(int player, int card, int t_player, int t_card, test_definition_t *this_test, actions_t raw_action)
{
	/*
	 * If the auras will be exiled and later returned attached to the original
	 * source under some condition, use exile_permanent_and_auras_attached()
	 * instead.  This helper simply applies an action to matching Aura permanents.
	 */
	if (!is_card_ref(t_player, t_card)) {
		return 0;
	}

	int p, c;
	int result = 0;

	for (p = 0; p < MAX_PLAYERS; p++) {
		counter_t counter_type = COUNTER_invalid;
		int counter_num = 0;
		actions_t action = cook_action(p, raw_action, &counter_type, &counter_num);

		if (action == (actions_t)-1) {
			return result;
		}
		else if (action == (actions_t)-2) {
			continue;
		}

		card_instance_t* aura;
		int active_count = bounded_active_cards_count(p);

		for (c = active_count - 1; c >= 0; --c) {
			if ((aura = in_play(p, c))
				&& aura->damage_target_player == t_player
				&& aura->damage_target_card == t_card
				&& is_what(p, c, TYPE_ENCHANTMENT)
				&& has_subtype(p, c, SUBTYPE_AURA)
				&& (!this_test
					|| (new_make_test_in_play(p, c, -1, this_test)
						&& (this_test->not_me == 0 || !(player == p && c == card))))) {
				result += action_on_card_impl(player, card, p, c, action, counter_type, counter_num);
			}
		}
	}

	return result;
}

int new_damage_all(int player, int card, int targ_player, int dmg, int mode, test_definition_t *this_test)
{
	if (!is_player_selector(targ_player)) {
		return 0;
	}

	int score = this_test ? new_get_test_score(this_test) : 0;
	int result = 0;
	int i, n, count;

	for (n = 0; n <= 1; ++n) {
		i = n == 0 ? current_turn : 1 - current_turn;

		if (player_matches_selector(targ_player, i)) {
			int active_count = bounded_active_cards_count(i);

			for (count = active_count - 1; count >= 0; --count) {
				if (in_play(i, count)
					&& is_what(i, count, TYPE_CREATURE)
					&& ((mode & NDA_ALL_CREATURES) || !this_test || new_make_test_in_play(i, count, score, this_test))
					&& (card != count || player != i || !((mode & NDA_NOT_TO_ME) || (this_test && this_test->not_me == 1)))) {
					int dmg_card = damage_creature(i, count, dmg, player, card);

					if (dmg_card != -1) {
						card_instance_t* damage = get_card_instance(player, dmg_card);

						if (mode & NDA_CANT_REGENERATE_IF_DEALT_DAMAGE_THIS_WAY) {
							damage->targets[3].card |= DMG_CANT_REGENERATE_IF_DEALT_DAMAGE_THIS_WAY;
						}

						if (mode & NDA_EXILE_IF_FATALLY_DAMAGED) {
							damage->targets[3].card |= DMG_EXILE_IF_LETHALLY_DAMAGED_THIS_WAY;
						}

						result += dmg;
					}
				}
			}

			if (mode & NDA_PLAYER_TOO) {
				damage_player(i, dmg, player, card);
			}
		}
	}

	return result;
}

void damage_all(int p, int c, int controller, int amount_damage, int all, int player_too, int keyword, int flag1, int subtype, int flag2, int color, int flag3, int id, int flag4, int cc, int flag5)
{
	test_definition_t this_test;
	default_test_definition(&this_test, TYPE_CREATURE);
	this_test.subtype = subtype;
	this_test.subtype_flag = flag2;
	this_test.color = color;
	this_test.color_flag = flag3;
	this_test.id = id;
	this_test.id_flag = flag4;
	this_test.cmc = cc;
	this_test.cmc_flag = flag5;
	this_test.keyword = keyword;
	this_test.keyword_flag = flag1;

	new_damage_all(p, c, controller, amount_damage, (1 * all) + (2 * player_too), &this_test);
}