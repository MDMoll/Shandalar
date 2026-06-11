// -*- c-basic-offset:2 -*-
// Low-level event, trigger, and card function handling.

#include "manalink.h"

/*
 * Event and trigger dispatch glue.
 *
 * This file sits between Manalink C card code and the original Shandalar event
 * machinery. Most functions here preserve or temporarily override global engine
 * state such as affected_card, trigger_condition, event_result, attacking_card,
 * and trigger_cause.
 *
 * Keep changes conservative. The engine depends on exact ordering around stack
 * resolution, trigger cleanup, timestamp iteration, and recalculation passes.
 * Guard obvious invalid references, but avoid broad rewrites unless the full
 * event contract is understood.
 */

// These are fairly common here, but shouldn't be used elsewhere.
#define push_affected_card_stack	EXE_FN(void, 0x435C80, void)
#define pop_affected_card_stack		EXE_FN(void, 0x435CD0, void)

typedef int (*CardFunction)(int, int, event_t);

enum {
  MAX_PLAYERS = 2,
  MAX_CARD_SLOTS = 150,
  MAX_TIMESTAMP_SLOTS = 500
};

static int is_real_player(int player)
{
  return player >= HUMAN && player <= AI;
}

static int is_card_slot(int card)
{
  return card >= 0 && card < MAX_CARD_SLOTS;
}

static int is_card_ref(int player, int card)
{
  return is_real_player(player) && is_card_slot(card);
}

static int bounded_active_cards_count(int player)
{
  return is_real_player(player) ? MIN(active_cards_count[player], MAX_CARD_SLOTS) : 0;
}

static int event_preserves_trigger_condition(event_t event)
{
  return event == EVENT_TRIGGER || event == EVENT_RESOLVE_TRIGGER || event == EVENT_END_TRIGGER;
}

static void clear_trigger_condition_unless_trigger_event(event_t event)
{
  if (!event_preserves_trigger_condition(event))
	trigger_condition = -1;
}

static card_instance_t* live_card_instance_or_null(int player, int card)
{
  if (!is_card_ref(player, card))
	return NULL;

  card_instance_t* instance = get_card_instance(player, card);
  if (!instance || instance->internal_card_id < 0)
	return NULL;

  return instance;
}

// A frontend for call_card_fn() where both address and instance should be computed from {player,card}.
int call_card_function(int player, int card, event_t event)
{
  card_instance_t* instance = live_card_instance_or_null(player, card);
  if (!instance)
	return 0;

  return call_card_function_i(instance, player, card, event);
}

// A frontend for call_card_fn() where address should be computed from instance.
int call_card_function_i(card_instance_t* instance, int player, int card, event_t event)
{
  if (!instance || instance->internal_card_id < 0)
	return 0;

  CardFunction fn = (CardFunction)(cards_data[instance->internal_card_id].code_pointer);
  return call_card_fn(fn, instance, player, card, event);
}

int call_card_fn_impl(void* address, card_instance_t* instance, int player, int card, event_t event);	// never call directly

// Puts instance into esi, then calls address(player, card, event).
int call_card_fn(void* address, card_instance_t* instance, int player, int card, event_t event)
{
  if (!address)
	return 0;

  int old_trigger_condition = trigger_condition;
  clear_trigger_condition_unless_trigger_event(event);

  int rval = call_card_fn_impl(address, instance, player, card, event);

  trigger_condition = old_trigger_condition;

  return rval;
}

extern int timestamp_card[MAX_TIMESTAMP_SLOTS];
extern int timestamp_player[MAX_TIMESTAMP_SLOTS];

void dispatch_event_raw(event_t event)
{
  /*
   * Walk all timestamped cards and dispatch event to those currently in play or
   * otherwise visible to the event system. The timestamp arrays define ordering;
   * card indexes can change while events resolve, so each slot is revalidated
   * before use.
   */
  minimize_nondraining_mana();

  /*
   * Some callers use dispatch_event_raw() directly and do not preserve these
   * globals even though they should. Preserve them here as the last defensive
   * wall before card code runs.
   */
  int old_affected_card_controller = affected_card_controller;
  int old_affected_card = affected_card;

  /*
   * Non-trigger events should not look like old triggers. Many card helpers
   * first check trigger_condition before doing heavier work.
   */
  int old_trigger_condition = trigger_condition;
  clear_trigger_condition_unless_trigger_event(event);

  /*
   * In the exe, attacking cards were tapped in the same loop that dispatched
   * EVENT_DECLARE_ATTACKERS. Doing it first is less surprising: all cards see a
   * consistent attacking/tapped state during the event.
   */
  unsigned int ts;
  if (event == EVENT_DECLARE_ATTACKERS)
	{
	  number_of_attackers_declared = 0;

	  for (ts = 0; ts < MAX_TIMESTAMP_SLOTS && timestamp_player[ts] != -1; ++ts)
		{
		  int player = timestamp_player[ts];
		  int card = timestamp_card[ts];

		  if (!is_card_ref(player, card))
			continue;

		  card_instance_t* instance;
		  if (player == current_turn
			  && (instance = in_play(player, card))
			  && instance->timestamp == (int)ts
			  && (instance->state & STATE_ATTACKING))
			{
			  ++number_of_attackers_declared;

			  if (!(instance->state & STATE_TAPPED)
				  && !(get_special_abilities_by_instance(instance) & SP_KEYWORD_VIGILANCE))
				tap_card(player, card);
			}
		}
	}

  if (event == EVENT_CAN_SKIP_TURN)	// First event dispatched every turn, used only by skip-turn effects and housekeeping.
	enable_xtrigger_flags = 0;

  for (ts = 0; ts < MAX_TIMESTAMP_SLOTS && timestamp_player[ts] != -1; ++ts)
	{
	  int player = timestamp_player[ts];
	  int card = timestamp_card[ts];

	  if (!is_card_ref(player, card))
		continue;

	  card_instance_t* instance = get_card_instance(player, card);
	  if (instance->timestamp == (int)ts
		  && instance->internal_card_id != -1
		  && !(instance->state & STATE_OUBLIETTED)
		  && (instance->state & (STATE_INVISIBLE | STATE_IN_PLAY)))
		{
		  CardFunction fn = (CardFunction)(cards_data[instance->internal_card_id].code_pointer);
		  call_card_fn(fn, instance, player, card, event);
		}
	}

  if (event == EVENT_DECLARE_ATTACKERS && number_of_attackers_declared > 0)
	{
	  dispatch_xtrigger2(current_turn, XTRIGGER_ATTACKING, "attacking", 0, 0, 0);

	  /*
	   * These were previously run after each attacking card tapped. That was both
	   * too many times and not enough, since non-attacking cards also handle
	   * EVENT_DECLARE_ATTACKERS.
	   */
	  EXE_FN(void, 0x477070, void)();	// resolve_damage_cards_and_prevent_damage()
	  EXE_FN(void, 0x477a90, void)();	// regenerate_or_graveyard_triggers()

	  recalculate_all_cards_in_play();
	}

  affected_card_controller = old_affected_card_controller;
  affected_card = old_affected_card;
  trigger_condition = old_trigger_condition;

  if (event == EVENT_TAPPED_TO_PLAY_ABILITY
	  || event == EVENT_PLAY_ABILITY
	  || (event == EVENT_CAST_SPELL
		  && (!is_card_ref(affected_card_controller, affected_card)
			  || !check_state(affected_card_controller, affected_card, STATE_IN_PLAY))))	// actually cast, not put_into_play
	recalculate_all_cards_in_play();
}

// dispatch_event() forces the initial value of event_result to 0; this lets you set it arbitrarily.
int dispatch_event_with_initial_event_result(int new_affected_card_controller, int new_affected_card, event_t event, int initial_event_result)
{
  /*
   * When the exe needs to do this, it inlines everything here. This wrapper keeps
   * the global-state save/restore sequence in one place.
   */
  push_affected_card_stack();

  event_result = initial_event_result;
  affected_card_controller = new_affected_card_controller;
  affected_card = new_affected_card;
  attacking_card_controller = 1 - new_affected_card_controller;
  attacking_card = -1;

  int old_trigger_condition = trigger_condition;
  clear_trigger_condition_unless_trigger_event(event);

  dispatch_event_raw(event);

  int new_event_result = event_result;

  trigger_condition = old_trigger_condition;
  pop_affected_card_stack();

  return new_event_result;
}

/*
 * dispatch_event() forces attacking_card_controller to 1-player and attacking_card
 * to -1; this lets callers set them explicitly. Returns event_result as set by
 * card functions before restoring the previous global context.
 */
int dispatch_event_with_attacker(int new_affected_card_controller, int new_affected_card, event_t event, int new_attacking_card_controller, int new_attacking_card)
{
  push_affected_card_stack();

  event_result = 0;
  affected_card_controller = new_affected_card_controller;
  affected_card = new_affected_card;
  attacking_card_controller = new_attacking_card_controller;
  attacking_card = new_attacking_card;

  int old_trigger_condition = trigger_condition;
  clear_trigger_condition_unless_trigger_event(event);

  dispatch_event_raw(event);

  int new_event_result = event_result;

  trigger_condition = old_trigger_condition;
  pop_affected_card_stack();

  return new_event_result;
}

/*
 * Identical to dispatch_event_with_attacker(), but only sends the event to
 * (new_affected_card_controller,new_affected_card). Differs from bare
 * call_card_function() in that it preserves affected_card_controller,
 * affected_card, trigger_condition, event_result, etc.
 */
int dispatch_event_with_attacker_to_one_card(int new_affected_card_controller, int new_affected_card, event_t event, int new_attacking_card_controller, int new_attacking_card)
{
  /*
   * 0x435B50 is almost equivalent, but returns the card function's return value
   * instead of event_result, and has special cases we don't need.
   */
  return dispatch_event_arbitrary_to_one_card(new_affected_card_controller, new_affected_card,
											  event,
											  new_affected_card_controller, new_affected_card,
											  new_attacking_card_controller, new_attacking_card);
}

int event_rval;

/*
 * Identical to dispatch_event_with_attacker_to_one_card(), but sends the event
 * to an arbitrary (player, card) pair which can differ from
 * (affected_card_controller, affected_card).
 */
int dispatch_event_arbitrary_to_one_card(int player, int card, event_t event, int new_affected_card_controller, int new_affected_card, int new_attacking_card_controller, int new_attacking_card)
{
  push_affected_card_stack();

  event_result = 0;
  affected_card_controller = new_affected_card_controller;
  affected_card = new_affected_card;
  attacking_card_controller = new_attacking_card_controller;
  attacking_card = new_attacking_card;

  int old_trigger_condition = trigger_condition;
  clear_trigger_condition_unless_trigger_event(event);

  event_rval = call_card_function(player, card, event);

  int new_event_result = event_result;

  trigger_condition = old_trigger_condition;
  pop_affected_card_stack();

  return new_event_result;
}

/* The semantics of this one are a bit unusual. Avoid unless you know exactly why you need it. */
int dispatch_event_to_single_card(int player, int card, event_t event, int new_attacking_card_controller, int new_attacking_card)
{
  if (!is_card_ref(player, card))
	return 0;

  card_instance_t* inst = get_card_instance(player, card);
  if (inst->internal_card_id == -1)
	return 0;

  if (event == EVENT_CAN_ACTIVATE
	  && ((inst->state & STATE_OUBLIETTED) || inst->upkeep_flags & 8))
	return 0;

  push_affected_card_stack();

  event_result = 0;
  affected_card_controller = player;
  affected_card = card;
  attacking_card_controller = new_attacking_card_controller;
  attacking_card = new_attacking_card;
  int old_736808 = EXE_DWORD(0x736808);

  int old_trigger_condition = trigger_condition;
  clear_trigger_condition_unless_trigger_event(event);

  int rval = call_card_function_i(inst, player, card, event);

  trigger_condition = old_trigger_condition;

  if (rval == 99
	  || !(land_can_be_played & (LCBP_REGENERATION | LCBP_SPELL_BEING_PLAYED | LCBP_DAMAGE_PREVENTION))
	  || !(event == EVENT_CAN_CAST || event == EVENT_CAN_ACTIVATE)
	  || EXE_FN(int, 0x435c30, int, int)(player, card)	// is_mana_source_but_not_act_ability
	  || (land_can_be_played & LCBP_CHARGING_MANA))
	EXE_DWORD(0x739D58) = event_result;
  else
	{
	  EXE_DWORD(0x736808) = old_736808;
	  rval = 0;
	}

  pop_affected_card_stack();
  return rval;
}

// Properly stashes globals then calls an arbitrary iid's card function for player/card instead of that card's own one.
int dispatch_event_to_single_card_overriding_function(int player, int card, event_t event, int iid)
{
  if (!is_card_ref(player, card) || iid < 0)
	return 0;

  push_affected_card_stack();

  event_result = 0;
  affected_card_controller = player;
  affected_card = card;
  attacking_card_controller = 1-player;
  attacking_card = -1;

  int old_trigger_condition = trigger_condition;
  clear_trigger_condition_unless_trigger_event(event);

  CardFunction fn = (CardFunction)(cards_data[iid].code_pointer);
  int rval = call_card_fn(fn, get_card_instance(player, card), player, card, event);

  trigger_condition = old_trigger_condition;
  pop_affected_card_stack();

  return rval;
}

int dispatch_trigger_to_one_card(int player, int card, event_t event)
{
  // 0x477970
  if (!is_card_ref(player, card))
	return 0;

  if (event == EVENT_TRIGGER
	  && (EXE_DWORD(0x60E9F8)	// suppress_this_trigger
		  || (get_card_instance(player, card)->state & STATE_PROCESSING)))
	return 0;

  if (trigger_condition < 200)
	return 0;

  push_affected_card_stack();	// missing in original, along with the pop below

  event_result = 0;
  affected_card_controller = player;
  affected_card = card;
  attacking_card = -1;
  dispatch_event_raw(event);
  int rval = event_result;

  pop_affected_card_stack();

  return rval;
}

xtrigger_t xtrigger_impl_value_dont_use_directly = 0;
enable_xtrigger_flags_t enable_xtrigger_flags = 0;

static void dispatch_trigger_impl(int player, trigger_t trig, xtrigger_t xtrig, const char *prompt, int TENTATIVE_allow_response)
{
  /*
   * Original at 0x4371E0.
   *
   * The trigger dispatcher temporarily marks cards as processing so the same
   * card doesn't repeatedly handle the same trigger. Nested triggers complicate
   * that state, so this function saves and restores STATE_PROCESSING and
   * STATE_IS_TRIGGERING for cards that already existed before the nested trigger.
   */
#define TRIGGER_DEPTH	EXE_DWORD(0x785E60)

  ++TRIGGER_DEPTH;
  int orig_tcc = trigger_cause_controller;
  int orig_tc = trigger_cause;
  int old_life_gained = life_gained;
  int old_62C180 = EXE_DWORD(0x62C180);
  int old_reason_for_trigger_controller = reason_for_trigger_controller;
  int old_62BCE8 = EXE_DWORD(0x62BCE8);

  EXE_DWORD(0x62BCE8) = trig;
  reason_for_trigger_controller = player;

  int p, c;

  char processing[2][MAX_CARD_SLOTS];
  int processing_set = 0;
  if (TRIGGER_DEPTH != 1)
	{
	  processing_set = 1;
	  for (p = 0; p < 2; ++p)
		{
		  int active_count = bounded_active_cards_count(p);

		  for (c = 0; c < active_count; ++c)
			{
			  card_instance_t* instance = get_card_instance(p, c);
			  if (instance->internal_card_id == -1)
				processing[p][c] = 2;
			  else if (instance->state & STATE_PROCESSING)
				{
				  processing[p][c] = 1;
				  instance->state &= ~STATE_PROCESSING;
				}
			  else
				processing[p][c] = 0;

			  if (instance->state & STATE_IS_TRIGGERING)
				{
				  processing[p][c] |= 4;
				  instance->state &= ~STATE_IS_TRIGGERING;
				}
			}

		  for (c = active_count; c < MAX_CARD_SLOTS; ++c)
			processing[p][c] = 2;
		}
	}

  unsigned int v8;
  do
	{
	  int old_620860 = EXE_DWORD(0x620860);
	  if (trace_mode & 2)
		EXE_DWORD(0x620860) = 1;
	  else if (player)
		EXE_DWORD(0x620860) = 2;
	  else
		EXE_DWORD(0x620860) = 1;

	  trigger_condition = trig;
	  xtrigger_impl_value_dont_use_directly = xtrig;

	  int old_60A554 = EXE_DWORD(0x60A554);
	  if (TENTATIVE_allow_response)
		EXE_DWORD(0x60A554) = TYPE_INSTANT | TYPE_INTERRUPT;
	  else
		EXE_DWORD(0x60A554) = 0;

	  cancel = 0;
	  EXE_DWORD(0x62852C) = 0;
	  EXE_DWORD(0x62C180) = 0;

	  v8 = EXE_FN(int, 0x475A30, int, const char*)(player, prompt);

	  EXE_DWORD(0x60A554) = old_60A554;
	  EXE_DWORD(0x60A53C) = old_60A554 & (TYPE_INSTANT | TYPE_INTERRUPT);
	  EXE_DWORD(0x620860) = old_620860;
	}
  while (((v8 < 1 ? 4 : 6) & EXE_DWORD(0x62C180))
		 || (TENTATIVE_allow_response && v8));

  /*
   * Do not just call dispatch_event(... EVENT_END_TRIGGER), since that clears
   * trigger_condition. Cards that queued internal trigger state use
   * EVENT_END_TRIGGER to clean it up under the same trigger context.
   */
  trigger_condition = trig;
  xtrigger_impl_value_dont_use_directly = xtrig;
  reason_for_trigger_controller = player;
  trigger_cause = orig_tc;
  trigger_cause_controller = orig_tcc;

  int old_aff_cc = affected_card_controller;
  int old_aff_c = affected_card;
  int old_ev = event_result;

  card_instance_t* inst;
  for (p = 0; p < 2; ++p)
	{
	  int active_count = bounded_active_cards_count(p);
	  for (c = 0; c < active_count; ++c)
		if ((inst = in_play(p, c)))
		  {
			affected_card_controller = p;
			affected_card = c;
			call_card_function_i(inst, p, c, EVENT_END_TRIGGER);
		  }
	}

  affected_card_controller = old_aff_cc;
  affected_card = old_aff_c;
  event_result = old_ev;
  trigger_cause = orig_tc;
  trigger_cause_controller = orig_tcc;

  trigger_condition = -1;

  --TRIGGER_DEPTH;
  if (TRIGGER_DEPTH == 0)
	{
	  for (p = 0; p < 2; ++p)
		{
		  int active_count = bounded_active_cards_count(p);
		  for (c = 0; c < active_count; ++c)
			get_card_instance(p, c)->state &= ~(STATE_PROCESSING | STATE_IS_TRIGGERING);
		}

	  if (!EXE_DWORD(0x7A31AC))
		{
		  EXE_DWORD(0x60A538) = 0;
		  if (ai_is_speculating != 1)
			EXE_DWORD(0x60A54C) = 0;
		}
	}
  else
	{
	  ASSERT(processing_set);
	  for (p = 0; p < 2; ++p)
		{
		  int active_count = bounded_active_cards_count(p);
		  for (c = 0; c < active_count; ++c)
			{
			  card_instance_t* instance = get_card_instance(p, c);

			  if (instance->internal_card_id == -1)
				instance->state &= ~(STATE_PROCESSING | STATE_IS_TRIGGERING);
			  else
				{
				  if (processing[p][c] & 4)
					{
					  instance->state |= STATE_IS_TRIGGERING;
					  processing[p][c] &= ~4;
					}

				  if (processing[p][c])
					instance->state |= STATE_PROCESSING;
				  else
					instance->state &= ~STATE_PROCESSING;
				}
			}
		}
	}

  EXE_DWORD(0x62C180) = old_62C180;
  reason_for_trigger_controller = old_reason_for_trigger_controller;
  EXE_DWORD(0x62BCE8) = old_62BCE8;
  life_gained = old_life_gained;

#undef TRIGGER_DEPTH
}

int dispatch_trigger(int player, trigger_t trig, const char *prompt, int TENTATIVE_allow_response)
{
  xtrigger_t old_xtrigger = xtrigger_impl_value_dont_use_directly;
  dispatch_trigger_impl(player, trig, 0, prompt, TENTATIVE_allow_response);
  xtrigger_impl_value_dont_use_directly = old_xtrigger;
  return 0;
}

int dispatch_trigger2(int player, trigger_t trig, const char *prompt, int TENTATIVE_allow_response, int new_trigger_cause_controller, int new_trigger_cause)
{
  int old_trigger_cause = trigger_cause;
  int old_trigger_cause_controller = trigger_cause_controller;
  xtrigger_t old_xtrigger = xtrigger_impl_value_dont_use_directly;

  trigger_cause_controller = new_trigger_cause_controller;
  trigger_cause = new_trigger_cause;

  dispatch_trigger_impl(player, trig, 0, prompt, TENTATIVE_allow_response);
  dispatch_trigger_impl(1-player, trig, 0, prompt, TENTATIVE_allow_response);

  trigger_cause = old_trigger_cause;
  trigger_cause_controller = old_trigger_cause_controller;
  xtrigger_impl_value_dont_use_directly = old_xtrigger;

  return 0;
}

int dispatch_xtrigger2(int player, xtrigger_t xtrig, const char *prompt, int TENTATIVE_allow_response, int new_trigger_cause_controller, int new_trigger_cause)
{
  int old_trigger_cause = trigger_cause;
  int old_trigger_cause_controller = trigger_cause_controller;
  xtrigger_t old_xtrigger = xtrigger_impl_value_dont_use_directly;

  trigger_cause_controller = new_trigger_cause_controller;
  trigger_cause = new_trigger_cause;

  dispatch_trigger_impl(player, TRIGGER_XTRIGGER, xtrig, prompt, TENTATIVE_allow_response);
  dispatch_trigger_impl(1-player, TRIGGER_XTRIGGER, xtrig, prompt, TENTATIVE_allow_response);

  trigger_cause = old_trigger_cause;
  trigger_cause_controller = old_trigger_cause_controller;
  xtrigger_impl_value_dont_use_directly = old_xtrigger;

  return 0;
}

static int should_resolve_ai_land_cip_trigger_without_stack(int player, int card)
{
  if (ai_is_speculating == 1 || (trace_mode & 2) || player != AI)
	return 0;

  if (trigger_condition != TRIGGER_COMES_INTO_PLAY
	  || reason_for_trigger_controller != player
	  || trigger_cause_controller != player
	  || trigger_cause != card)
	return 0;

  return in_play(player, card) && is_what(player, card, TYPE_LAND);
}

static void clear_is_triggering_on_all_cards(void)
{
  int p, c;
  for (p = 0; p <= 1; ++p)
	{
	  int active_count = bounded_active_cards_count(p);
	  for (c = 0; c < active_count; ++c)
		get_card_instance(p, c)->state &= ~STATE_IS_TRIGGERING;
	}
}

static void dispatch_resolve_trigger_to_one_card_without_stack(int player, int card)
{
  dispatch_event_with_attacker_to_one_card(player, card, EVENT_RESOLVE_TRIGGER, 1-player, -1);
}

static void resolve_ai_land_cip_trigger_without_stack(int player, int card)
{
  /*
   * AI land CIP triggers are safe to resolve directly without stack UI. This is
   * a narrow optimization because land CIP effects are generally forced and
   * noninteractive in AI play.
   */
  clear_is_triggering_on_all_cards();

  card_instance_t* instance = get_card_instance(player, card);
  instance->state |= STATE_PROCESSING | STATE_IS_TRIGGERING;

  int old_tcond = trigger_condition;
  int old_xtcond = xtrigger_impl_value_dont_use_directly;
  int old_tcc = trigger_cause_controller;
  int old_tc = trigger_cause;

  dispatch_resolve_trigger_to_one_card_without_stack(player, card);

  trigger_condition = old_tcond;
  xtrigger_impl_value_dont_use_directly = old_xtcond;
  trigger_cause = old_tc;
  trigger_cause_controller = old_tcc;
  instance->state &= ~(STATE_PROCESSING | STATE_IS_TRIGGERING);

  EXE_DWORD(0x60E9F8) = 0;

  dispatch_event(player, card, EVENT_TRIGGER_RESOLVED);

  trigger_condition = old_tcond;
  xtrigger_impl_value_dont_use_directly = old_xtcond;
  trigger_cause = old_tc;
  trigger_cause_controller = old_tcc;
}

void resolve_trigger(int player, int card, int TENTATIVE_reason_for_trigger_controller)
{
  // Original at 434800
  if (!is_card_ref(player, card))
	return;

  if (trace_mode & 2)
	{
	  char buf[500];
	  card_instance_t* trace_instance = get_card_instance(player, card);
	  const char* name = trace_instance->internal_card_id >= 0 ? cards_data[trace_instance->internal_card_id].name : "<invalid>";
	  scnprintf(buf, sizeof(buf), "%d: Player #%d is processing %s(%d).\n", EXE_DWORD(0x60EC40)++, player, name, card);
	  EXE_FN(void, 0x4A7D80, const char*)(buf);	// append_to_trace_txt()
	}

  EXE_DWORD(0x60E9F8) = 0;	// suppress_this_trigger; only ever reset, but keep it for exe parity

  if (should_resolve_ai_land_cip_trigger_without_stack(player, card))
	{
	  resolve_ai_land_cip_trigger_without_stack(player, card);
	  return;
	}

  put_card_or_activation_onto_stack(player, card, EVENT_RESOLVE_TRIGGER, TENTATIVE_reason_for_trigger_controller, 0);
  if (cancel == 1)
	{
	  obliterate_top_card_of_stack();
	  EXE_DWORD(0x60E9F8) = 0;
	}
  else
	{
	  if (ai_is_speculating != 1)
		{
		  EXE_FN(void, 0x436700, void)();	// set_stack_damage_targets()

		  if (reason_for_trigger_controller != 0)
			{
			  load_text(0, "PROMPT_PROC1");
			  char buf[300];
			  scnprintf(buf, sizeof(buf), text_lines[0], opponent_name);
			  EXE_FN(int, 0x471b60, int, int, int, int, const char*, int)(player, card, -1, -1, buf, 0);	// raw_do_dialog()
			}

		  clear_is_triggering_on_all_cards();
		}

	  card_instance_t* instance = get_card_instance(player, card);
	  instance->state |= STATE_PROCESSING | STATE_IS_TRIGGERING;

	  int old_tcond = trigger_condition;
	  int old_xtcond = xtrigger_impl_value_dont_use_directly;
	  int old_tcc = trigger_cause_controller;
	  int old_tc = trigger_cause;

	  EXE_FN(void, 0x436740, void)();	// resolve_top_card_on_stack()

	  trigger_condition = old_tcond;
	  xtrigger_impl_value_dont_use_directly = old_xtcond;
	  trigger_cause = old_tc;
	  trigger_cause_controller = old_tcc;
	  instance->state &= ~STATE_IS_TRIGGERING;

	  EXE_DWORD(0x60E9F8) = 0;

	  dispatch_event(player, card, EVENT_TRIGGER_RESOLVED);

	  trigger_condition = old_tcond;
	  xtrigger_impl_value_dont_use_directly = old_xtcond;
	  trigger_cause = old_tc;
	  trigger_cause_controller = old_tcc;
	}
}

int get_abilities(int player, int card, event_t event, int new_attacking_card)
{
  // 0x4352d0
  if (!is_card_ref(player, card))
	return event == EVENT_CHANGE_TYPE ? -1 : 0;

  card_instance_t* instance = get_card_instance(player, card);
  int iid = instance->internal_card_id;

  if (iid == activation_card)
	{
	  if (event == EVENT_CHANGE_TYPE)
		return iid;
	  else if (is_card_ref(instance->parent_controller, instance->parent_card))
		return get_abilities(instance->parent_controller, instance->parent_card, event, new_attacking_card);
	  else
		return 0;
	}

  if (EXE_DWORD(0x60A548))
	push_affected_card_stack();

  affected_card_controller = player;
  affected_card = card;

  if (iid == -1)
	{
	  iid = instance->backup_internal_card_id;
	  instance->regen_status &= ~KEYWORD_RECALC_ALL;
	}

  if (iid < 0)
	{
	  if (EXE_DWORD(0x60A548))
		pop_affected_card_stack();
	  return event == EVENT_CHANGE_TYPE ? -1 : 0;
	}

  EXE_DWORD(0x73825C) = iid;
  attacking_card = new_attacking_card;

  int preliminary = 0;
  int recalc = 0;

  switch (event)
	{
	  case EVENT_POWER:
		if (instance->regen_status & KEYWORD_RECALC_POWER)
		  {
			instance->regen_status &= ~KEYWORD_RECALC_POWER;
			preliminary = cards_data[iid].power;
			if ((instance->state & (STATE_OUBLIETTED|STATE_IN_PLAY)) == STATE_IN_PLAY && preliminary > 0)
			  preliminary &= ~0x4000;
			preliminary += instance->counter_power;
			recalc = 1;
		  }
		else
		  event_result = instance->power;
		break;

	  case EVENT_TOUGHNESS:
		if (instance->regen_status & KEYWORD_RECALC_TOUGHNESS)
		  {
			instance->regen_status &= ~KEYWORD_RECALC_TOUGHNESS;
			preliminary = cards_data[iid].toughness;
			if ((instance->state & (STATE_OUBLIETTED|STATE_IN_PLAY)) == STATE_IN_PLAY && preliminary > 0)
			  preliminary &= ~0x4000;
			preliminary += instance->counter_toughness;
			recalc = 1;
		  }
		else
		  event_result = instance->toughness;
		break;

	  case EVENT_ABILITIES:
		if (instance->regen_status & KEYWORD_RECALC_ABILITIES)
		  {
			instance->regen_status &= ~KEYWORD_RECALC_ABILITIES;

			if (iid == damage_card)
			  {
				event_result = instance->regen_status;
				break;
			  }

			preliminary = ((instance->regen_status & (KEYWORD_RECALC_SET_COLOR|KEYWORD_RECALC_POWER|KEYWORD_RECALC_TOUGHNESS|KEYWORD_RECALC_CHANGE_TYPE))
						   | cards_data[iid].static_ability);

			if (preliminary & (KEYWORD_PROT_COLORED | KEYWORD_BASIC_LANDWALK))
			  {
				if (preliminary & KEYWORD_PROT_COLORED)
				  preliminary = (preliminary & ~KEYWORD_PROT_COLORED) | get_sleighted_protection(player, card, (preliminary & KEYWORD_PROT_COLORED));

				if (preliminary & KEYWORD_BASIC_LANDWALK)
				  preliminary = (preliminary & ~KEYWORD_BASIC_LANDWALK) | get_hacked_walk(player, card, (preliminary & KEYWORD_BASIC_LANDWALK));
			  }

			if (EXE_DWORD(0x60A548))
			  {
				int typ = cards_data[iid].type;
				if ((typ & TYPE_PERMANENT) && !(typ & TYPE_EFFECT))
				  {
					instance->targets[16].card = 0;
					instance->state &= ~STATE_VIGILANCE;
					instance->token_status &= ~(STATUS_WALL_CAN_ATTACK | STATUS_CANT_ATTACK);
					remove_special_flags(player, card, SF_MODULAR | SF_HEXPROOF_OVERRIDE);
					remove_special_flags2(player, card, SF2_THOUSAND_YEAR_ELIXIR);
				  }
			  }

			recalc = 1;
		  }
		else
		  event_result = instance->regen_status;
		break;

	  case EVENT_CHANGE_TYPE:
		if (instance->internal_card_id == -1)
		  event_result = -1;
		else if ((instance->regen_status & KEYWORD_RECALC_CHANGE_TYPE)
				 && (iid < EXE_DWORD(0x628c64)/*iid_legacy_data_card*/ || iid > EXE_DWORD(0x786d5c)/*iid_legacy_swap_power_toughness*/))
		  {
			instance->regen_status &= ~KEYWORD_RECALC_CHANGE_TYPE;
			instance->backup_internal_card_id = preliminary = instance->internal_card_id = instance->original_internal_card_id;
			instance->mana_color = cards_data[instance->internal_card_id].color;
			instance->destroys_if_blocked = 0;
			instance->token_status &= ~STATUS_LEGACY_TYPECHANGE;
			remove_special_flags2(player, card, SF2_ENCHANTED_EVENING | SF2_MYCOSYNTH_LATTICE | SF2_TEMPORARY_COPY_OF_TOKEN);
			recalc = 1;
		  }
		else
		  event_result = iid;
		break;

	  case EVENT_RECALC_DAMAGE:
		preliminary = instance->damage_on_card;
		recalc = 1;
		break;

	  case EVENT_SET_COLOR:
		if (instance->regen_status & KEYWORD_RECALC_SET_COLOR)
		  {
			instance->regen_status &= ~KEYWORD_RECALC_SET_COLOR;

			if (iid == damage_card)
			  {
				event_result = instance->color;
				break;
			  }

			if (instance->initial_color)
			  preliminary = instance->initial_color;
			else
			  {
				card_data_t* cd = &cards_data[iid], *unaltered_cd;

				/*
				 * Lands and mana-producing artifacts keep mana color in cd->color.
				 * Their actual color is colorless, except Dryad Arbor.
				 */
				if (cd->type & TYPE_LAND)
				  preliminary = cards_data[iid].id == CARD_ID_DRYAD_ARBOR ? COLOR_TEST_GREEN : 0;
				else if ((cd->type & TYPE_ARTIFACT) && (cd->extra_ability & EA_MANA_SOURCE)
						 && (unaltered_cd = &cards_data[get_internal_card_id_from_csv_id(cd->id)])
						 && (unaltered_cd->type & TYPE_ARTIFACT) && (unaltered_cd->extra_ability & EA_MANA_SOURCE))
				  preliminary = 0;
				else
				  preliminary = cd->color;
			  }

			preliminary &= COLOR_TEST_ANY_COLORED;
			recalc = 1;
		  }
		else
		  event_result = instance->color;
		break;

	  default:
		preliminary = 0;
		recalc = 1;
		break;
	}

  if (recalc)
	{
	  event_result = preliminary;
	  if (EXE_DWORD(0x60A548))
		{
		  dispatch_event_raw(event);

		  if ((land_can_be_played & LCBP_NEED_EVENT_CHANGE_TYPE_SECOND_PASS) && event == EVENT_CHANGE_TYPE)
			{
			  land_can_be_played &= ~LCBP_NEED_EVENT_CHANGE_TYPE_SECOND_PASS;
			  instance->internal_card_id = event_result;
			  if (event_result != -1)
				instance->backup_internal_card_id = event_result;

			  land_can_be_played |= LCBP_DURING_EVENT_CHANGE_TYPE_SECOND_PASS;
			  dispatch_event_raw(event);
			  land_can_be_played &= ~LCBP_DURING_EVENT_CHANGE_TYPE_SECOND_PASS;
			}
		}

	  if (event == EVENT_ABILITIES && (player_bits[player] & PB_CANT_HAVE_OR_GAIN_ABILITIES_MASK))
		{
		  int typ = cards_data[iid].type;
		  if ((typ & TYPE_PERMANENT) && !(typ & TYPE_EFFECT))
			{
			  if (player_bits[player] & PB_CANT_HAVE_OR_GAIN_FIRST_STRIKE)
				event_result &= ~KEYWORD_FIRST_STRIKE;
			  if (player_bits[player] & PB_CANT_HAVE_OR_GAIN_FLYING)
				event_result &= ~KEYWORD_FLYING;
			  if (player_bits[player] & PB_CANT_HAVE_OR_GAIN_DEATHTOUCH)
				instance->targets[16].card &= ~SP_KEYWORD_DEATHTOUCH;
			  if (player_bits[player] & PB_CANT_HAVE_OR_GAIN_TRAMPLE)
				event_result &= ~KEYWORD_TRAMPLE;
			  if (player_bits[player] & PB_CANT_HAVE_OR_GAIN_SHROUD)
				event_result &= ~KEYWORD_SHROUD;
			  if (player_bits[player] & PB_CANT_HAVE_OR_GAIN_HEXPROOF)
				instance->targets[16].card &= ~SP_KEYWORD_HEXPROOF;
			}
		}

	  if (event == EVENT_POWER)
		{
		  if (event_result < 0)
			event_result = 0;

		  if (instance->token_status & STATUS_BERSERK)
			event_result *= 2;
		}
	}

  int saved_event_result = event_result;
  if (event == EVENT_TOUGHNESS
	  && (instance->state & (STATE_OUBLIETTED|STATE_INVISIBLE|STATE_IN_PLAY)) == STATE_IN_PLAY
	  && (cards_data[iid].type & TYPE_CREATURE)
	  && (event_result <= 0
		  || instance->damage_on_card >= event_result)
	  && !(instance->token_status & STATUS_CANNOT_BE_DESTROYED)
	  && EXE_DWORD(0x785E60) == 0
	  && !(land_can_be_played & (LCBP_REGENERATION|LCBP_DAMAGE_PREVENTION)))
	{
	  if (event_result > 0)
		kill_card(player, card, KILL_DESTROY);
	  else
		kill_card(player, card, KILL_STATE_BASED_ACTION);

	  EXE_FN(int, 0x477a90, void)();	// regenerate_or_graveyard_triggers()
	}

  if (EXE_DWORD(0x60A548))
	pop_affected_card_stack();

  if (event != EVENT_CHANGE_TYPE)
	{
	  switch (event)
		{
		  case EVENT_POWER:
			instance->power = saved_event_result;
			break;

		  case EVENT_TOUGHNESS:
			instance->toughness = saved_event_result;
			break;

		  case EVENT_SET_COLOR:
			instance->color = saved_event_result & COLOR_TEST_ANY_COLORED;
			break;

		  case EVENT_ABILITIES:
			instance->regen_status = saved_event_result;

			/*
			 * EVENT_CHANGE_TYPE is the canonical location for this, but
			 * deathtouch is usually added during EVENT_ABILITIES, and sometimes
			 * can only be added there.
			 */
			if (instance->targets[16].card & SP_KEYWORD_DEATHTOUCH)
			  {
				int typ = cards_data[iid].type;
				if ((typ & TYPE_PERMANENT) && !(typ & TYPE_EFFECT))
				  instance->destroys_if_blocked |= DIFB_DESTROYS_UNPROTECTED;
			  }
			break;

		  default:
			break;
		}
	  return saved_event_result;
	}

  instance->internal_card_id = saved_event_result;
  if (saved_event_result != -1)
	instance->backup_internal_card_id = saved_event_result;
  else
	return saved_event_result;

  card_data_t* cd = &cards_data[saved_event_result];
  if (cd->extra_ability & EA_MANA_SOURCE)
	{
	  int csvid = cd->id;
	  int override = get_color_of_mana_produced_by_id(csvid, instance->info_slot, player);
	  if (override != -1)
		instance->mana_color = override;
	  else
		instance->mana_color = cards_ptr[csvid]->mana_source_colors;

	  instance->card_color = instance->mana_color;
	}

  // Makes Enchant Creature auras fall off previously animated cards that are no longer creatures.
  if (instance->token_status & STATUS_ANIMATED && !(cd->type & TYPE_CREATURE))
	{
	  int p, c;
	  for (p = 0; p <= 1; ++p)
		{
		  int active_count = bounded_active_cards_count(p);
		  for (c = 0; c < active_count; ++c)
			if (in_play(p, c))
			  {
				card_instance_t* inst = get_card_instance(p, c);
				if (inst->damage_target_player == player && inst->damage_target_card == card
					&& (cards_data[inst->internal_card_id].type & TYPE_ENCHANTMENT)
					&& cards_ptr[cards_data[inst->internal_card_id].id]->subtype1 == 44)
				  kill_card(p, c, KILL_STATE_BASED_ACTION);
			  }
		}
	}

  if (instance->state & STATE_IN_PLAY)
	event_flags |= cards_data[instance->internal_card_id].extra_ability & (EA_MARTYR|EA_SELECT_ATTACK|EA_SELECT_BLOCK|EA_LICH|EA_PAID_ATTACK|
																				 EA_PAID_BLOCK|EA_FORCE_ATTACK|EA_BEFORE_COMBAT|EA_DECLARE_ATTACK|
																				 EA_FELLWAR_STONE|EA_CONTROLLED);

  return saved_event_result;
}

// Fixes a bug in the exe. Do not call directly.
void event_activate_then_duplicate_into_stack(int player, int card, int event, int new_attacking_card_controller, int new_attacking_card)
{
  /*
   * Normally, activate() puts an activation card on the stack, dispatches
   * EVENT_ACTIVATE, then recopies the source card back onto the activation card.
   * The abbreviated mana-source path skips that recopy. If EVENT_ACTIVATE wrote
   * targets or info_slot, EVENT_RESOLVE_ACTIVATION would see stale data.
   *
   * This shim dispatches EVENT_ACTIVATE, then manually duplicates the relevant
   * part of recopy_card_onto_stack().
   */
  dispatch_event_with_attacker_to_one_card(player, card, event, new_attacking_card_controller, new_attacking_card);

  if (stack_size <= 0)
	return;

  int idx = stack_size - 1;
  if (!is_card_ref(stack_cards[idx].player, stack_cards[idx].card))
	return;

  card_instance_t* instance = get_card_instance(stack_cards[idx].player, stack_cards[idx].card);
  if (instance->internal_card_id == activation_card)
	{
	  if (!is_card_ref(instance->parent_controller, instance->parent_card))
		return;

	  int old_parent_card = instance->parent_card;
	  int old_parent_controller = instance->parent_controller;
	  uint32_t old_timestamp = instance->timestamp;
	  card_instance_t* parent_instance = get_card_instance(instance->parent_controller, instance->parent_card);

	  if (parent_instance->state & STATE_DONT_RECOPY_ONTO_STACK)
		parent_instance->state &= ~STATE_DONT_RECOPY_ONTO_STACK;
	  else
		{
		  STATIC_ASSERT(sizeof(card_instance_t) == 300, card_instance_t_size_differs_from_exe);
		  memcpy(instance, parent_instance, sizeof(card_instance_t));
		  instance->internal_card_id = activation_card;
		  instance->unknown0x14 = 0;
		  instance->kill_code = 0;
		  instance->state |= STATE_IN_PLAY;
		  instance->parent_controller = old_parent_controller;
		  instance->parent_card = old_parent_card;
		  instance->timestamp = old_timestamp;

		  if (parent_instance->internal_card_id != -1)
			instance->original_internal_card_id = parent_instance->internal_card_id;
		  else
			instance->original_internal_card_id = parent_instance->backup_internal_card_id;
		}

	  instance->token_status &= ~STATUS_DYING;	// so kill_card() will reap it, even if the original card sacrificed itself
	}
}

int current_trigger_or_event_is_forced(void)
{
  // 0x476F10

  /*
   * If a trigger or event is not listed here and the player has a stop on the
   * current phase, the engine prompts for "Done" even when there is only one
   * forced trigger handler or no meaningful choice.
   */
  uint8_t ctoe = EXE_DWORD(0x62bce8) & 0xff;	// TENTATIVE_current_trigger_or_event
  if (ctoe == EVENT_UNKNOWN8E)
	return must_attack ? 1 : 0;

  if (ctoe != EVENT_CAN_SKIP_TURN
	  && ctoe != 107	// 0x6B or -148; no known uses
	  && ctoe != EVENT_CAST_SPELL
	  && ctoe != EVENT_ACTIVATE
	  && ctoe != EVENT_DEAL_DAMAGE
	  && ctoe != 111	// 0x6F or -144; no known uses
	  && ctoe != EVENT_REGENERATE
	  && ctoe != EVENT_RESOLVE_SPELL
	  && ctoe != EVENT_RESOLVE_ACTIVATION
	  && ctoe != EVENT_CAN_ACTIVATE
	  && ctoe != EVENT_CAN_CAST
	  && ctoe != 117	// 0x75 or -138; no known uses
	  && ctoe != 118	// 0x76 or -137; no known uses
	  && ctoe != EVENT_GRAVEYARD_FROM_PLAY
	  && ctoe != EVENT_BLOCK_LEGALITY
	  && ctoe != EVENT_ATTACK_LEGALITY
	  && ctoe != 122	// 0x7A or -133; no known uses
	  && ctoe != 123	// 0x7B or -132; no known uses
	  && ctoe != 124	// 0x7C or -131; no known uses
	  && ctoe != EVENT_TRIGGER
	  && ctoe != EVENT_RESOLVE_TRIGGER
	  && ctoe != EVENT_COUNT_MANA
	  && ctoe != EVENT_UNKNOWN80
	  && ctoe != EVENT_TAP_CARD
	  && ctoe != EVENT_UNTAP
	  && ctoe != EVENT_UNTAP_CARD
	  && ctoe != EVENT_SET_UNTAP_COST
	  && ctoe != EVENT_SETUP_UPKEEP_COSTS
	  && ctoe != EVENT_UPKEEP_COSTS_UNPAID
	  && ctoe != EVENT_CHECK_UPK_PAYMENT
	  && ctoe != EVENT_CHECK_UNTAP_PAYMENT
	  && ctoe != EVENT_MUST_ATTACK
	  && ctoe != EVENT_UNKNOWN8E
	  && ctoe != EVENT_SHOULD_AI_PLAY
	  && ctoe != 200	// 0xC8 or -55; not searched
	  && ctoe != TRIGGER_UPKEEP
	  && ctoe != TRIGGER_DURING_UPKEEP
	  && ctoe != TRIGGER_END_UPKEEP
	  && ctoe != TRIGGER_END_COMBAT
	  && ctoe != TRIGGER_EOT
	  && ctoe != TRIGGER_DRAW_PHASE
	  && ctoe != TRIGGER_REPLACE_CARD_DRAW
	  && ctoe != TRIGGER_CARD_DRAWN
	  && ctoe != TRIGGER_DISCARD
	  && ctoe != TRIGGER_TAP_CARD
	  && ctoe != TRIGGER_SPELL_CAST
	  && ctoe != TRIGGER_LEAVE_PLAY
	  && ctoe != TRIGGER_GRAVEYARD_FROM_PLAY
	  && ctoe != TRIGGER_GRAVEYARD_ORDER
	  && ctoe != TRIGGER_DEAL_DAMAGE
	  && ctoe != TRIGGER_BOUNCE_PERMANENT
	  && ctoe != TRIGGER_MUST_ATTACK
	  && ctoe != TRIGGER_COMES_INTO_PLAY
	  && ctoe != TRIGGER_PAY_TO_ATTACK
	  && ctoe != TRIGGER_XTRIGGER)
	return 1;

  return 0;
}