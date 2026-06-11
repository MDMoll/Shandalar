#include "manalink.h"

/*
 * Zone-counting helpers.
 *
 * Library, graveyard, and exile zones are all fixed 500-entry arrays terminated
 * by -1.  Most callers pass a real player, but graveyard-counting helpers also
 * accept ANYBODY and sum both graveyards.  Keep these routines non-mutating
 * except for purge_rfg().
 */

enum {
  ZONE_CARD_LIMIT = 500
};

static int zone_player_is_valid(int player)
{
  return player >= HUMAN && player <= AI;
}

static int zone_player_matches(int requested_player, int candidate_player)
{
  return requested_player == ANYBODY || requested_player == candidate_player;
}

static int count_zone_cards(const int* zone)
{
  if (!zone) {
    return 0;
  }

  int i;
  for (i = 0; i < ZONE_CARD_LIMIT; ++i) {
    if (zone[i] == -1) {
      return i;
    }
  }

  return ZONE_CARD_LIMIT;
}

static int count_graveyard_for_player(int player)
{
  return zone_player_is_valid(player) ? count_zone_cards(get_grave(player)) : 0;
}

static int count_rfg_for_player(int player)
{
  return zone_player_is_valid(player) ? count_zone_cards(rfg_ptr[player]) : 0;
}

static int count_zone_by_type(const int* zone, type_t type)
{
  if (!zone) {
    return 0;
  }

  int i;
  int result = 0;
  for (i = 0; i < ZONE_CARD_LIMIT && zone[i] != -1; ++i) {
    if (is_what(-1, zone[i], type)) {
      ++result;
    }
  }

  return result;
}

static int count_graveyard_by_color_for_player(int player, int selected_color)
{
  if (!zone_player_is_valid(player) || selected_color == 0) {
    return 0;
  }

  /*
   * Global color-changing effects apply to cards outside the battlefield too.
   * Preserve the old behavior: if the global color hack grants the requested
   * color, every card in that graveyard counts, including lands.
   */
  if (get_global_color_hack(player) & selected_color) {
    return count_graveyard_for_player(player);
  }

  const int* grave = get_grave(player);
  int i;
  int result = 0;

  for (i = 0; i < ZONE_CARD_LIMIT && grave[i] != -1; ++i) {
    int iid = grave[i];
    if (!is_what(-1, iid, TYPE_LAND) && (cards_data[iid].color & selected_color)) {
      ++result;
    }
  }

  return result;
}

int count_graveyard_by_color(int player, int selected_color)
{
  int p;
  int result = 0;

  for (p = HUMAN; p <= AI; ++p) {
    if (zone_player_matches(player, p)) {
      result += count_graveyard_by_color_for_player(p, selected_color);
    }
  }

  return result;
}

int count_graveyard_by_id(int player, int id)
{
  if (id < 0) {
    return 0;
  }

  int p;
  int result = 0;

  for (p = HUMAN; p <= AI; ++p) {
    if (zone_player_matches(player, p)) {
      const int* grave = get_grave(p);
      int i;

      for (i = 0; i < ZONE_CARD_LIMIT && grave[i] != -1; ++i) {
        if (cards_data[grave[i]].id == id) {
          ++result;
        }
      }
    }
  }

  return result;
}

// Count the number of cards in a sentinel-terminated zone.
int count_zone(int player, const int* zone)
{
  (void)player;
  return count_zone_cards(zone);
}

int count_deck(int player)
{
  return zone_player_is_valid(player) ? count_zone_cards(deck_ptr[player]) : 0;
}

int count_graveyard(int player)
{
  int p;
  int result = 0;

  for (p = HUMAN; p <= AI; ++p) {
    if (zone_player_matches(player, p)) {
      result += count_graveyard_for_player(p);
    }
  }

  return result;
}

int count_rfg(int player)
{
  if (player == ANYBODY) {
    return count_rfg_for_player(HUMAN) + count_rfg_for_player(AI);
  }

  return count_rfg_for_player(player);
}

void purge_rfg(int player)
{
  if (!zone_player_is_valid(player)) {
    return;
  }

  int count = count_rfg_for_player(player);
  if (count > 0) {
    rfg_ptr[player][count - 1] = -1;
  }
}

int count_graveyard_by_type(int player, int type)
{
  int p;
  int result = 0;

  for (p = HUMAN; p <= AI; ++p) {
    if (zone_player_matches(player, p)) {
      result += count_zone_by_type(get_grave(p), type);
    }
  }

  return result;
}

// Equivalent to count_graveyard_by_type(player, type) > 0, but stops when it finds the first qualifying card.
int any_in_graveyard_by_type(int player, int type)
{
  int p;

  for (p = HUMAN; p <= AI; ++p) {
    if (zone_player_matches(player, p)) {
      const int* grave = get_grave(p);
      int i;

      for (i = 0; i < ZONE_CARD_LIMIT && grave[i] != -1; ++i) {
        if (is_what(-1, grave[i], type)) {
          return 1;
        }
      }
    }
  }

  return 0;
}

int count_graveyard_by_subtype(int player, subtype_t subtype)
{
  int p;
  int result = 0;

  for (p = HUMAN; p <= AI; ++p) {
    if (zone_player_matches(player, p)) {
      const int* grave = get_grave(p);
      int i;

      for (i = 0; i < ZONE_CARD_LIMIT && grave[i] != -1; ++i) {
        if (has_subtype(-1, grave[i], subtype)) {
          ++result;
        }
      }
    }
  }

  return result;
}

int any_in_graveyard_by_subtype(int player, subtype_t subtype)
{
  int p;

  for (p = HUMAN; p <= AI; ++p) {
    if (zone_player_matches(player, p)) {
      const int* grave = get_grave(p);
      int i;

      for (i = 0; i < ZONE_CARD_LIMIT && grave[i] != -1; ++i) {
        if (has_subtype(-1, grave[i], subtype)) {
          return 1;
        }
      }
    }
  }

  return 0;
}

int count_deck_by_type(int player, int type)
{
  return zone_player_is_valid(player) ? count_zone_by_type(deck_ptr[player], type) : 0;
}

int new_special_count_grave(int player, test_definition_t* this_test)
{
  if (!this_test) {
    return 0;
  }

  int test_type = new_get_test_score(this_test);
  int p;
  int result = 0;

  for (p = HUMAN; p <= AI; ++p) {
    if (zone_player_matches(player, p)) {
      const int* grave = get_grave(p);
      int i;

      for (i = 0; i < ZONE_CARD_LIMIT && grave[i] != -1; ++i) {
        if (new_make_test(p, grave[i], test_type, this_test)) {
          ++result;
        }
      }
    }
  }

  return result;
}

int special_count_grave(int player, int type, int flag1, int subtype, int flag2, int color, int flag3, int id, int flag4, int cc, int flag5)
{
  int test_type = get_test_score(type, flag1, subtype, flag2, color, flag3, id, flag4, cc, flag5);
  int p;
  int result = 0;

  for (p = HUMAN; p <= AI; ++p) {
    if (zone_player_matches(player, p)) {
      const int* grave = get_grave(p);
      int count = count_graveyard_for_player(p) - 1;

      while (count > -1) {
        if (make_test(p, grave[count], test_type, type, flag1, subtype, flag2, color, flag3, id, flag4, cc, flag5)) {
          ++result;
        }
        --count;
      }
    }
  }

  return result;
}