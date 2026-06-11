#include "manalink.h"

/*
 * Shared creature-boost helpers.
 *
 * These effects run while the engine is recalculating a single affected card.
 * The source card is {player, card}; the card currently being evaluated is
 * {affected_card_controller, affected_card}.  Keep the guardrails here small:
 * these functions are called often during power/toughness/ability passes.
 *
 * Special keywords are still only forwarded during EVENT_ABILITIES.  Do not use
 * boost_subtype() for special keywords that must react to other events.
 */

enum {
  BOOST_MAX_PLAYERS = 2,
  BOOST_MAX_CARD_SLOTS = 150,
  BOOST_ALL_SUBTYPES = -1
};

static int boost_is_real_player(int player)
{
  return player >= HUMAN && player <= AI;
}

static int boost_is_card_slot(int card)
{
  return card >= 0 && card < BOOST_MAX_CARD_SLOTS;
}

static int boost_is_card_ref(int player, int card)
{
  return boost_is_real_player(player) && boost_is_card_slot(card);
}

static int boost_event_has_work(event_t event, int power, int toughness, int abilities, int sp_abilities)
{
  return ((event == EVENT_POWER && power)
          || (event == EVENT_TOUGHNESS && toughness)
          || (event == EVENT_ABILITIES && (abilities || sp_abilities)));
}

static int boost_common_checks_pass(int player, int card, bct_t flags)
{
  if (!boost_is_card_ref(player, card)
      || !boost_is_card_ref(affected_card_controller, affected_card)) {
    return 0;
  }

  if (!in_play(player, card) || !in_play(affected_card_controller, affected_card)) {
    return 0;
  }

  if (!is_what(affected_card_controller, affected_card, TYPE_CREATURE)) {
    return 0;
  }

  if (is_humiliated(player, card)) {
    return 0;
  }

  if ((flags & BCT_CONTROLLER_ONLY) && affected_card_controller != player) {
    return 0;
  }

  if ((flags & BCT_OPPONENT_ONLY) && affected_card_controller == player) {
    return 0;
  }

  if (!(flags & BCT_INCLUDE_SELF) && affect_me(player, card)) {
    return 0;
  }

  return 1;
}

static void apply_boost_to_affected_card(int player, int card, event_t event, int power, int toughness, int abilities, int sp_abilities)
{
  if (event == EVENT_POWER) {
    event_result += power;
  } else if (event == EVENT_TOUGHNESS) {
    event_result += toughness;
  } else if (event == EVENT_ABILITIES) {
    event_result |= abilities;

    if (sp_abilities) {
      special_abilities(affected_card_controller, affected_card, event, sp_abilities, player, card);
    }
  }
}

/*
 * Boost creatures of a subtype while this source remains active.
 *
 * subtype may be BOOST_ALL_SUBTYPES/-1 to affect every creature that passes the
 * controller/self filters.  This helper deliberately uses has_creature_type()
 * instead of has_subtype() so cards with changeling and similar creature-type
 * logic are handled consistently.
 */
void boost_subtype(int player, int card, event_t event, subtype_t subtype, int power, int toughness, int abilities, int sp_abilities, bct_t flags)
{
  if (!boost_event_has_work(event, power, toughness, abilities, sp_abilities)) {
    return;
  }

  if (!boost_common_checks_pass(player, card, flags)) {
    return;
  }

  if ((int)subtype != BOOST_ALL_SUBTYPES
      && !has_creature_type(affected_card_controller, affected_card, subtype)) {
    return;
  }

  apply_boost_to_affected_card(player, card, event, power, toughness, abilities, sp_abilities);
}

int boost_creature_type(int player, int card, event_t event, subtype_t subtype, int power, int toughness, int abilities, bct_t flags)
{
  boost_subtype(player, card, event, subtype, power, toughness, abilities, 0, flags);
  return 0;
}

/*
 * Boost creatures whose current color overlaps clr.
 *
 * clr is a color_test_t-style bitfield.  Unless BCT_NO_SLEIGHT is set, color
 * words are sleighted from this source before matching the affected creature.
 */
int boost_creature_by_color(int player, int card, event_t event, int clr, int power, int toughness, int abilities, bct_t flags)
{
  if (!boost_event_has_work(event, power, toughness, abilities, 0)) {
    return 0;
  }

  if (!boost_common_checks_pass(player, card, flags)) {
    return 0;
  }

  clr &= COLOR_TEST_ANY_COLORED;
  if (!clr) {
    return 0;
  }

  if (!(flags & BCT_NO_SLEIGHT)) {
    clr = get_sleighted_color_test(player, card, clr);
  }

  if (!(get_color(affected_card_controller, affected_card) & clr)) {
    return 0;
  }

  apply_boost_to_affected_card(player, card, event, power, toughness, abilities, 0);

  return 0;
}