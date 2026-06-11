// -*- c-basic-offset:2 -*-
// Tracks cards that died this turn for morbid, death triggers, and "return dead this turn" effects.

#include "manalink.h"

/*
 * Deadbox records a compact snapshot of permanents that went to a graveyard
 * from the battlefield this turn.
 *
 * The game stores one CARD_ID_DEADBOX effect per player.  Each deadbox uses
 * targets[0] through targets[17] as a flat 36-entry array: each target's player
 * half and card half can hold one internal_card_id.  Token deaths set the high
 * bit so callers can distinguish "a token died" from "a nontoken card with this
 * internal_card_id died."
 *
 * The counters are the fast path:
 *   COUNTER_DEATH  counts all recorded deaths, including tokens.
 *   COUNTER_CORPSE counts only nontoken deaths.
 *
 * The target entries are the detailed path used when callers need type, id, or
 * planeswalker filtering.  If the list ever fills, the counters still advance;
 * broad "anything died" checks remain correct, but detailed lookups can no
 * longer see the overflowed deaths.  This preserves the old behavior while
 * making the limitation explicit.
 */

#define TOKEN_FLAG ((uint32_t)1u << 31)
#define TOKEN_PAYLOAD_MASK (~TOKEN_FLAG)

enum {
	DEADBOX_PLAYERS = 2,
	DEADBOX_CARD_SLOTS = 150,
	DEADBOX_STORAGE_TARGETS = 18,
	DEADBOX_TARGET_ARRAY_SIZE = 19,
	DEADBOX_MAX_ENTRIES = DEADBOX_STORAGE_TARGETS * 2,
	DEADBOX_CHOICE_CAPACITY = 50,
	DEADBOX_DISPLAY_BUFFER_SIZE = 600
};

static int deadbox_player_is_valid(int player)
{
	return player >= HUMAN && player <= AI;
}

static int deadbox_card_slot_is_valid(int card)
{
	return card >= 0 && card < DEADBOX_CARD_SLOTS;
}

static int deadbox_card_ref_is_valid(int player, int card)
{
	return deadbox_player_is_valid(player) && deadbox_card_slot_is_valid(card);
}

static int deadbox_player_matches_filter(int filter, int player)
{
	return filter == ANYBODY || filter == player;
}

static int deadbox_active_cards_count(int player)
{
	return deadbox_player_is_valid(player) ? MIN(active_cards_count[player], DEADBOX_CARD_SLOTS) : 0;
}

static int deadbox_entry_is_empty(int entry)
{
	return entry == -1;
}

static int deadbox_entry_is_token(int entry)
{
	return ((uint32_t)entry & TOKEN_FLAG) != 0;
}

static int deadbox_entry_iid(int entry)
{
	return (int)((uint32_t)entry & TOKEN_PAYLOAD_MASK);
}

static int make_deadbox_entry(int iid, int is_token_entry)
{
	return is_token_entry ? (int)((uint32_t)iid | TOKEN_FLAG) : iid;
}

static int deadbox_entry_is_valid(int entry)
{
	return !deadbox_entry_is_empty(entry) && deadbox_entry_iid(entry) >= 0;
}

static int* deadbox_entry_at(card_instance_t* instance, int entry_index)
{
	int target_index = entry_index / 2;

	if (!instance || entry_index < 0 || entry_index >= DEADBOX_MAX_ENTRIES) {
		return NULL;
	}

	return (entry_index & 1) ? &instance->targets[target_index].card : &instance->targets[target_index].player;
}

static int store_deadbox_entry(card_instance_t* instance, int entry)
{
	/*
	 * Store in the first free half-target.  The exact slot has no meaning; the
	 * deadbox is a bag, not an ordered zone.
	 */
	int i;
	for (i = 0; i < DEADBOX_MAX_ENTRIES; ++i) {
		int* slot = deadbox_entry_at(instance, i);
		if (slot && *slot == -1) {
			*slot = entry;
			return 1;
		}
	}

	return 0;
}

static void clear_deadbox_entries(card_instance_t* instance)
{
	int i;
	for (i = 0; i < DEADBOX_STORAGE_TARGETS; ++i) {
		instance->targets[i].player = -1;
		instance->targets[i].card = -1;
	}
}

static int count_deadbox_entry_for_mode(int entry, int mode)
{
	if (!deadbox_entry_is_valid(entry)) {
		return 0;
	}

	int tok = deadbox_entry_is_token(entry) ? TARGET_TYPE_TOKEN : 0;
	int iid = deadbox_entry_iid(entry);
	int typ = get_type(-1, iid) | tok;

	/*
	 * TYPE_ENCHANTMENT is not set for planeswalkers here.  That is fine for the
	 * existing callers: broad permanent counts use the counter fast path, and
	 * GDC_NONPLANESWALKER deliberately excludes planeswalkers.
	 */
	return (mode & typ)
		   && !((mode & GDC_NONTOKEN) && tok)
		   && !((mode & GDC_NONPLANESWALKER) && (typ & TARGET_TYPE_PLANESWALKER));
}

static int deadbox_append(char* buffer, int buffer_size, int pos, const char* fmt, ...)
{
	if (!buffer || pos < 0 || pos >= buffer_size) {
		return pos;
	}

	va_list args;
	va_start(args, fmt);
	int written = vscnprintf(buffer + pos, buffer_size - pos, fmt, args);
	va_end(args);

	if (written > 0) {
		pos += written;
		if (pos >= buffer_size) {
			pos = buffer_size - 1;
		}
	}

	return pos;
}

static int deadbox_append_entry_name(char* buffer, int buffer_size, int pos, int entry)
{
	if (!deadbox_entry_is_valid(entry)) {
		return pos;
	}

	int iid = deadbox_entry_iid(entry);
	card_ptr_t* cp = cards_ptr[cards_data[iid].id];

	return deadbox_append(buffer, buffer_size, pos, " %s\n", cp->name);
}

static int get_deadbox_corpse_count_for_player(int player)
{
	int dbc = get_deadbox_card(player);
	return dbc >= 0 ? count_counters(player, dbc, COUNTER_CORPSE) : 0;
}

void increase_dead_count(int player, int iid, int is_tok)
{
	if (!deadbox_player_is_valid(player) || iid < 0) {
		return;
	}

	int dbc = get_deadbox_card(player);
	if (dbc < 0) {
		return;
	}

	card_instance_t *instance = get_card_instance(player, dbc);
	int entry = make_deadbox_entry(iid, is_tok);

	/*
	 * If storage is full, still advance the counters.  Broad "a permanent died"
	 * and "a nontoken permanent died" queries should remain accurate even though
	 * detailed type queries cannot see overflow entries.
	 */
	store_deadbox_entry(instance, entry);

	++hack_silent_counters;
	add_counter(player, dbc, COUNTER_DEATH);
	if (is_tok == 0) {
		add_counter(player, dbc, COUNTER_CORPSE);
	}
	--hack_silent_counters;
}

int get_dead_count(int player, int mode)
{
	// mode is the card_type to check plus the flags. Check gdc_flags_t in manalink.h for details.

	if (!deadbox_player_is_valid(player) && player != ANYBODY) {
		return 0;
	}

	int amount = 0;
	int p;

	for (p = 0; p < DEADBOX_PLAYERS; ++p) {
		if (!deadbox_player_matches_filter(player, p)) {
			continue;
		}

		int dbc = get_deadbox_card(p);
		if (dbc < 0) {
			continue;
		}

		if (mode == TYPE_PERMANENT || mode == TYPE_ANY) {
			amount += count_counters(p, dbc, COUNTER_DEATH);
		}
		else if (mode == (TYPE_PERMANENT|GDC_NONTOKEN) || mode == (TYPE_ANY|GDC_NONTOKEN)) {
			amount += count_counters(p, dbc, COUNTER_CORPSE);
		}
		else {
			card_instance_t* instance = get_card_instance(p, dbc);
			int i;

			for (i = 0; i < DEADBOX_MAX_ENTRIES; ++i) {
				int* entry = deadbox_entry_at(instance, i);
				if (entry && count_deadbox_entry_for_mode(*entry, mode)) {
					++amount;
				}
			}
		}
	}

	return amount;
}

void reset_dead_count(void)
{
	int p;
	for (p = 0; p < DEADBOX_PLAYERS; ++p) {
		int dbc = get_deadbox_card(p);
		if (dbc < 0) {
			continue;
		}

		card_instance_t *instance = get_card_instance(p, dbc);
		remove_all_counters(p, dbc, -1);
		clear_deadbox_entries(instance);
	}
}

int card_deadbox(int player, int card, event_t event)
{
	if (IS_AI(player)) {
		return 0;
	}

	if (event == EVENT_GRAVEYARD_FROM_PLAY) {
		if (!deadbox_card_ref_is_valid(affected_card_controller, affected_card)) {
			return 0;
		}

		if (!in_play(affected_card_controller, affected_card)) {
			return 0;
		}

		card_instance_t *dead = get_card_instance(affected_card_controller, affected_card);
		if (dead->kill_code > 0 && dead->kill_code != KILL_REMOVE && is_what(affected_card_controller, affected_card, TYPE_PERMANENT)) {
			int owner = is_stolen(affected_card_controller, affected_card) ? 1 - affected_card_controller : affected_card_controller;
			int iid = get_original_internal_card_id(affected_card_controller, affected_card);

			/*
			 * Roughly half the deadbox uses want the state of the card as it
			 * left play, and half want it as it is in the graveyard.  Store the
			 * original internal id for now because dynamically-created ids can
			 * stop being valid at end of turn once no card is still using them.
			 *
			 * A future representation should store both the original iid and the
			 * leaving-play type bits.  That would let morbid-like checks and
			 * graveyard-targeting checks stop sharing a lossy encoding.
			 */
			increase_dead_count(owner, iid, is_token(affected_card_controller, affected_card));
		}
	}

	if (event == EVENT_CAN_ACTIVATE) {
		return !IS_AI(player)
			   && (get_deadbox_corpse_count_for_player(player)
				   || get_deadbox_corpse_count_for_player(1 - player));
	}

	if (event == EVENT_ACTIVATE) {
		char buffer[DEADBOX_DISPLAY_BUFFER_SIZE];
		int p;

		for (p = 0; p < DEADBOX_PLAYERS; ++p) {
			int dbc = get_deadbox_card(p);
			if (dbc < 0) {
				continue;
			}

			int pos = 0;
			pos = deadbox_append(buffer, DEADBOX_DISPLAY_BUFFER_SIZE, pos, "%s Deadbox contains:\n",
								 p == player ? "Your" : "Your opponent's");

			card_instance_t* instance = get_card_instance(p, dbc);
			int amount = 0;
			int i;

			for (i = 0; i < DEADBOX_MAX_ENTRIES; ++i) {
				int* entry = deadbox_entry_at(instance, i);
				if (entry && !deadbox_entry_is_empty(*entry)) {
					pos = deadbox_append_entry_name(buffer, DEADBOX_DISPLAY_BUFFER_SIZE, pos, *entry);
					++amount;
				}
			}

			if (amount > 0) {
				do_dialog(player, player, card, -1, -1, buffer, 0);
			}
		}

		cancel = 1;
	}

	if (event == EVENT_CAN_SKIP_TURN && player == HUMAN) {
		reset_dead_count();
	}

	return 0;
}

static int deadbox_entry_matches_reanimation_filter(int entry, int selected_type, int exclude_planeswalkers)
{
	if (!deadbox_entry_is_valid(entry) || deadbox_entry_is_token(entry)) {
		return 0;
	}

	int iid = deadbox_entry_iid(entry);

	return is_what(-1, iid, selected_type)
		   && !(exclude_planeswalkers && is_planeswalker(-1, iid));
}

static void seek_grave_to_reanimate_this_turn(int player, int selected_type, reanimate_mode_t mode)
{
	int non_pw = selected_type & GDC_NONPLANESWALKER;
	selected_type &= ~(GDC_NONTOKEN | GDC_NONPLANESWALKER);	/* we effectively force nontoken anyway, just in case there's a different nontoken card of the same
															 * name in the graveyard */

	int p;
	for (p = 0; p < DEADBOX_PLAYERS; ++p) {
		if (!deadbox_player_matches_filter(player, p)) {
			continue;
		}

		int dbc = get_deadbox_card(p);
		if (dbc < 0) {
			continue;
		}

		card_instance_t *instance = get_card_instance(p, dbc);
		int i;

		for (i = 0; i < DEADBOX_MAX_ENTRIES; ++i) {
			int* entry = deadbox_entry_at(instance, i);
			if (entry && deadbox_entry_matches_reanimation_filter(*entry, selected_type, non_pw)) {
				int iid = deadbox_entry_iid(*entry);
				seek_grave_for_id_to_reanimate(player, -1, p, cards_data[iid].id, mode);
			}
		}
	}
}

void reanimate_all_dead_this_turn(int player, int selected_type)
{
	seek_grave_to_reanimate_this_turn(player, selected_type, REANIMATE_DEFAULT);
}

void return_all_dead_this_turn_to_hand(int player, int selected_type)
{
	seek_grave_to_reanimate_this_turn(player, selected_type, REANIMATE_RETURN_TO_HAND);
}

static int add_deadbox_choice(int choices[DEADBOX_CHOICE_CAPACITY][DEADBOX_PLAYERS], int counts[DEADBOX_PLAYERS], int owner, int iid)
{
	if (!deadbox_player_is_valid(owner) || counts[owner] >= DEADBOX_CHOICE_CAPACITY) {
		return 0;
	}

	choices[counts[owner]][owner] = iid;
	++counts[owner];

	return 1;
}

int choose_a_dead_this_turn(int player, int card, int t_player, int can_cancel, int ai_selection_mode, test_definition_t *this_test, int target_pos)
{
	if (target_pos < 0 || target_pos + 1 >= DEADBOX_TARGET_ARRAY_SIZE) {
		return -1;
	}

	card_instance_t *instance = get_card_instance(player, card);

	int choices[DEADBOX_CHOICE_CAPACITY][DEADBOX_PLAYERS];
	int choice_count[DEADBOX_PLAYERS] = {0, 0};
	int p;
	int my_test = new_get_test_score(this_test);

	for (p = 0; p < DEADBOX_PLAYERS; ++p) {
		if (!deadbox_player_matches_filter(t_player, p)) {
			continue;
		}

		int dbc = get_deadbox_card(p);
		if (dbc < 0) {
			continue;
		}

		card_instance_t *deadb = get_card_instance(p, dbc);
		int i;

		for (i = 0; i < DEADBOX_MAX_ENTRIES; ++i) {
			int* entry = deadbox_entry_at(deadb, i);
			if (entry && deadbox_entry_is_valid(*entry) && !deadbox_entry_is_token(*entry)) {
				int iid = deadbox_entry_iid(*entry);
				if (new_make_test(p, iid, my_test, this_test)) {
					add_deadbox_choice(choices, choice_count, p, iid);
				}
			}
		}
	}

	for (p = 0; p < DEADBOX_PLAYERS; ++p) {
		if (choice_count[p] <= 0) {
			continue;
		}

		int show_array[DEADBOX_CHOICE_CAPACITY];
		int i;
		for (i = 0; i < choice_count[p]; ++i) {
			show_array[i] = choices[i][p];
		}

		int selected = select_card_from_zone(player, p, show_array, choice_count[p], can_cancel, ai_selection_mode, -1, this_test);
		if (selected != -1) {
			instance->targets[target_pos].player = p;
			instance->targets[target_pos].card = -1;
			instance->targets[target_pos + 1].player = selected;
			instance->targets[target_pos + 1].card = choices[selected][p];
			return selected;
		}
	}

	return -1;
}

// This is inaccurate, as there's no way to know if a specific card went into graveyard from play after the exact moment it happens.
int has_type_dead_this_turn_in_grave(int player, test_definition_t *this_test)
{
	int p;
	for (p = 0; p < DEADBOX_PLAYERS; ++p) {
		if (!deadbox_player_matches_filter(player, p)) {
			continue;
		}

		int dbc = get_deadbox_card(p);
		if (dbc < 0) {
			continue;
		}

		card_instance_t *instance = get_card_instance(p, dbc);
		int i;

		for (i = 0; i < DEADBOX_MAX_ENTRIES; ++i) {
			int* entry = deadbox_entry_at(instance, i);
			if (entry && deadbox_entry_is_valid(*entry) && !deadbox_entry_is_token(*entry)) {
				int iid = deadbox_entry_iid(*entry);
				if (new_make_test(p, iid, -1, this_test)
					&& seek_grave_for_id_to_reanimate(player, -1, p, cards_data[iid].id, REANIMATEXTRA_LEAVE_IN_GRAVEYARD) > -1) {
					return 1;
				}
			}
		}
	}

	return 0;
}
