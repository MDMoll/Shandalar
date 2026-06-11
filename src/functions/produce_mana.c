#include "manalink.h"

/*
 * Mana production helpers.
 *
 * Shandalar stores "mana just produced by tapping" in global state so cards like
 * Mana Flare can inspect it later during EVENT_TAP_CARD.  This file has to keep
 * three related pieces of state synchronized:
 *
 *   tapped_for_mana_color: either a single color, -1 for none, -2 for special
 *                          nonrespondable mana, or 0x100 to inspect the array.
 *   tapped_for_mana[]:     per-color amounts when more than one type, or more
 *                          than one mana, was produced by the tap.
 *   chosen_colors:         color-test bitfield of colors actively chosen by the
 *                          player or AI in helper functions that prompt.
 *
 * Keep the helper functions conservative.  Many callers depend on exact legacy
 * behavior around Contamination, Deep Water, Quarum Trench Gnomes, and mana
 * abilities that should not dispatch an extra EVENT_TAP_CARD.
 */

int tapped_for_mana[COLOR_ARTIFACT + 1] = { 0 };

char always_prompt_for_color = 0;

/*
 * Color or colors actively chosen to tap for.
 *
 * This is not set when mana color is overridden by effects such as Contamination.
 * Only some functions in this file maintain it, so inspect the caller before
 * relying on it.
 */
color_test_t chosen_colors = 0;

enum {
	MAX_CARD_SLOTS = 150
};

static int is_valid_mana_color(color_t color)
{
	return color >= COLOR_COLORLESS && color <= COLOR_ARTIFACT;
}

static void clear_tapped_for_mana(void)
{
	color_t color;

	for (color = COLOR_COLORLESS; color <= COLOR_ARTIFACT; ++color) {
		tapped_for_mana[color] = 0;
	}
}

static void remember_tapped_for_mana(color_t color, int amount)
{
	clear_tapped_for_mana();

	if (is_valid_mana_color(color)) {
		tapped_for_mana[color] = amount >= 0 ? amount : 0;
		tapped_for_mana_color = amount == 1 ? color : 0x100;
	}
	else {
		tapped_for_mana_color = -1;
	}
}

static const char* mana_prompt_card_full_name(int player, int card)
{
	if (player < HUMAN || player > AI || card < 0 || card >= MIN(active_cards_count[player], MAX_CARD_SLOTS)) {
		return "Unknown";
	}

	card_instance_t* instance = get_card_instance(player, card);
	if (!instance || instance->internal_card_id < 0 || !cards_at_7c7000[instance->internal_card_id]) {
		return "Unknown";
	}

	int csvid = cards_at_7c7000[instance->internal_card_id]->id;
	if (csvid < 0 || csvid >= available_slots || !cards_ptr[csvid] || !cards_ptr[csvid]->full_name) {
		return "Unknown";
	}

	return cards_ptr[csvid]->full_name;
}

/*
 * Forwards to either declare_mana_available() or declare_mana_available_hex(),
 * depending on whether exactly one bit is set in colors.
 *
 * declare_mana_available() is preferable when the exact color is known because
 * declare_mana_available_hex() has limited slots and is slower in has_mana(),
 * charge_mana(), and related checks.
 */
void declare_mana_available_maybe_hex(int player, color_test_t colors, int amount)
{
	colors &= (COLOR_TEST_ANY | COLOR_TEST_ARTIFACT);

	if (!colors) {
		return;
	}

	if (num_bits_set(colors) == 1) {
		declare_mana_available(player, single_color_test_bit_to_color(colors), amount);
	}
	else {
		declare_mana_available_hex(player, colors, amount);
	}
}

void declare_mana_available_any_combination_of_colors(int player, color_test_t available, int amount)
{
	/***************************************************************************************************************************************************************
	 *    A simple declare_mana_available_hex(player, COLOR_TEST_ANY_COLORED, count) is equivalent to "This can produce [count] of any one color among             *
	 * COLOR_TEST_ANY_COLORED", which of course isn't accurate here - available = COLOR_TEST_BLACK|COLOR_TEST_BLUE and amount = 2 should be able to pay for |U|B.  *
	 *    On the other hand, we don't want to do for (i = 0; i < count; ++i) declare_mana_available_hex(player, COLOR_TEST_ANY_COLORED, 1) either, since           *
	 * declare_mana_available_hex() can only store the result of 50 calls per player.  Multiple copies of a card like this can easily run out.                     *
	 *    As a compromise between extremes, call declare_mana_available_hex() up to seven times (one for each bit set in available), or amount (if that's lower),  *
	 * distributing the amounts of mana evenly.  This is accurate, but still uses more slots than I'm really comfortable with.  On the plus side, I'm not aware of *
	 * any effects like this with more than five bits set, and choose_a_color_exe() can't deal with COLOR_TEST_ARTIFACT in any case.                               *
	 *    If this still turns out to be a problem, expanding the number of slots available to declare_mana_available_hex() is feasible, though still a fairly      *
	 * significant project.                                                                                                                                        *
	 ***************************************************************************************************************************************************************/
	if (amount > 0) {
		unsigned int i;
		unsigned int num_bits = num_bits_set(available & (COLOR_TEST_ANY | COLOR_TEST_ARTIFACT));

		for (i = 0; i < num_bits; ++i) {
			if ((amount + i) >= num_bits) {
				declare_mana_available_hex(player, available, (amount + i) / num_bits);
			}
		}
	}
}

int mana_producer(int player, int card, event_t event)
{
	card_instance_t* instance = get_card_instance(player, card);

	switch (event) {
		case EVENT_CHANGE_TYPE:
			if (affect_me(player, card) && instance->targets[12].card != -1 && instance->parent_card < 0) {
				event_result = instance->targets[12].card;
			}
			break;

		case EVENT_RESOLVE_SPELL:
			play_land_sound_effect(player, card);
			instance->targets[13].player = get_id(player, card);
			break;

		case EVENT_CAST_SPELL:
			if (affect_me(player, card)) {
				ai_modifier += 100;
			}
			break;

		case EVENT_CAN_ACTIVATE:
			return CAN_TAP_FOR_MANA(player, card);

		case EVENT_ACTIVATE:
			if (can_produce_mana(player, card)) {
				if (check_special_flags2(player, card, SF2_CONTAMINATION) && is_what(player, card, TYPE_LAND)) {
					produce_mana_tapped(player, card, COLOR_BLACK, 1);
				}
				else {
					/* I'd think this should be ->mana_color, but match what the exe does. */
					int colors = instance->card_color & (COLOR_TEST_ANY | COLOR_TEST_ARTIFACT);
					if (num_bits_set(colors) == 1) {
						produce_mana_tapped(player, card, single_color_test_bit_to_color(colors), 1);
					}
					else {
						produce_mana_tapped_all_one_color(player, card, instance->card_color, 1);
					}
				}
			}
			else {
				cancel = 1;
			}
			break;

		case EVENT_COUNT_MANA:
			if (affect_me(player, card) && CAN_TAP_FOR_MANA(player, card)) {
				if (check_special_flags2(player, card, SF2_CONTAMINATION) && is_what(player, card, TYPE_LAND)) {
					declare_mana_available(player, COLOR_BLACK, 1);
				}
				else {
					int clr = instance->card_color;

					if ((check_special_flags2(player, card, SF2_QUARUM_TRENCH_GNOMES) & (clr & COLOR_TEST_WHITE))) {
						clr &= ~COLOR_TEST_WHITE;
					}
					if (check_special_flags2(player, card, SF2_DEEP_WATER)) {
						clr = COLOR_TEST_BLUE;
					}

					declare_mana_available_maybe_hex(player, clr & (COLOR_TEST_ANY | COLOR_TEST_ARTIFACT), 1);
				}
			}
			break;

		default:
			break;
	}

	return 0;
}

int mana_producer_tapped(int player, int card, event_t event)
{
	comes_into_play_tapped(player, card, event);
	return mana_producer(player, card, event);
}

void produce_mana_multi(int player, int cless, int black, int blue, int green, int red, int white)
{
	produce_mana(player, COLOR_COLORLESS, cless);
	produce_mana(player, COLOR_BLACK, black);
	produce_mana(player, COLOR_BLUE, blue);
	produce_mana(player, COLOR_GREEN, green);
	produce_mana(player, COLOR_RED, red);
	produce_mana(player, COLOR_WHITE, white);
}

/* Converts from a color_test_t with exactly one bit set to a color_t. */
int single_color_test_bit_to_color(color_test_t col)
{
	return (col & COLOR_TEST_BLACK ? COLOR_BLACK
			: col & COLOR_TEST_BLUE ? COLOR_BLUE
			: col & COLOR_TEST_GREEN ? COLOR_GREEN
			: col & COLOR_TEST_RED ? COLOR_RED
			: col & COLOR_TEST_WHITE ? COLOR_WHITE
			: col & COLOR_TEST_ARTIFACT ? COLOR_ARTIFACT
			: COLOR_COLORLESS);
}

static color_t apply_quarum_trench_gnomes_mana_replacement(int player, int card, color_t color)
{
	return (card >= 0
			&& color == COLOR_WHITE
			&& is_what(player, card, TYPE_LAND)
			&& check_special_flags2(player, card, SF2_QUARUM_TRENCH_GNOMES)) ? COLOR_COLORLESS : color;
}

/*
 * Produces the specified color/amount of mana and taps card.
 *
 * Do not replace the direct STATE_TAPPED write with tap_card().  tap_card()
 * clears tapped_for_mana_color and sends an extra EVENT_TAP_CARD, which breaks
 * effects such as Psychic Venom and Mana Flare-style inspections.
 */
void produce_mana_tapped(int player, int card, color_t color, int amount)
{
	if (card >= 0
		&& check_special_flags2(player, card, SF2_CONTAMINATION)
		&& is_what(player, card, TYPE_LAND)) {
		produce_mana(player, COLOR_BLACK, 1);
		remember_tapped_for_mana(COLOR_BLACK, 1);
	}
	else if (card >= 0
			 && check_special_flags2(player, card, SF2_QUARUM_TRENCH_GNOMES)
			 && color == COLOR_WHITE
			 && is_what(player, card, TYPE_LAND)) {
		int produced = amount > 0 ? amount : 0;
		if (produced > 0) {
			produce_mana(player, COLOR_COLORLESS, produced);
		}
		remember_tapped_for_mana(COLOR_COLORLESS, produced);
	}
	else if (card >= 0
			 && check_special_flags2(player, card, SF2_DEEP_WATER)
			 && is_what(player, card, TYPE_LAND)) {
		produce_mana(player, COLOR_BLUE, 1);
		remember_tapped_for_mana(COLOR_BLUE, 1);
	}
	else {
		if (amount > 0) {
			produce_mana(player, color, amount);
		}
		remember_tapped_for_mana(color, amount);
	}

	if (card >= 0) {
		get_card_instance(player, card)->state |= STATE_TAPPED;
	}
}

/* As produce_mana_tapped(), but for two colors of mana. */
void produce_mana_tapped2(int player, int card, color_t color1, int amount1, color_t color2, int amount2)
{
	if (color1 == color2) {
		produce_mana_tapped(player, card, color1, amount1 + amount2);
		return;
	}
	else if (card >= 0
			 && check_special_flags2(player, card, SF2_CONTAMINATION)
			 && is_what(player, card, TYPE_LAND)) {
		produce_mana(player, COLOR_BLACK, 1);
		remember_tapped_for_mana(COLOR_BLACK, 1);
	}
	else {
		if (amount1 > 0) {
			produce_mana(player, color1, amount1);
		}
		if (amount2 > 0) {
			produce_mana(player, color2, amount2);
		}

		clear_tapped_for_mana();
		if (is_valid_mana_color(color1)) {
			tapped_for_mana[color1] = amount1 >= 0 ? amount1 : 0;
		}
		if (is_valid_mana_color(color2)) {
			tapped_for_mana[color2] = amount2 >= 0 ? amount2 : 0;
		}
		tapped_for_mana_color = 0x100;
	}

	if (card >= 0) {
		get_card_instance(player, card)->state |= STATE_TAPPED;
	}
}

/* As produce_mana_tapped(), but for arbitrary amounts of colored/unrestricted-colorless mana. */
void produce_mana_tapped_multi(int player, int card, int colorless, int black, int blue, int green, int red, int white)
{
	if (card >= 0
		&& check_special_flags2(player, card, SF2_CONTAMINATION)
		&& is_what(player, card, TYPE_LAND)) {
		produce_mana(player, COLOR_BLACK, 1);
		remember_tapped_for_mana(COLOR_BLACK, 1);
	}
	else {
		if (colorless > 0) {
			produce_mana(player, COLOR_COLORLESS, colorless);
		}
		if (black > 0) {
			produce_mana(player, COLOR_BLACK, black);
		}
		if (blue > 0) {
			produce_mana(player, COLOR_BLUE, blue);
		}
		if (green > 0) {
			produce_mana(player, COLOR_GREEN, green);
		}
		if (red > 0) {
			produce_mana(player, COLOR_RED, red);
		}
		if (white > 0) {
			produce_mana(player, COLOR_WHITE, white);
		}

		clear_tapped_for_mana();
		tapped_for_mana[COLOR_COLORLESS] = colorless >= 0 ? colorless : 0;
		tapped_for_mana[COLOR_BLACK] = black >= 0 ? black : 0;
		tapped_for_mana[COLOR_BLUE] = blue >= 0 ? blue : 0;
		tapped_for_mana[COLOR_GREEN] = green >= 0 ? green : 0;
		tapped_for_mana[COLOR_RED] = red >= 0 ? red : 0;
		tapped_for_mana[COLOR_WHITE] = white >= 0 ? white : 0;
		tapped_for_mana[COLOR_ARTIFACT] = 0;
		tapped_for_mana_color = 0x100;
	}

	if (card >= 0) {
		get_card_instance(player, card)->state |= STATE_TAPPED;
	}
}

static int choose_any_color_from_available(color_test_t available)
{
	int i;

	for (i = COLOR_COLORLESS; i <= COLOR_ARTIFACT; ++i) {
		if (available & (1 << i)) {
			return i;
		}
	}

	return COLOR_COLORLESS;
}

/* Chooses one color for one point of mana in a multi-color-choice sequence. */
static int choose_combination_of_colors_of_mana_to_produce_onechoice(int* cols, int player, color_test_t available, int num_mana, const char* prompt)
{
	int i;
	int highest = -1;
	int num_available_needed = 0;
	color_test_t needed = 0;
	int charging[COLOR_ARTIFACT + 1] = { 0 };

	if (needed_mana_colors > 0) {
		for (i = COLOR_COLORLESS; i <= COLOR_ARTIFACT; ++i) {
			if ((*charge_mana_addr_of_pay_mana_xbugrw)[i] == -1) {
				charging[i] = 1000;
			}
			else {
				charging[i] = ((*charge_mana_addr_of_pay_mana_xbugrw)[i]
							   - cols[i]
							   + (charge_mana_addr_of_pre_mana_pool
								  ? (*charge_mana_addr_of_pre_mana_pool)[i] - mana_pool[player][i]
								  : 0));
			}
		}

		for (i = COLOR_COLORLESS; i <= COLOR_ARTIFACT; ++i) {
			if (charging[i] < 0) {
				if (charging[COLOR_ARTIFACT] > 0 && i != COLOR_ARTIFACT) {
					int amt_to_apply = MIN(-charging[i], charging[COLOR_ARTIFACT]);
					charging[COLOR_ARTIFACT] -= amt_to_apply;
					charging[i] += amt_to_apply;
					if (charging[i] >= 0) {
						continue;
					}
				}
				if (charging[COLOR_COLORLESS] > 0 && i != COLOR_COLORLESS) {
					int amt_to_apply = MIN(-charging[i], charging[COLOR_COLORLESS]);
					charging[COLOR_COLORLESS] -= amt_to_apply;
					charging[i] += amt_to_apply;
				}
			}
			else if (charging[i] > 0) {
				needed |= 1 << i;

				if (i != COLOR_COLORLESS && i != COLOR_ARTIFACT && (available & (1 << i))) {
					++num_available_needed;
					num_mana -= charging[i];

					if (highest == -1 || charging[i] > charging[highest]) {
						highest = i;
					}
				}
			}
		}

		if (num_mana >= 0) {
			if (highest >= 0) {
				return highest;
			}
			else if (always_prompt_for_color) {
				highest = choose_any_color_from_available(available);
				return choose_a_color_exe(player, prompt, 1, highest, available);
			}
		}
	}

	if (num_available_needed == 1) {
		return highest;
	}

	if (highest == -1 && !always_prompt_for_color) {
		highest = choose_any_color_from_available(available);

		if (needed_mana_colors > 0 && (charging[COLOR_COLORLESS] > 0 || charging[COLOR_ARTIFACT] > 0)) {
			return highest;
		}
	}

	return choose_a_color_exe(player, prompt, 1, highest, available);
}

static void choose_combination_of_colors_of_mana_to_produce_into_arr(int* cols, int player, color_test_t available, int num_mana, const char* prompt)
{
	char buf[300];
	int num_left = num_mana;

	for (; num_left > 0; --num_left) {
		if (prompt || player == AI || ai_is_speculating == 1) {
			buf[0] = 0;
		}
		else {
			if (num_left == num_mana) {
				if (num_left == 1) {
					prompt = "What kind of mana?";
					buf[0] = 0;
				}
				else {
					scnprintf(buf, sizeof(buf), "%d mana left", num_left);
				}
			}
			else {
				int pos = scnprintf(buf, sizeof(buf), "%d mana left (producing ", num_left);
				int i;

				if (pos < 0) {
					pos = 0;
				}
				if (pos >= (int)sizeof(buf)) {
					pos = (int)sizeof(buf) - 1;
				}

				if (cols[COLOR_COLORLESS] > 0) {
					pos += scnprintf(buf + pos, sizeof(buf) - pos, "%d", cols[COLOR_COLORLESS]);
				}
				if (pos < 0) {
					pos = 0;
				}
				if (pos >= (int)sizeof(buf)) {
					pos = (int)sizeof(buf) - 1;
				}

				for (i = 0; i < cols[COLOR_WHITE] && pos < (int)sizeof(buf) - 1; ++i) {
					buf[pos++] = 'W';
				}
				for (i = 0; i < cols[COLOR_BLUE] && pos < (int)sizeof(buf) - 1; ++i) {
					buf[pos++] = 'U';
				}
				for (i = 0; i < cols[COLOR_BLACK] && pos < (int)sizeof(buf) - 1; ++i) {
					buf[pos++] = 'B';
				}
				for (i = 0; i < cols[COLOR_RED] && pos < (int)sizeof(buf) - 1; ++i) {
					buf[pos++] = 'R';
				}
				for (i = 0; i < cols[COLOR_GREEN] && pos < (int)sizeof(buf) - 1; ++i) {
					buf[pos++] = 'G';
				}
				for (i = 0; i < cols[COLOR_ARTIFACT] && pos < (int)sizeof(buf) - 1; ++i) {
					buf[pos++] = 'A';
				}
				if (pos < (int)sizeof(buf) - 1) {
					buf[pos++] = ')';
				}
				buf[pos] = 0;
			}
		}

		int result = choose_combination_of_colors_of_mana_to_produce_onechoice(cols, player, available, num_left, prompt ? prompt : buf);
		if (cancel == 1 || result < COLOR_COLORLESS || result > COLOR_ARTIFACT) {
			cancel = 1;
			return;
		}

		++cols[result];
		prompt = NULL;
	}
}

/*
 * As produce_mana_tapped(), but num_mana of any combination of colors set in
 * available.  If prompt is NULL, one is constructed on the fly for each color
 * choice, showing amount of mana left and colors chosen so far.
 */
int produce_mana_tapped_any_combination_of_colors(int player, int card, color_test_t available, int num_mana, const char* prompt)
{
	chosen_colors = 0;
	cancel = 0;

	if (card >= 0
		&& check_special_flags2(player, card, SF2_CONTAMINATION)
		&& is_what(player, card, TYPE_LAND)) {
		produce_mana(player, COLOR_BLACK, 1);
		remember_tapped_for_mana(COLOR_BLACK, 1);
	}
	else {
		int cols[COLOR_ARTIFACT + 1] = { 0 };
		choose_combination_of_colors_of_mana_to_produce_into_arr(cols, player, available, num_mana, prompt);

		if (cancel == 1) {
			return 0;
		}
		else {
			int i;
			clear_tapped_for_mana();

			for (i = COLOR_COLORLESS; i <= COLOR_ARTIFACT; ++i) {
				if (cols[i] > 0) {
					color_t produced_color = apply_quarum_trench_gnomes_mana_replacement(player, card, i);
					produce_mana(player, produced_color, cols[i]);
					chosen_colors |= 1 << produced_color;
					tapped_for_mana[produced_color] += MAX(0, cols[i]);
				}
			}

			tapped_for_mana_color = 0x100;
		}
	}

	if (card >= 0) {
		get_card_instance(player, card)->state |= STATE_TAPPED;
	}

	return 1;
}

/*
 * As produce_mana_tapped_any_combination_of_colors(), but doesn't tap card,
 * account for Contamination, or set tapped_for_mana_color.
 */
int produce_mana_any_combination_of_colors(int player, color_test_t available, int num_mana, const char* prompt)
{
	chosen_colors = 0;
	cancel = 0;

	int cols[COLOR_ARTIFACT + 1] = { 0 };
	choose_combination_of_colors_of_mana_to_produce_into_arr(cols, player, available, num_mana, prompt);

	if (cancel == 1) {
		return 0;
	}
	else {
		int i;
		for (i = COLOR_COLORLESS; i <= COLOR_ARTIFACT; ++i) {
			if (cols[i] > 0) {
				produce_mana(player, i, cols[i]);
				chosen_colors |= 1 << i;
			}
		}
		return 1;
	}
}

static int choose_mana_all_one_color(int player, color_test_t available, int num_mana, const char* prompt, color_test_t default_color)
{
	if (num_mana <= 0) {
		return choose_any_color_from_available(available);
	}

	int i;
	int highest = -1;
	int num_available_needed = 0;
	color_test_t needed = 0;
	int charging[COLOR_ARTIFACT + 1] = { 0 };

	if (needed_mana_colors > 0) {
		for (i = COLOR_COLORLESS; i <= COLOR_ARTIFACT; ++i) {
			if ((*charge_mana_addr_of_pay_mana_xbugrw)[i] == -1) {
				charging[i] = 1000;
			}
			else {
				charging[i] = ((*charge_mana_addr_of_pay_mana_xbugrw)[i]
							   + (charge_mana_addr_of_pre_mana_pool
								  ? (*charge_mana_addr_of_pre_mana_pool)[i] - mana_pool[player][i]
								  : 0));
			}
		}

		for (i = COLOR_COLORLESS; i <= COLOR_ARTIFACT; ++i) {
			if (charging[i] < 0) {
				if (charging[COLOR_ARTIFACT] > 0 && i != COLOR_ARTIFACT) {
					int amt_to_apply = MIN(-charging[i], charging[COLOR_ARTIFACT]);
					charging[COLOR_ARTIFACT] -= amt_to_apply;
					charging[i] += amt_to_apply;
					if (charging[i] >= 0) {
						continue;
					}
				}
				if (charging[COLOR_COLORLESS] > 0 && i != COLOR_COLORLESS) {
					int amt_to_apply = MIN(-charging[i], charging[COLOR_COLORLESS]);
					charging[COLOR_COLORLESS] -= amt_to_apply;
					charging[i] += amt_to_apply;
				}
			}
			else if (charging[i] > 0) {
				needed |= 1 << i;

				if (i != COLOR_COLORLESS && i != COLOR_ARTIFACT && (available & (1 << i))) {
					++num_available_needed;
					num_mana -= charging[i];

					if (highest == -1 || charging[i] > charging[highest]) {
						highest = i;
					}
				}
			}
		}

		if (num_available_needed == 1 && num_mana >= 0) {
			if (highest >= 0) {
				return highest;
			}
			else {
				if (available & default_color) {
					highest = choose_any_color_from_available(available & default_color);
				}
				else {
					highest = choose_any_color_from_available(available);
				}

				if ((charging[COLOR_COLORLESS] > 0 || charging[COLOR_ARTIFACT] > 0) && !always_prompt_for_color) {
					return highest;
				}
				else {
					return choose_a_color_exe(player, prompt, 1, highest, available);
				}
			}
		}
	}

	if (num_available_needed == 1) {
		return highest;
	}

	if (highest == -1 && !always_prompt_for_color) {
		if (available & default_color) {
			highest = choose_any_color_from_available(available & default_color);
		}
		else {
			highest = choose_any_color_from_available(available);
		}

		if (needed_mana_colors > 0
			&& (MAX(0, charging[COLOR_COLORLESS]) + MAX(0, charging[COLOR_ARTIFACT]) >= num_mana)) {
			return highest;
		}
	}

	return choose_a_color_exe(player, prompt, 1, highest, available);
}

/*
 * As produce_mana_tapped_all_one_color(), but if no specific color is needed
 * and default_color is producible, produce that color.
 */
int produce_mana_tapped_all_one_color_with_default(int player, int card, color_test_t available, int num_mana, color_test_t default_color)
{
	chosen_colors = 0;
	cancel = 0;

	if (card >= 0
		&& check_special_flags2(player, card, SF2_CONTAMINATION)
		&& is_what(player, card, TYPE_LAND)) {
		produce_mana(player, COLOR_BLACK, 1);
		remember_tapped_for_mana(COLOR_BLACK, 1);
	}
	else {
		int choice = choose_mana_all_one_color(player, available, num_mana, "What kind of mana?", default_color);
		if (cancel == 1 || choice == -1) {
			cancel = 1;
			return 0;
		}
		else {
			color_t produced_color = apply_quarum_trench_gnomes_mana_replacement(player, card, choice);

			if (num_mana > 0) {
				produce_mana(player, produced_color, num_mana);
				chosen_colors |= 1 << produced_color;
			}

			remember_tapped_for_mana(produced_color, num_mana);

			if (player == AI && !(trace_mode & 2) && ai_is_speculating != 1) {
				int displayed_choice = produced_color;
				if (displayed_choice == COLOR_COLORLESS) {
					displayed_choice = COLOR_ARTIFACT;
				}
				load_text(0, "FELLWAR_STONE");
				do_dialog(player, player, card, -1, -1, text_lines[displayed_choice >= COLOR_BLACK && displayed_choice <= COLOR_ARTIFACT ? displayed_choice : COLOR_ARTIFACT], 0);
			}
		}
	}

	if (card >= 0) {
		get_card_instance(player, card)->state |= STATE_TAPPED;
	}

	return 1;
}

/* As produce_mana_tapped(), but num_mana all of the same color chosen from available. */
int produce_mana_tapped_all_one_color(int player, int card, color_test_t available, int num_mana)
{
	return produce_mana_tapped_all_one_color_with_default(player, card, available, num_mana, COLOR_TEST_COLORLESS);
}

/*
 * As produce_mana_tapped_all_one_color(), but doesn't tap card, account for
 * Contamination, or set tapped_for_mana_color.
 */
int produce_mana_all_one_color(int player, color_test_t available, int num_mana)
{
	chosen_colors = 0;
	cancel = 0;

	if (num_mana > 0) {
		int choice = choose_mana_all_one_color(player, available, num_mana, "What kind of mana?", COLOR_TEST_COLORLESS);
		if (cancel == 1 || choice == -1) {
			cancel = 1;
			return 0;
		}
		else {
			produce_mana(player, choice, num_mana);
			chosen_colors |= 1 << choice;
		}
	}

	return 1;
}

/*
 * Produces amount of mana in any combination of the types stored in
 * tapped_for_mana_color/tapped_for_mana[].  This is the back-end function of
 * Mana Flare and similar effects.
 */
void produce_mana_of_any_type_tapped_for(int player, int card, int amount)
{
	chosen_colors = 0;

	switch (tapped_for_mana_color) {
		case 0x100: {
			color_test_t colors = 0;
			color_t col;
			color_t first_found = 0x100;
			int num_found = 0;

			for (col = COLOR_COLORLESS; col < COLOR_ARTIFACT; ++col) {
				if (tapped_for_mana[col] > 0) {
					++num_found;
					colors |= (1 << col);
					if (first_found == 0x100) {
						first_found = col;
					}
				}
			}

			/*
			 * Mana Flare does not copy mana restrictions, such as Mishra's
			 * Workshop or Pillar of the Paruns.  Treat artifact-only mana as
			 * unrestricted colorless for the extra mana.
			 */
			if (tapped_for_mana[COLOR_ARTIFACT] > 0 && !(colors & COLOR_TEST_COLORLESS)) {
				++num_found;
				colors |= COLOR_TEST_COLORLESS;
				if (first_found == 0x100) {
					first_found = COLOR_COLORLESS;
				}
			}

			if (num_found == 0) {
				break;
			}
			else if (num_found == 1) {
				produce_mana(affected_card_controller, first_found, amount);
				chosen_colors |= 1 << first_found;
			}
			else {
				char prompt[300];
				scnprintf(prompt, sizeof(prompt), "%s (%s): What kind of mana?",
						  mana_prompt_card_full_name(player, card),
						  mana_prompt_card_full_name(affected_card_controller, affected_card));

				produce_mana_any_combination_of_colors(affected_card_controller, colors, amount, prompt);
				if (cancel == 1) {
					/*
					 * It is too late to cancel tapping the source.  Produce the
					 * first legal color, but let cancel propagate so autotapping
					 * can stop.
					 */
					produce_mana(affected_card_controller, first_found, amount);
					chosen_colors |= 1 << first_found;
				}
			}
			break;
		}

		case COLOR_ARTIFACT:
			produce_mana(affected_card_controller, COLOR_COLORLESS, amount);
			chosen_colors = 1 << COLOR_COLORLESS;
			break;

		case COLOR_COLORLESS:
		case COLOR_BLACK:
		case COLOR_BLUE:
		case COLOR_GREEN:
		case COLOR_RED:
		case COLOR_WHITE:
			produce_mana(affected_card_controller, tapped_for_mana_color, amount);
			chosen_colors = 1 << tapped_for_mana_color;
			break;
	}
}

/*
 * Produces amount of color mana when allowed_player taps a card of type/subtype
 * for mana.  allowed_player may be ANYBODY.
 */
void produce_mana_when_subtype_is_tapped(int allowed_player, event_t event, int type, int subtype, color_t color, int amount)
{
	if (event == EVENT_TAP_CARD
		&& tapped_for_mana_color >= 0
		&& (allowed_player == ANYBODY || allowed_player == affected_card_controller)
		&& ((type & TYPE_PERMANENT) || is_what(affected_card_controller, affected_card, type))
		&& (subtype <= 0 || has_subtype(affected_card_controller, affected_card, subtype))) {
		produce_mana(affected_card_controller, color, amount);
	}

	if (event == EVENT_COUNT_MANA
		&& (allowed_player == ANYBODY || allowed_player == affected_card_controller)
		&& !is_tapped(affected_card_controller, affected_card)
		&& !is_animated_and_sick(affected_card_controller, affected_card)
		&& can_produce_mana(affected_card_controller, affected_card)
		&& ((type & TYPE_PERMANENT) || is_what(affected_card_controller, affected_card, type))
		&& (subtype <= 0 || has_subtype(affected_card_controller, affected_card, subtype))) {
		declare_mana_available(affected_card_controller, color, amount);
	}
}

typedef enum {
	PM_FIXED,
	PM_ALL_ONE,
	PM_ANY_COMBO
} produce_mana_t;

static int wga_backend(int player, int card, event_t event, int subtype, produce_mana_t what_to_produce, int colors, int amount)
{
	target_definition_t td;
	default_target_definition(player, card, &td, TYPE_LAND);
	td.preferred_controller = player;

	if (subtype > 0) {
		td.required_subtype = subtype;
	}

	card_instance_t *instance = get_card_instance(player, card);

	switch (event) {
		case EVENT_CAN_CAST:
			return can_target(&td);

		case EVENT_CAST_SPELL:
			if (affect_me(player, card)) {
				char buffer[100];
				scnprintf(buffer, 100, "Select target %s.", subtype > 0 ? get_hacked_land_text(player, card, "%s", subtype) : "land");

				if (new_pick_target(&td, buffer, 0, 1 | GS_LITERAL_PROMPT)) {
					if (player == AI && instance->targets[0].player == player) {
						ai_modifier += 30;
					}
				}
				else {
					cancel = 1;
				}
			}
			break;

		case EVENT_RESOLVE_SPELL:
			if (valid_target(&td)) {
				instance->damage_target_player = instance->targets[0].player;
				instance->damage_target_card = instance->targets[0].card;
			}
			else {
				kill_card(player, card, KILL_BURY);
				spell_fizzled = 1;
			}
			instance->number_of_targets = 0;
			break;

		case EVENT_TAP_CARD:
			if (instance->damage_target_card != -1
				&& affect_me(instance->damage_target_player, instance->damage_target_card)
				&& tapped_for_mana_color >= 0
				&& amount > 0) {
				switch (what_to_produce) {
					case PM_FIXED:
						produce_mana(instance->damage_target_player, colors, amount);
						break;
					case PM_ALL_ONE:
						produce_mana_all_one_color(instance->damage_target_player, colors, amount);
						break;
					case PM_ANY_COMBO:
						produce_mana_any_combination_of_colors(instance->damage_target_player, colors, amount, NULL);
						break;
				}
			}
			break;

		case EVENT_COUNT_MANA:
			if (instance->damage_target_card != -1
				&& affect_me(instance->damage_target_player, instance->damage_target_card)
				&& !is_tapped(instance->damage_target_player, instance->damage_target_card)
				&& !is_animated_and_sick(instance->damage_target_player, instance->damage_target_card)
				&& can_produce_mana(instance->damage_target_player, instance->damage_target_card)
				&& amount > 0) {
				switch (what_to_produce) {
					case PM_FIXED:
						declare_mana_available(instance->damage_target_player, colors, amount);
						break;
					case PM_ALL_ONE:
						declare_mana_available_maybe_hex(instance->damage_target_player, colors, amount);
						break;
					case PM_ANY_COMBO:
						declare_mana_available_any_combination_of_colors(instance->damage_target_player, colors, amount);
						break;
				}
			}
			break;

		default:
			break;
	}

	return 0;
}

/* An enchant land that produces a specific amount of a specific color when enchanted land is tapped for mana. */
int wild_growth_aura(int player, int card, event_t event, int subtype, color_t color, int amount)
{
	return wga_backend(player, card, event, subtype, PM_FIXED, color, amount);
}

/* As wild_growth_aura(), but produces num_mana of any one color set in available. */
int wild_growth_aura_all_one_color(int player, int card, event_t event, int subtype, color_test_t available, int num_mana)
{
	return wga_backend(player, card, event, subtype, PM_ALL_ONE, available, num_mana);
}

/* As wild_growth_aura(), but produces num_mana of any combination of colors set in available. */
int wild_growth_aura_any_combination_of_colors(int player, int card, event_t event, int subtype, color_test_t available, int num_mana)
{
	return wga_backend(player, card, event, subtype, PM_ANY_COMBO, available, num_mana);
}

/*
 * A generic creature that taps to produce mana, all of a single constant color.
 * If reluctance_to_fight is nonzero, the AI tends not to attack or block when
 * it controls few lands of matching color.
 */
int mana_producing_creature(int player, int card, event_t event, int reluctance_to_fight, color_t color, int amount)
{
	if (event == EVENT_CAN_ACTIVATE) {
		return !is_tapped(player, card) && !is_sick(player, card) && can_produce_mana(player, card) && (player != AI || amount > 0);
	}
	else if (event == EVENT_ACTIVATE) {
		produce_mana_tapped(player, card, color, amount);
	}
	else if (event == EVENT_COUNT_MANA) {
		if (affect_me(player, card) && mana_producing_creature(player, card, EVENT_CAN_ACTIVATE, reluctance_to_fight, color, amount)) {
			declare_mana_available(player, color, amount);
		}
	}
	else if (reluctance_to_fight && (event == EVENT_ATTACK_RATING || event == EVENT_BLOCK_RATING) && affect_me(player, card)) {
		if (color == COLOR_COLORLESS || color == COLOR_ARTIFACT) {
			color = COLOR_ANY;
		}

		if (event == EVENT_ATTACK_RATING) {
			ai_defensive_modifier += reluctance_to_fight / (landsofcolor_controlled[player][color] + 2);
		}
		else {
			ai_defensive_modifier -= 4 * reluctance_to_fight / (landsofcolor_controlled[player][color] + 2);
		}
	}

	return 0;
}

/* As mana_producing_creature(), but produces multiple colors. */
int mana_producing_creature_multi(int player, int card, event_t event, int reluctance_to_fight, int colorless, int black, int blue, int green, int red, int white)
{
	if (event == EVENT_CAN_ACTIVATE) {
		return !is_tapped(player, card) && !is_sick(player, card) && can_produce_mana(player, card)
			&& (player != AI || colorless > 0 || black > 0 || blue > 0 || green > 0 || red > 0 || white > 0);
	}
	else if (event == EVENT_ACTIVATE) {
		produce_mana_tapped_multi(player, card, colorless, black, blue, green, red, white);
	}
	else if (event == EVENT_COUNT_MANA) {
		if (affect_me(player, card)
			&& mana_producing_creature_multi(player, card, EVENT_CAN_ACTIVATE, reluctance_to_fight, colorless, black, blue, green, red, white)) {
			declare_mana_available(player, COLOR_COLORLESS, colorless);
			declare_mana_available(player, COLOR_BLACK, black);
			declare_mana_available(player, COLOR_BLUE, blue);
			declare_mana_available(player, COLOR_GREEN, green);
			declare_mana_available(player, COLOR_RED, red);
			declare_mana_available(player, COLOR_WHITE, white);
		}
	}
	else if (reluctance_to_fight && (event == EVENT_ATTACK_RATING || event == EVENT_BLOCK_RATING) && affect_me(player, card)) {
		if (event == EVENT_ATTACK_RATING) {
			ai_defensive_modifier += reluctance_to_fight / (landsofcolor_controlled[player][COLOR_ANY] + 2);
		}
		else {
			ai_defensive_modifier -= 4 * reluctance_to_fight / (landsofcolor_controlled[player][COLOR_ANY] + 2);
		}
	}

	return 0;
}

/* As mana_producing_creature(), but produces num_mana all of a single color in available. */
int mana_producing_creature_all_one_color(int player, int card, event_t event, int reluctance_to_fight, color_test_t available, int num_mana)
{
	if (event == EVENT_CAN_ACTIVATE) {
		return !is_tapped(player, card) && !is_sick(player, card) && can_produce_mana(player, card) && (player != AI || num_mana > 0);
	}
	else if (event == EVENT_ACTIVATE) {
		produce_mana_tapped_all_one_color(player, card, available, num_mana);
	}
	else if (event == EVENT_COUNT_MANA) {
		if (affect_me(player, card) && mana_producing_creature_all_one_color(player, card, EVENT_CAN_ACTIVATE, reluctance_to_fight, available, num_mana)) {
			declare_mana_available_maybe_hex(player, available, num_mana);
		}
	}
	else if (reluctance_to_fight && (event == EVENT_ATTACK_RATING || event == EVENT_BLOCK_RATING) && affect_me(player, card)) {
		if (event == EVENT_ATTACK_RATING) {
			ai_defensive_modifier += reluctance_to_fight / (landsofcolor_controlled[player][COLOR_ANY] + 2);
		}
		else {
			ai_defensive_modifier -= 4 * reluctance_to_fight / (landsofcolor_controlled[player][COLOR_ANY] + 2);
		}
	}

	return 0;
}

/* A generic artifact that taps to produce mana of its mana_color.  If sac is nonzero, sacrifices itself when activated. */
int artifact_mana_all_one_color(int player, int card, event_t event, int amount, int sac)
{
	if (event == EVENT_CAN_ACTIVATE) {
		return !is_tapped(player, card) && !is_animated_and_sick(player, card) && can_produce_mana(player, card)
			&& (!sac || can_sacrifice_as_cost(player, 1, TYPE_ARTIFACT, 0, 0, 0, 0, 0, 0, 0, -1, 0));
	}
	else if (event == EVENT_ACTIVATE) {
		ai_modifier -= sac ? 36 : 12;

		produce_mana_tapped_all_one_color(player, card, get_card_instance(player, card)->mana_color, amount);
		if (cancel != 1 && sac) {
			kill_card(player, card, KILL_SACRIFICE);
		}
	}
	else if (event == EVENT_COUNT_MANA) {
		if (affect_me(player, card) && artifact_mana_all_one_color(player, card, EVENT_CAN_ACTIVATE, amount, sac)) {
			declare_mana_available_maybe_hex(player, get_card_instance(player, card)->mana_color, amount);
		}
	}

	return 0;
}

int two_mana_land(int player, int card, event_t event, color_t color1, color_t color2)
{
	switch (event) {
		case EVENT_RESOLVE_SPELL:
			play_land_sound_effect(player, card);
			return 0;

		case EVENT_CAN_ACTIVATE:
			return !is_tapped(player, card) && !is_animated_and_sick(player, card) && can_produce_mana(player, card);

		case EVENT_ACTIVATE:
			declare_mana_available(player, color2, -1);
			declare_mana_available(player, color1, -1);
			produce_mana_tapped2(player, card, color1, 1, color2, 1);
			return 0;

		case EVENT_COUNT_MANA:
			if (!affect_me(player, card) || !two_mana_land(player, card, EVENT_CAN_ACTIVATE, color1, color2)) {
				return 0;
			}

			if (check_special_flags2(player, card, SF2_CONTAMINATION)) {
				declare_mana_available(player, COLOR_BLACK, 1);
			}
			else {
				declare_mana_available(player, color2, 1);
				declare_mana_available(player, color1, 1);
			}
			return 0;

		default:
			return 0;
	}
}

int sac_land(int player, int card, event_t event, int base_color, int color1, int color2)
{
	if (event == EVENT_RESOLVE_SPELL) {
		get_card_instance(player, card)->info_slot = (1 << base_color) | (1 << color1) | (1 << color2);
	}
	else if (event == EVENT_ACTIVATE) {
		int choice = 0;
		if (!(player == AI || ai_is_speculating == 1 || ldoubleclicked)) {
			choice = do_dialog(player, player, card, -1, -1, " 1 Mana Ability\n Sac Ability\n Cancel", 0);
		}

		if (choice == 1) {
			produce_mana_tapped2(player, card, color1, 1, color2, 1);
			kill_card(player, card, KILL_SACRIFICE);
		}
		else if (choice == 2) {
			spell_fizzled = 1;
		}
		else {
			return mana_producer_fixed(player, card, event, base_color);
		}
		return 0;
	}

	return mana_producer_fixed(player, card, event, base_color);
}

int sac_land_tapped(int player, int card, event_t event, int base_color, int color1, int color2)
{
	comes_into_play_tapped(player, card, event);
	return sac_land(player, card, event, base_color, color1, color2);
}

/*
 * Implements "X you control have 'T: Add Y to your mana pool.'", where Y is a
 * specific color and amount of mana.
 */
int permanents_you_control_can_tap_for_mana(int player, int card, event_t event, type_t type, int32_t subtype, color_t color, int amount)
{
	if (event == EVENT_CAN_ACTIVATE) {
		return check_for_untapped_nonsick_subtype(player, type, subtype);
	}

	if (event == EVENT_ACTIVATE) {
		target_definition_t td;
		default_target_definition(player, card, &td, type);
		td.allowed_controller = player;
		td.preferred_controller = player;
		td.illegal_state = TARGET_STATE_TAPPED | TARGET_STATE_SUMMONING_SICK;
		td.illegal_abilities = 0;

		if (subtype > 0) {
			td.required_subtype = subtype;
		}

		if (pick_target(&td, type == TYPE_CREATURE ? "TARGET_CREATURE" : type == TYPE_LAND ? "TARGET_LAND" : "TARGET_PERMANENT")) {
			card_instance_t* instance = get_card_instance(player, card);
			instance->number_of_targets = 1;
			produce_mana_tapped(instance->targets[0].player, instance->targets[0].card, color, amount);

			if (!(instance->targets[0].player == player && instance->targets[0].card == card)) {
				dispatch_event(instance->targets[0].player, instance->targets[0].card, EVENT_TAP_CARD);
			}
		}
		else {
			cancel = 1;
		}
	}

	if (event == EVENT_COUNT_MANA && affect_me(player, card)) {
		int count = amount * count_untapped_nonsick_subtype(player, type, subtype);
		if (count > 0) {
			declare_mana_available(player, color, count);
		}
	}

	return 0;
}

/* As permanents_you_control_can_tap_for_mana(), but choice of colors is set in available. */
int permanents_you_control_can_tap_for_mana_all_one_color(int player, int card, event_t event, type_t type, int32_t subtype, color_test_t available, int num_mana)
{
	if ((available & (COLOR_TEST_ANY | COLOR_TEST_ARTIFACT)) == 0) {
		return 0;
	}

	if (event == EVENT_CAN_ACTIVATE && !is_humiliated(player, card)) {
		return check_for_untapped_nonsick_subtype(player, type, subtype);
	}

	if (event == EVENT_ACTIVATE) {
		target_definition_t td;
		default_target_definition(player, card, &td, type);
		td.allowed_controller = player;
		td.preferred_controller = player;
		td.illegal_state = TARGET_STATE_TAPPED | TARGET_STATE_SUMMONING_SICK;
		td.illegal_abilities = 0;

		if (subtype > 0) {
			td.required_subtype = subtype;
		}

		if (pick_target(&td, type == TYPE_CREATURE ? "TARGET_CREATURE" : type == TYPE_LAND ? "TARGET_LAND" : "TARGET_PERMANENT")) {
			card_instance_t* instance = get_card_instance(player, card);
			instance->number_of_targets = 1;
			produce_mana_tapped_all_one_color(instance->targets[0].player, instance->targets[0].card, available, num_mana);

			if (!(instance->targets[0].player == player && instance->targets[0].card == card)) {
				dispatch_event(instance->targets[0].player, instance->targets[0].card, EVENT_TAP_CARD);
			}
		}
		else {
			cancel = 1;
		}
	}

	if (event == EVENT_COUNT_MANA && affect_me(player, card)) {
		int count = count_untapped_nonsick_subtype(player, type, subtype) * num_mana;
		declare_mana_available_any_combination_of_colors(player, available, count);
	}

	return 0;
}

/*
 * Implements "Tap an untapped X you control: Add Y to your mana pool."
 * This differs from permanents_you_control_can_tap_for_mana() because it can tap
 * summoning-sick creatures and correctly models the tap as an activation cost.
 */
int tap_a_permanent_you_control_for_mana(int player, int card, event_t event, type_t type, int32_t subtype, color_t color, int amount)
{
	if (event == EVENT_CAN_ACTIVATE && can_use_activated_abilities(player, card)) {
		return count_untapped_subtype(player, type, subtype) > 0;
	}

	if (event == EVENT_ACTIVATE) {
		target_definition_t td;
		default_target_definition(player, card, &td, type);
		td.allowed_controller = player;
		td.preferred_controller = player;
		td.illegal_state = TARGET_STATE_TAPPED;
		td.illegal_abilities = 0;

		if (subtype > 0) {
			td.required_subtype = subtype;
		}

		if (pick_target(&td, type == TYPE_CREATURE ? "TARGET_CREATURE" : type == TYPE_LAND ? "TARGET_LAND" : "TARGET_PERMANENT")) {
			card_instance_t* instance = get_card_instance(player, card);
			instance->number_of_targets = 1;

			produce_mana(player, color, amount);
			tapped_for_mana_color = -2;

			if (instance->targets[0].player == player && instance->targets[0].card == card) {
				instance->state |= STATE_TAPPED;
			}
			else {
				get_card_instance(instance->targets[0].player, instance->targets[0].card)->state |= STATE_TAPPED;
				dispatch_event(instance->targets[0].player, instance->targets[0].card, EVENT_TAP_CARD);
			}
		}
		else {
			cancel = 1;
		}
	}

	if (event == EVENT_COUNT_MANA && affect_me(player, card)) {
		int count = amount * count_untapped_subtype(player, type, subtype);
		if (count > 0) {
			declare_mana_available(player, color, count);
		}
	}

	return 0;
}

/* This is only correct for lands in play; info_slot is not generally set while a land is still in hand. */
int get_colors_of_mana_land_could_produce_ignoring_costs(int player, int card)
{
	card_data_t* card_d = get_card_data(player, card);
	if (!(card_d->extra_ability & EA_MANA_SOURCE)) {
		return 0;
	}

	int id = card_d->id;
	if (id == CARD_ID_VIVID_CREEK || id == CARD_ID_VIVID_CRAG || id == CARD_ID_VIVID_MEADOW || id == CARD_ID_VIVID_MARSH || id == CARD_ID_VIVID_GROVE
		|| id == CARD_ID_TENDO_ICE_BRIDGE || id == CARD_ID_MIRRODINS_CORE || id == CARD_ID_SHIMMERING_GROTTO || id == CARD_ID_CAVERN_OF_SOULS || id == CARD_ID_UNKNOWN_SHORES) {
		return COLOR_TEST_ANY_COLORED;
	}
	else if (id == CARD_ID_COMMAND_TOWER || id == CARD_ID_ANCIENT_SPRING || id == CARD_ID_GEOTHERMAL_CREVICE || id == CARD_ID_IRRIGATION_DITCH
			 || id == CARD_ID_SULFUR_VENT || id == CARD_ID_TINDER_FARM || id == CARD_ID_GEMSTONE_CAVERNS || id == CARD_ID_NIMBUS_MAZE
			 || id == CARD_ID_RIVER_OF_TEARS || id == CARD_ID_TAINTED_FIELD || id == CARD_ID_TAINTED_ISLE || id == CARD_ID_TAINTED_PEAK
			 || id == CARD_ID_TAINTED_WOOD || id == CARD_ID_GEM_BAZAAR || id == CARD_ID_NYKTHOS_SHRINE_TO_NYX || id == CARD_ID_URBORG_TOMB_OF_YAWGMOTH
			 || id == CARD_ID_OPAL_PALACE || id == CARD_ID_COMMANDERS_SPHERE) {
		card_instance_t* instance = get_card_instance(player, card);
		return instance->info_slot;
	}
	else if (id == CARD_ID_REFLECTING_POOL || id == CARD_ID_EXOTIC_ORCHARD) {
		card_instance_t* instance = get_card_instance(player, card);
		return instance->mana_color;
	}
	else if (id == CARD_ID_SPRINGJACK_PASTURE) {
		return COLOR_TEST_ANY;
	}
	else {
		return card_d->color;
	}
}

int get_color_of_mana_produced_by_id(int csvid, int info_slot, int player)
{
	switch (csvid) {
		case CARD_ID_CAVERN_OF_SOULS:
		case CARD_ID_CHANNEL:
		case CARD_ID_DOUBLING_CUBE:
		case CARD_ID_ELEMENTAL_RESONANCE:
			return COLOR_TEST_COLORLESS;

		case CARD_ID_CASCADE_BLUFFS:
		case CARD_ID_COMMAND_TOWER:
		case CARD_ID_FETID_HEATH:
		case CARD_ID_FIRE_LIT_THICKET:
		case CARD_ID_FLOODED_GROVE:
		case CARD_ID_GENERIC_ANIMATED_LAND:
		case CARD_ID_GRAVEN_CAIRNS:
		case CARD_ID_MYSTIC_GATE:
		case CARD_ID_OPAL_PALACE:
		case CARD_ID_RUGGED_PRAIRIE:
		case CARD_ID_SUNKEN_RUINS:
		case CARD_ID_TWILIGHT_MIRE:
		case CARD_ID_WOODED_BASTION:
		case CARD_ID_COMMANDERS_SPHERE:
		case CARD_ID_BLOOM_TENDER:
		case CARD_ID_AN_HAVVA_TOWNSHIP:
		case CARD_ID_AYSEN_ABBEY:
		case CARD_ID_CASTLE_SENGIR:
		case CARD_ID_KOSKUN_KEEP:
		case CARD_ID_WIZARDS_SCHOOL:
			return info_slot;

		case CARD_ID_CHROME_MOX:
		case CARD_ID_HONORED_HIERARCH:
		case CARD_ID_REALMWRIGHT:
			return info_slot >= 0 ? info_slot : 0;

		case CARD_ID_PRISMATIC_LENS:
		case CARD_ID_SHIMMERING_GROTTO:
		case CARD_ID_UNKNOWN_SHORES:
			return info_slot >= 0 ? info_slot : COLOR_TEST_COLORLESS | COLOR_TEST_ANY_COLORED;

		case CARD_ID_COLDSTEEL_HEART:
		case CARD_ID_GEM_BAZAAR:
		case CARD_ID_PARADISE_PLUME:
		case CARD_ID_UTOPIA_SPRAWL:
			return info_slot >= 0 ? info_slot : COLOR_TEST_ANY_COLORED;

		case CARD_ID_GEMSTONE_CAVERNS:
			return info_slot >= 0 ? info_slot : COLOR_TEST_COLORLESS;

		case CARD_ID_MIRRODINS_CORE:
		case CARD_ID_SOL_GRAIL:
		case CARD_ID_VERDANT_HAVEN:
			return info_slot >= 0 ? info_slot : COLOR_TEST_ANY_COLORED;

		case CARD_ID_PHYREXIAN_TOWER:
			return info_slot >= 0 ? info_slot : COLOR_TEST_COLORLESS | COLOR_TEST_BLACK;

		case CARD_ID_NIMBUS_MAZE:
			return info_slot >= 0 ? info_slot : COLOR_TEST_COLORLESS | COLOR_TEST_WHITE | COLOR_TEST_BLUE;

		case CARD_ID_RIVER_OF_TEARS:
			return info_slot >= 0 ? info_slot : COLOR_TEST_BLUE | COLOR_TEST_BLACK;

		case CARD_ID_TAINTED_FIELD:
			return info_slot >= 0 ? info_slot : COLOR_TEST_COLORLESS | COLOR_TEST_WHITE | COLOR_TEST_BLACK;

		case CARD_ID_TAINTED_ISLE:
			return info_slot >= 0 ? info_slot : COLOR_TEST_COLORLESS | COLOR_TEST_BLUE | COLOR_TEST_BLACK;

		case CARD_ID_TAINTED_PEAK:
			return info_slot >= 0 ? info_slot : COLOR_TEST_COLORLESS | COLOR_TEST_BLACK | COLOR_TEST_RED;

		case CARD_ID_TAINTED_WOOD:
			return info_slot >= 0 ? info_slot : COLOR_TEST_COLORLESS | COLOR_TEST_BLACK | COLOR_TEST_GREEN;

		case CARD_ID_TENDO_ICE_BRIDGE:
			return info_slot >= 0 ? info_slot : COLOR_TEST_COLORLESS | COLOR_TEST_ANY_COLORED;

		case CARD_ID_NYKTHOS_SHRINE_TO_NYX:
			return info_slot >= 0 ? info_slot : COLOR_TEST_COLORLESS | COLOR_TEST_ANY_COLORED;

		case CARD_ID_VIVID_CRAG:
			return info_slot >= 0 ? info_slot : COLOR_TEST_RED;

		case CARD_ID_STORMTIDE_LEVIATHAN:
			return info_slot >= 0 ? info_slot : COLOR_TEST_BLUE;

		case CARD_ID_VIVID_CREEK:
			return info_slot >= 0 ? info_slot : COLOR_TEST_BLUE;

		case CARD_ID_VIVID_GROVE:
		case CARD_ID_QUIRION_ELVES:
			return info_slot >= 0 ? info_slot : COLOR_TEST_GREEN;

		case CARD_ID_URBORG_TOMB_OF_YAWGMOTH:
			return info_slot >= 0 ? info_slot : COLOR_TEST_BLACK;

		case CARD_ID_VIVID_MARSH:
			return info_slot >= 0 ? info_slot : COLOR_TEST_BLACK;

		case CARD_ID_VIVID_MEADOW:
			return info_slot >= 0 ? info_slot : COLOR_TEST_WHITE;

		case CARD_ID_SPRINGJACK_PASTURE:
			return info_slot >= 0 && (info_slot & (1 << 30)) ? COLOR_TEST_ANY : COLOR_TEST_COLORLESS;

		case CARD_ID_EXOTIC_ORCHARD:
		case CARD_ID_FELLWAR_STONE:
		case CARD_ID_SYLVOK_EXPLORER:
			player = 1 - player;
			/* fall through */
		case CARD_ID_REFLECTING_POOL:
		case CARD_ID_SQUANDERED_RESOURCES:
		case CARD_ID_STAR_COMPASS: {
			if (info_slot == -1) {
				return COLOR_TEST_ANY_COLORED;
			}

			int count;
			int color = 0;
			int active_count = MIN(active_cards_count[player], MAX_CARD_SLOTS);

			for (count = 0; count < active_count && (color & 0x3F) != 0x3F; ++count) {
				card_data_t* card_d = get_card_data(player, count);
				if ((card_d->type & TYPE_LAND) && in_play(player, count) && card_d->id != CARD_ID_REFLECTING_POOL) {
					if (csvid != CARD_ID_STAR_COMPASS || has_subtype(player, count, SUBTYPE_BASIC)) {
						color |= get_colors_of_mana_land_could_produce_ignoring_costs(player, count);
					}
				}
			}

			if (csvid == CARD_ID_REFLECTING_POOL || csvid == CARD_ID_SQUANDERED_RESOURCES) {
				if (color & COLOR_TEST_ARTIFACT) {
					color &= ~COLOR_TEST_ARTIFACT;
					color |= COLOR_TEST_COLORLESS;
				}
			}
			else {
				color &= ~(COLOR_TEST_COLORLESS | COLOR_TEST_ARTIFACT);
			}

			return color;
		}

		default:
			return -1;
	}
}

/*
 * Stores last-hacked color_test in info_slot.  No avoiding it; it is needed by
 * get_color_of_mana_produced_by_id(), which doesn't get a card reference.
 */
int all_lands_are_basiclandtype(int player, int card, int event, int whose_lands, int land_color, int land_subtype)
{
	int remove_subtypes = 0;
	int add_subtypes = 0;

	if (event == EVENT_CAST_SPELL
		&& in_play(player, card)
		&& (affected_card_controller == whose_lands || whose_lands == ANYBODY)
		&& is_what(affected_card_controller, affected_card, TYPE_LAND)) {
		add_a_subtype(affected_card_controller, affected_card, get_hacked_subtype(player, card, land_subtype));
	}

	if (event == EVENT_CHANGE_TYPE
		&& in_play(player, card)
		&& (affected_card_controller == whose_lands || whose_lands == ANYBODY)) {
		card_instance_t* instance = get_card_instance(player, card);
		int col = 1 << get_hacked_color(player, card, land_color);
		if (instance->info_slot != col) {
			add_subtypes = 1;
			instance->info_slot = col;
		}
	}

	if (leaves_play(player, card, event)) {
		remove_subtypes = 1;
	}

	if (add_subtypes) {
		int p;
		int c;
		int subt = get_hacked_subtype(player, card, land_subtype);

		for (p = 0; p < 2; ++p) {
			if (p == whose_lands || whose_lands == ANYBODY) {
				int active_count = MIN(active_cards_count[p], MAX_CARD_SLOTS);
				for (c = 0; c < active_count; ++c) {
					if (in_play(p, c) && is_what(p, c, TYPE_LAND)) {
						add_a_subtype(p, c, subt);
					}
				}
			}
		}

		get_card_instance(player, card)->info_slot = 1 << get_hacked_color(player, card, land_color);
	}

	if (remove_subtypes) {
		test_definition_t this_test;
		default_test_definition(&this_test, TYPE_LAND);
		new_manipulate_all(player, card, whose_lands, &this_test, ACT_RESET_ADDED_SUBTYPE);
	}

	if (player == whose_lands || whose_lands == ANYBODY) {
		return permanents_you_control_can_tap_for_mana(player, card, event, TYPE_LAND, -1, get_hacked_color(player, card, land_color), 1);
	}
	else {
		return 0;
	}
}

void minimize_nondraining_mana(void)
{
	int p;
	int c;
	int nondraining;

	for (p = 0; p <= 1; ++p) {
		for (c = 0; c < 7; ++c) {
			if ((nondraining = (mana_doesnt_drain_from_pool[p][c] & MANADRAIN_AMT_MASK))
				&& mana_pool[p][c] < nondraining) {
				mana_doesnt_drain_from_pool[p][c] = mana_pool[p][c] | (mana_doesnt_drain_from_pool[p][c] & ~MANADRAIN_AMT_MASK);
			}
		}
	}
}

/*
 * A specific amount of a specific color of mana already in player's mana pool
 * doesn't drain until end of turn.
 */
void mana_doesnt_drain_until_eot(int player, color_t color, int amt)
{
	if (player < HUMAN || player > AI || !is_valid_mana_color(color) || amt < 0) {
		return;
	}

	int nondraining = mana_doesnt_drain_from_pool[player][color] & MANADRAIN_AMT_MASK;

	nondraining += amt;
	nondraining = MIN(nondraining, MANADRAIN_AMT_MASK);
	nondraining = MIN(nondraining, mana_pool[player][color]);

	mana_doesnt_drain_from_pool[player][color] &= ~MANADRAIN_AMT_MASK;
	mana_doesnt_drain_from_pool[player][color] |= nondraining;
}

int finalize_activation(int player, int card);

void mana_burn(void)
{
	if (!(trace_mode & 2)) {
	restart:;
		int c;
		card_instance_t* instance;
		int active_count = MIN(active_cards_count[AI], MAX_CARD_SLOTS);

		if (mana_pool[AI][7] && current_phase > PHASE_NORMAL_COMBAT_DAMAGE && active_count > 0) {
			for (c = 0; c < active_count; ++c) {
				if ((instance = in_play(AI, c))
					&& dispatch_event_with_attacker_to_one_card(AI, c, EVENT_CAN_WASTE_MANA, 1 - AI, -1)) {
					if (EXE_FN(int, 0x434040, int, int, int)(AI, AI, c)) {
						finalize_activation(AI, c);
					}
					goto restart;
				}
			}
		}
	}

	int p;
	int c;
	for (p = 0; p <= 1; ++p) {
		for (c = 0; c < 7; ++c) {
			mana_doesnt_drain_from_pool[p][c] &= MANADRAIN_AMT_MASK;
		}
	}

	dispatch_event(0, 0, EVENT_MANA_POOL_DRAINING);

	for (p = 0; p < 2; ++p) {
		int amt_drained = 0;
		int amt_left = 0;
		int clr;

		if (mana_pool[p][COLOR_ANY] > 0) {
			for (clr = COLOR_ARTIFACT; clr >= COLOR_COLORLESS; --clr) {
				if (mana_doesnt_drain_from_pool[p][clr] & MANADRAIN_DOESNT_DRAIN) {
					amt_left += mana_pool[p][clr];
				}
				else {
					uint8_t nondraining = mana_doesnt_drain_from_pool[p][clr] & MANADRAIN_AMT_MASK;

					if (nondraining) {
						if (nondraining > mana_pool[p][clr]) {
							nondraining = mana_pool[p][clr];
							mana_doesnt_drain_from_pool[p][clr] &= ~MANADRAIN_AMT_MASK;
							mana_doesnt_drain_from_pool[p][clr] |= nondraining;
						}
						mana_pool[p][clr] -= nondraining;
					}

					if (mana_doesnt_drain_from_pool[p][clr] & MANADRAIN_BECOMES_COLORLESS) {
						amt_left += mana_pool[p][clr];
						mana_pool[p][COLOR_COLORLESS] += mana_pool[p][clr];
					}
					else {
						amt_drained += mana_pool[p][clr];
					}

					mana_pool[p][clr] = nondraining;
					amt_left += nondraining;
				}
			}
		}

		mana_pool[p][COLOR_ANY] = amt_left;

		int setting_mana_burn;
		if (amt_drained > 0
			&& (setting_mana_burn = get_setting(SETTING_MANA_BURN))
			&& !(setting_mana_burn == 2 && IS_AI(p))) {
			if (ai_is_speculating != 1) {
				play_sound_effect(WAV_MANABURN);
				EXE_FN(void, 0x470250, int, int)(p, amt_drained);
			}
			life[p] -= amt_drained;
		}
	}
}