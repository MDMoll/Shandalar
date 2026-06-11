#include "manalink.h"

/*
 * Aura helpers.
 *
 * Aura attachment uses two different pieces of card_instance_t state:
 *   damage_target_player/card: the permanent currently enchanted.
 *   targets[0]: the target currently being chosen or resolved.
 *
 * When an Aura moves, damage_target_* still points at the old permanent while
 * targets[0] points at the new one.  Keep those two roles separate; several
 * cleanup effects need to be removed from the old target before being applied
 * to the new target.
 */

enum {
  AURA_MAX_PLAYERS = 2,
  AURA_MAX_CARD_SLOTS = 150
};

static int is_real_player(int player)
{
  return player >= HUMAN && player <= AI;
}

static int is_card_slot(int card)
{
  return card >= 0 && card < AURA_MAX_CARD_SLOTS;
}

static int is_card_ref(int player, int card)
{
  return is_real_player(player) && is_card_slot(card);
}

static int bounded_active_cards_count(int player)
{
  return is_real_player(player) ? MIN(active_cards_count[player], AURA_MAX_CARD_SLOTS) : 0;
}

static int aura_has_attached_target(const card_instance_t* instance)
{
  return instance && is_card_ref(instance->damage_target_player, instance->damage_target_card);
}

static int aura_has_chosen_target(const card_instance_t* instance)
{
  return instance && is_card_ref(instance->targets[0].player, instance->targets[0].card);
}

static int aura_target_forbids_aura_spells(int player, int card)
{
  if (!is_card_ref(player, card) || is_humiliated(player, card)) {
    return 0;
  }

  switch (get_id(player, card)) {
    case CARD_ID_TETSUO_UMEZAWA:
    case CARD_ID_BARTEL_RUNEAXE:
      return 1;

    default:
      return 0;
  }
}

static void copy_attached_target_to_targets0(card_instance_t* instance)
{
  instance->targets[0].player = instance->damage_target_player;
  instance->targets[0].card = instance->damage_target_card;
  instance->number_of_targets = 1;
}

static void mark_attached_target_untargettable(card_instance_t* instance, int mode)
{
  if (aura_has_attached_target(instance)) {
    state_untargettable(instance->damage_target_player, instance->damage_target_card, mode);
  }
}

static void prepare_aura_move_target_definition(int player, int card, const card_instance_t* instance,
                                                const target_definition_t* source, target_definition_t* dest)
{
  *dest = *source;

  if (!check_special_flags(player, card, SF_MOVE_AURA_LEGAL_TARGET)) {
    dest->illegal_abilities = 0;
  }

  if (check_special_flags(player, card, SF_MOVE_AURA_SAME_CONTROLLER)) {
    dest->allowed_controller = instance->damage_target_player;
    dest->preferred_controller = instance->damage_target_player;
  }

  if (check_special_flags2(player, card, SF2_ENCHANTMENT_ALTERATION)
      && aura_has_attached_target(instance)) {
    dest->required_type = get_type(instance->damage_target_player, instance->damage_target_card);
  }
}

static int aura_can_move_to_another_target(int player, int card, card_instance_t* instance, target_definition_t* td)
{
  target_definition_t move_td;
  prepare_aura_move_target_definition(player, card, instance, td, &move_td);
  return target_available(player, card, &move_td) > 1 ? 1 : 0;
}

static int prompt_looks_literal(const char* prompt)
{
  return prompt && prompt[0] && prompt[1] >= 'a' && prompt[1] <= 'z';
}

static int pick_aura_move_target(int player, int card, card_instance_t* instance, target_definition_t* td,
                                 const char* prompt, int literal_prompt)
{
  target_definition_t move_td;
  prepare_aura_move_target_definition(player, card, instance, td, &move_td);

  mark_attached_target_untargettable(instance, 1);
  int result = new_pick_target(&move_td, prompt ? prompt : "TARGET_CREATURE", 0,
                               literal_prompt ? GS_LITERAL_PROMPT : 0);
  mark_attached_target_untargettable(instance, 0);

  return result;
}

static void apply_generic_aura_restrictions(int aura_player, int aura_card, int t_player, int t_card,
                                            int subtype, int flags)
{
  if (!is_card_ref(t_player, t_card)) {
    return;
  }

  if (flags & GA_FORBID_NONMANA_ACTIVATED_ABILITIES) {
    disable_nonmana_activated_abilities(t_player, t_card, 1);
  }
  if (flags & GA_FORBID_HUMILIATE) {
    humiliate(aura_player, aura_card, t_player, t_card, 1);
  }
  if (flags & GA_FORBID_ALL_ACTIVATED_ABILITIES) {
    disable_all_activated_abilities(t_player, t_card, 1);
  }

  if (subtype > 0) {
    if (flags & GA_FORCE_SUBTYPE) {
      force_a_subtype(t_player, t_card, subtype);
    }
    if (flags & GA_ADD_SUBTYPE) {
      add_a_subtype(t_player, t_card, subtype);
    }
  }
}

static void remove_generic_aura_restrictions(int aura_player, int aura_card, int t_player, int t_card,
                                             int subtype, int flags)
{
  if (!is_card_ref(t_player, t_card)) {
    return;
  }

  if (flags & GA_FORBID_NONMANA_ACTIVATED_ABILITIES) {
    disable_nonmana_activated_abilities(t_player, t_card, 0);
  }
  if (flags & GA_FORBID_HUMILIATE) {
    humiliate(aura_player, aura_card, t_player, t_card, 0);
  }
  if (flags & GA_FORBID_ALL_ACTIVATED_ABILITIES) {
    disable_all_activated_abilities(t_player, t_card, 0);
  }

  if (subtype > 0) {
    if (flags & GA_FORCE_SUBTYPE) {
      reset_subtypes(t_player, t_card, 1);
    }
    if (flags & GA_ADD_SUBTYPE) {
      reset_subtypes(t_player, t_card, 2);
    }
  }
}

static void apply_disabling_mode(int aura_player, int aura_card, int t_player, int t_card, int disabling_mode)
{
  if (!is_card_ref(t_player, t_card)) {
    return;
  }

  if (disabling_mode == 1) {
    disable_nonmana_activated_abilities(t_player, t_card, 1);
  }
  else if (disabling_mode == 2) {
    disable_all_activated_abilities(t_player, t_card, 1);
  }
  else if (disabling_mode == 3) {
    humiliate(aura_player, aura_card, t_player, t_card, 1);
  }
}

static void remove_disabling_mode(int aura_player, int aura_card, int t_player, int t_card, int disabling_mode)
{
  if (!is_card_ref(t_player, t_card)) {
    return;
  }

  if (disabling_mode == 1) {
    disable_nonmana_activated_abilities(t_player, t_card, 0);
  }
  else if (disabling_mode == 2) {
    disable_all_activated_abilities(t_player, t_card, 0);
  }
  else if (disabling_mode == 3) {
    humiliate(aura_player, aura_card, t_player, t_card, 0);
  }
}

static int resolve_aura_attachment(int player, int card, event_t event, target_definition_t* td)
{
  card_instance_t* instance = get_card_instance(player, card);
  int target_is_forced = check_special_flags(player, card, SF_TARGETS_ALREADY_SET) || aura_has_attached_target(instance);

  if (aura_has_attached_target(instance)) {
    copy_attached_target_to_targets0(instance);
  }

  if (!target_is_forced) {
    if (!valid_target(td)) {
      return 0;
    }

    if (aura_target_forbids_aura_spells(instance->targets[0].player, instance->targets[0].card)) {
      return 0;
    }
  }

  if (!aura_has_chosen_target(instance)) {
    return 0;
  }

  return attach_aura_to_target(player, card, event, instance->targets[0].player, instance->targets[0].card);
}

static void apply_card_specific_aura_targeting_rules(int player, int card, int csvid, target_definition_t* td)
{
  switch (csvid) {
    case CARD_ID_EMBLEM_OF_THE_WARMIND:
    case CARD_ID_DYING_WISH:
    case CARD_ID_MURDER_INVESTIGATION:
    case CARD_ID_SPLINTER_TWIN:
    case CARD_ID_FIRE_WHIP:
    case CARD_ID_HERMETIC_STUDY:
    case CARD_ID_QUICKSILVER_DAGGER:
    case CARD_ID_RIME_TRANSFUSION:
    case CARD_ID_FAITH_UNBROKEN:
      td->allowed_controller = player;
      break;

    default:
      break;
  }

  if (csvid == CARD_ID_ROOTS) {
    td->illegal_abilities |= KEYWORD_FLYING;
  }
  if (csvid == CARD_ID_WURMWEAVER_COIL) {
    td->required_color = COLOR_TEST_GREEN;
  }
  if (csvid == CARD_ID_GROUNDED && player == AI) {
    td->required_abilities = KEYWORD_FLYING;
  }

  if (check_special_flags3(player, card, SF3_IN_PLAY_DIRECTLY)) {
    td->illegal_abilities = 0;
  }
}

int attach_aura_to_target(int aura_player, int aura_card, event_t event, int t_player, int t_card)
{
  if (!is_card_ref(t_player, t_card)) {
    return 0;
  }

  if (check_for_guardian_beast_protection(t_player, t_card)) {
    return 0;
  }

  if (event == EVENT_RESOLVE_SPELL) {
    set_special_flags(aura_player, aura_card, SF_CORRECTLY_RESOLVED);
  }

  card_instance_t* instance = get_card_instance(aura_player, aura_card);
  instance->damage_target_player = t_player;
  instance->damage_target_card = t_card;

  return 1;
}

int aura_attached_to_me(int player, int card, event_t event, resolve_trigger_t trigger_mode, test_definition_t *test2)
{
  if (trigger_condition != TRIGGER_COMES_INTO_PLAY
      || !affect_me(player, card)
      || reason_for_trigger_controller != player
      || is_humiliated(player, card)) {
    return 0;
  }

  test_definition_t this_test;
  default_test_definition(&this_test, TYPE_ENCHANTMENT);
  this_test.subtype = SUBTYPE_AURA;

  if (!new_make_test_in_play(trigger_cause_controller, trigger_cause, -1, &this_test)) {
    return 0;
  }

  card_instance_t* aura = get_card_instance(trigger_cause_controller, trigger_cause);
  if (aura->damage_target_player != player || aura->damage_target_card != card) {
    return 0;
  }

  if (test2 && !new_make_test_in_play(trigger_cause_controller, trigger_cause, -1, test2)) {
    return 0;
  }

  if (event == EVENT_TRIGGER) {
    event_result |= trigger_mode;
  }
  else if (event == EVENT_RESOLVE_TRIGGER) {
    return 1;
  }

  return 0;
}

int generic_aura_impl(int player, int card, event_t event, target_definition_t *td, const char* prompt,
                      int p_plus, int t_plus, int k_plus, int k_special, int subtype, int clr, int flags)
{
  if (!td) {
    return 0;
  }

  card_instance_t* instance = get_card_instance(player, card);
  int csvid = instance->internal_card_id >= 0 ? cards_data[instance->internal_card_id].id : -1;

  if (prompt == NULL) {
    prompt = "TARGET_CREATURE";
  }

  apply_card_specific_aura_targeting_rules(player, card, csvid, td);

  if (event == EVENT_CAN_CAST) {
    return generic_spell(player, card, event, GS_CAN_TARGET | GS_AURA, td, NULL, 1, NULL);
  }

  if (event == EVENT_CAST_SPELL && affect_me(player, card)) {
    if (check_special_flags(player, card, SF_TARGETS_ALREADY_SET) || aura_has_attached_target(instance)) {
      if (aura_has_attached_target(instance)) {
        copy_attached_target_to_targets0(instance);
      }
    }
    else {
      generic_spell(player, card, event, GS_CAN_TARGET | GS_AURA, td, prompt, 1, NULL);
    }

    if (player == AI && !(trace_mode & 2)) {
      ai_modifier += (instance->targets[0].player == td->preferred_controller) ? 24 : -24;
    }
  }

  if (event == EVENT_RESOLVE_SPELL) {
    int good = resolve_aura_attachment(player, card, event, td);

    if (good) {
      remove_special_flags3(player, card, SF3_IN_PLAY_DIRECTLY);
      apply_generic_aura_restrictions(player, card, instance->targets[0].player, instance->targets[0].card, subtype, flags);
    }
    else {
      kill_card(player, card, KILL_BURY);
      cancel = 1;
    }

    instance->number_of_targets = 0;
  }

  if (in_play(player, card) && aura_has_attached_target(instance)) {
    int attached_player = instance->damage_target_player;
    int attached_card = instance->damage_target_card;

    if (event == EVENT_POWER && affect_me(attached_player, attached_card)) {
      event_result += p_plus;
    }

    if (event == EVENT_TOUGHNESS && affect_me(attached_player, attached_card)) {
      event_result += t_plus;
    }

    if (event == EVENT_ABILITIES && affect_me(attached_player, attached_card)) {
      event_result |= k_plus;
    }

    if (event == EVENT_SET_COLOR && affect_me(attached_player, attached_card)) {
      if (flags & GA_FORCE_COLOR) {
        event_result = clr;
      }
      else {
        event_result |= clr;
      }
    }

    if (k_special > 0) {
      special_abilities(attached_player, attached_card, event, k_special, player, card);
    }

    if (leaves_play(player, card, event)) {
      remove_generic_aura_restrictions(player, card, attached_player, attached_card, subtype, flags);
    }

    if (event == EVENT_CAN_MOVE_AURA) {
      return aura_can_move_to_another_target(player, card, instance, td);
    }

    if (event == EVENT_MOVE_AURA) {
      return pick_aura_move_target(player, card, instance, td, prompt, prompt_looks_literal(prompt));
    }

    if (event == EVENT_RESOLVE_MOVING_AURA) {
      target_definition_t move_td;
      prepare_aura_move_target_definition(player, card, instance, td, &move_td);

      int old_player = instance->damage_target_player;
      int old_card = instance->damage_target_card;
      int new_player = instance->targets[0].player;
      int new_card = instance->targets[0].card;

      if (valid_target(&move_td) && attach_aura_to_target(player, card, event, new_player, new_card)) {
        remove_generic_aura_restrictions(player, card, old_player, old_card, subtype, flags);
        apply_generic_aura_restrictions(player, card, new_player, new_card, subtype, flags);
      }
    }
  }

  return 0;
}

int generic_aura(int player, int card, event_t event, int pref_controller, int p_plus, int t_plus,
                 int k_plus, int k_special, int subtype, int clr, int flags)
{
  target_definition_t td;
  default_target_definition(player, card, &td, TYPE_CREATURE);
  td.preferred_controller = pref_controller;
  return generic_aura_impl(player, card, event, &td, NULL, p_plus, t_plus, k_plus, k_special, subtype, clr, flags);
}

/* An aura that does absolutely nothing itself. */
static int targeted_aura_impl(int player, int card, event_t event, target_definition_t* td,
                              const char* prompt, int custom_prompt, int disabling_mode)
{
  if (!td) {
    return 0;
  }

  card_instance_t* instance = get_card_instance(player, card);

  if (event == EVENT_CAN_CAST) {
    if (check_special_flags3(player, card, SF3_IN_PLAY_DIRECTLY)) {
      td->illegal_abilities = 0;
    }
    return generic_spell(player, card, event, GS_CAN_TARGET, td, NULL, 1, NULL);
  }

  if (event == EVENT_CAST_SPELL && affect_me(player, card)) {
    if (check_special_flags3(player, card, SF3_IN_PLAY_DIRECTLY)) {
      td->illegal_abilities = 0;
    }

    if (aura_has_attached_target(instance)) {
      copy_attached_target_to_targets0(instance);
    }
    else if (custom_prompt) {
      generic_spell(player, card, event, GS_CAN_TARGET | GS_AURA | GS_LITERAL_PROMPT, td, prompt, 1, NULL);
    }
    else {
      generic_spell(player, card, event, GS_CAN_TARGET | GS_AURA, td, prompt, 1, NULL);
    }

    if (player == AI && !(trace_mode & 2)) {
      ai_modifier += (instance->targets[0].player == td->preferred_controller) ? 24 : -24;
    }
  }

  if (event == EVENT_RESOLVE_SPELL) {
    int good = resolve_aura_attachment(player, card, event, td);

    if (good) {
      remove_special_flags3(player, card, SF3_IN_PLAY_DIRECTLY);
      apply_disabling_mode(player, card, instance->targets[0].player, instance->targets[0].card, disabling_mode);
    }
    else {
      kill_card(player, card, KILL_BURY);
      cancel = 1;
    }

    instance->number_of_targets = 0;
  }

  if (event == EVENT_GET_SELECTED_CARD && aura_has_attached_target(instance)) {
    *(int*)(0x78FA30) = instance->damage_target_card | (instance->damage_target_player << 8);
  }

  if (in_play(player, card) && (aura_has_attached_target(instance) || aura_has_chosen_target(instance))) {
    if ((leaves_play(player, card, event) && aura_has_attached_target(instance))
        || (instance->damage_target_player != instance->targets[2].player
            && instance->damage_target_card != instance->targets[2].card)) {
      instance->targets[2].player = instance->damage_target_player;
      instance->targets[2].card = instance->damage_target_card;
    }

    if (event == EVENT_CAN_MOVE_AURA) {
      return aura_can_move_to_another_target(player, card, instance, td);
    }

    if (event == EVENT_MOVE_AURA) {
      return pick_aura_move_target(player, card, instance, td, prompt ? prompt : "TARGET_CREATURE", custom_prompt);
    }

    if (event == EVENT_RESOLVE_MOVING_AURA) {
      target_definition_t move_td;
      prepare_aura_move_target_definition(player, card, instance, td, &move_td);

      int old_player = instance->damage_target_player;
      int old_card = instance->damage_target_card;
      int new_player = instance->targets[0].player;
      int new_card = instance->targets[0].card;

      if (valid_target(&move_td) && attach_aura_to_target(player, card, event, new_player, new_card)) {
        remove_disabling_mode(player, card, old_player, old_card, disabling_mode);
        apply_disabling_mode(player, card, new_player, new_card, disabling_mode);
      }
    }
  }

  return 0;
}

int targeted_aura(int player, int card, event_t event, target_definition_t* td, const char* prompt)
{
  return targeted_aura_impl(player, card, event, td, prompt, 0, 0);
}

int targeted_aura_custom_prompt(int player, int card, event_t event, target_definition_t* td, const char* prompt)
{
  return targeted_aura_impl(player, card, event, td, prompt, 1, 0);
}

int disabling_targeted_aura(int player, int card, event_t event, target_definition_t* td, const char* prompt, int mode)
{
  return targeted_aura_impl(player, card, event, td, prompt, 1, mode);
}

/* An aura with enchant creature that does absolutely nothing itself. */
int vanilla_aura(int player, int card, event_t event, int pref_controller)
{
  target_definition_t td;
  default_target_definition(player, card, &td, TYPE_CREATURE);
  td.preferred_controller = pref_controller;

  return targeted_aura(player, card, event, &td, "TARGET_CREATURE");
}

/* A frontend to vanilla_aura() with AI settings for Pacifism-like auras. */
int disabling_aura(int player, int card, int event)
{
  card_instance_t* instance = get_card_instance(player, card);

  int rval = vanilla_aura(player, card, event, 1-player);

  if (event == EVENT_CAST_SPELL && affect_me(player, card) && cancel != 1) {
    card_instance_t* tgt_inst = get_card_instance(instance->targets[0].player, instance->targets[0].card);

    if ((tgt_inst->regen_status & KEYWORD_DEFENDER) && !(tgt_inst->token_status & STATUS_WALL_CAN_ATTACK)) {
      ai_modifier -= 48;
    }

    if (instance->damage_target_player < 0 && instance->damage_target_card < 0
        && another_copy_attached(player, card, get_card_instance(player, card)->internal_card_id)) {
      ai_modifier -= 96;
    }
  }

  if (event == EVENT_SHOULD_AI_PLAY) {
    instance = get_card_instance(player, card);
    if (instance->damage_target_player == HUMAN) {
      ai_modifier += get_card_instance(instance->damage_target_player, instance->damage_target_card)->attack_rating / 2;
    }
  }

  return rval;
}

int aura_with_variable_pt_boost_depending_on_condition(int player, int card, event_t event, target_definition_t *td,
                                                       test_definition_t *this_test, int pow, int tou, int pow2, int tou2)
{
  card_instance_t* instance = get_card_instance(player, card);

  if (in_play(player, card) && aura_has_attached_target(instance)) {
    int p = instance->damage_target_player;
    int c = instance->damage_target_card;

    if (event == EVENT_POWER && affect_me(p, c)) {
      event_result += new_make_test_in_play(p, c, -1, this_test) ? pow : pow2;
    }

    if (event == EVENT_TOUGHNESS && affect_me(p, c)) {
      event_result += new_make_test_in_play(p, c, -1, this_test) ? tou : tou2;
    }
  }

  return targeted_aura(player, card, event, td, "TARGET_CREATURE");
}

int count_auras_enchanting_me(int player, int card)
{
  card_instance_t* aura;
  int result = 0;
  int i, count;

  for (i = 0; i < AURA_MAX_PLAYERS; ++i) {
    int active_count = bounded_active_cards_count(i);
    for (count = 0; count < active_count; ++count) {
      if ((aura = in_play(i, count))
          && is_what(i, count, TYPE_ENCHANTMENT)
          && has_subtype(i, count, SUBTYPE_AURA)
          && aura->damage_target_player == player
          && aura->damage_target_card == card) {
        ++result;
      }
    }
  }

  return result;
}

static int is_enchanted_impl(int player, int card, int subtype)
{
  card_instance_t* aura;
  int i, count;

  for (i = 0; i < AURA_MAX_PLAYERS; ++i) {
    int active_count = bounded_active_cards_count(i);
    for (count = 0; count < active_count; ++count) {
      if ((aura = in_play(i, count))
          && is_what(i, count, TYPE_ENCHANTMENT)
          && has_subtype(i, count, SUBTYPE_AURA)
          && aura->damage_target_player == player
          && aura->damage_target_card == card
          && (subtype <= 0 || has_subtype(i, count, subtype))) {
        return 1;
      }
    }
  }

  return 0;
}

int is_enchanted(int player, int card)
{
  return is_enchanted_impl(player, card, 0);
}

void put_into_play_aura_attached_to_target(int aura_player, int aura_card, int t_player, int t_card)
{
  if (!is_card_ref(t_player, t_card)) {
    return;
  }

  set_special_flags(aura_player, aura_card, SF_TARGETS_ALREADY_SET);

  card_instance_t* aura = get_card_instance(aura_player, aura_card);
  aura->targets[0].player = t_player;
  aura->targets[0].card = t_card;
  aura->number_of_targets = 1;
  aura->damage_target_player = t_player;
  aura->damage_target_card = t_card;

  put_into_play(aura_player, aura_card);
}

/* Uses targets[9] to store the player/card it was attached to at activation time. */
int aura_granting_activated_ability(int player, int card, event_t event, int preferred_controller, int mode,
                                    int cless, int black, int blue, int green, int red, int white,
                                    uint32_t variable_costs, target_definition_t *td, const char *prompt)
{
  int rval = attachment_granting_activated_ability(player, card, event, mode, cless, black, blue, green, red, white,
                                                   variable_costs, td, prompt);
  return rval ? rval : vanilla_aura(player, card, event, preferred_controller);
}

int gain_control_and_attach_as_aura(int player, int card, event_t event, int t_player, int t_card)
{
  if (!is_card_ref(t_player, t_card)) {
    return 0;
  }

  int flag = player != t_player ? 1 : 0;

  if (check_special_flags3(t_player, t_card, SF3_IS_STEALING_AURA)) {
    card_instance_t* instance = get_card_instance(t_player, t_card);
    flag = player != instance->damage_target_player ? 1 : 0;

    int result = gain_control_permanently(player, instance->damage_target_player, instance->damage_target_card);
    if (result > -1) {
      if (event == EVENT_RESOLVE_SPELL) {
        set_special_flags3(player, card, SF3_IS_STEALING_AURA);
      }
      if (flag) {
        add_state(player, card, STATE_POWER_STRUGGLE);
      }
      else if (check_state(player, card, STATE_POWER_STRUGGLE)) {
        remove_state(player, card, STATE_POWER_STRUGGLE);
      }

      result = gain_control_permanently_no_sound(player, t_player, t_card);
      return attach_aura_to_target(player, card, event, player, result);
    }
  }
  else {
    int result = gain_control_permanently(player, t_player, t_card);
    if (result > -1) {
      if (event == EVENT_RESOLVE_SPELL) {
        set_special_flags3(player, card, SF3_IS_STEALING_AURA);
      }
      if (flag) {
        add_state(player, card, STATE_POWER_STRUGGLE);
      }
      else if (check_state(player, card, STATE_POWER_STRUGGLE)) {
        remove_state(player, card, STATE_POWER_STRUGGLE);
      }

      return attach_aura_to_target(player, card, event, player, result);
    }
  }

  return 0;
}

static int is_enchanted_by_another_stealing_aura(int player, int card, int orig_aura_player, int orig_aura_card)
{
  card_instance_t* aura;
  int i, count;

  for (i = 0; i < AURA_MAX_PLAYERS; ++i) {
    int active_count = bounded_active_cards_count(i);
    for (count = 0; count < active_count; ++count) {
      if ((aura = in_play(i, count))
          && is_what(i, count, TYPE_ENCHANTMENT)
          && has_subtype(i, count, SUBTYPE_AURA)
          && !(i == orig_aura_player && count == orig_aura_card)
          && aura->damage_target_player == player
          && aura->damage_target_card == card
          && check_special_flags3(i, count, SF3_IS_STEALING_AURA)) {
        return 1;
      }
    }
  }

  return 0;
}

int generic_stealing_aura(int player, int card, event_t event, target_definition_t *td, const char *prompt)
{
  if (player < 0 || card < 0) {
    return 0;
  }

  if (event == EVENT_RESOLVE_SPELL) {
    card_instance_t* instance = get_card_instance(player, card);
    if (valid_target(td)) {
      gain_control_and_attach_as_aura(player, card, event, instance->targets[0].player, instance->targets[0].card);
    }
    else {
      kill_card(player, card, KILL_SACRIFICE);
    }
  }

  if (event == EVENT_CAN_MOVE_AURA || event == EVENT_MOVE_AURA) {
    return targeted_aura(player, card, event, td, prompt);
  }

  if (event == EVENT_RESOLVE_MOVING_AURA) {
    if (valid_target(td)) {
      card_instance_t* instance = get_card_instance(player, card);
      gain_control_and_attach_as_aura(player, card, event, instance->targets[0].player, instance->targets[0].card);
    }
  }

  if (leaves_play(player, card, event)) {
    card_instance_t* instance = get_card_instance(player, card);
    if (check_state(player, card, STATE_POWER_STRUGGLE)
        && in_play(instance->damage_target_player, instance->damage_target_card)) {
      if (!check_special_flags3(instance->damage_target_player, instance->damage_target_card, SF3_IS_STEALING_AURA)) {
        if (!is_enchanted_by_another_stealing_aura(instance->damage_target_player, instance->damage_target_card, player, card)) {
          re_enable_the_most_recent_control_effect_attached_to_me(instance->damage_target_player, instance->damage_target_card);
        }
        gain_control_permanently_no_sound(1-instance->damage_target_player,
                                          instance->damage_target_player,
                                          instance->damage_target_card);
      }
      else {
        card_instance_t* aura = get_card_instance(instance->damage_target_player, instance->damage_target_card);
        gain_control_permanently_no_sound(1-aura->damage_target_player, aura->damage_target_player, aura->damage_target_card);
        gain_control_permanently_no_sound(1-instance->damage_target_player,
                                          instance->damage_target_player,
                                          instance->damage_target_card);
      }
    }
  }

  return generic_spell(player, card, event, GS_CAN_TARGET | GS_AURA, td, prompt, 1, NULL);
}

static int generic_animating_aura_impl(int player, int card, event_t event, target_definition_t *td,
                                       const char *prompt, int pow, int tou, int key, int s_key, int custom_prompt)
{
  card_instance_t* instance = get_card_instance(player, card);

  if (aura_has_attached_target(instance) && !is_humiliated(player, card) && in_play(player, card)) {
    int p = instance->damage_target_player;
    int c = instance->damage_target_card;

    special_abilities(p, c, event, s_key, player, card);

    if (event == EVENT_ABILITIES && affect_me(p, c)) {
      event_result |= key;
    }

    /* If the Aura moved, clear the old animated permanent before animating the new one. */
    if (p != instance->targets[2].player || c != instance->targets[2].card) {
      if (instance->targets[2].player == -1 || instance->targets[2].card == -1) {
        instance->targets[2] = instance->targets[0];
      }
      else {
        if (in_play(instance->targets[2].player, instance->targets[2].card)) {
          remove_status(instance->targets[2].player, instance->targets[2].card, STATUS_ANIMATED);
        }
        instance->dummy3 = -1;
        instance->targets[2].player = p;
        instance->targets[2].card = c;
      }
    }

    if (!is_what(-1, get_original_internal_card_id(p, c), TYPE_CREATURE)) {
      if (event == EVENT_CHANGE_TYPE && affect_me(p, c)) {
        if (instance->dummy3 < 1) {
          int newtype = create_a_card_type(event_result);
          if (newtype != -1) {
            cards_at_7c7000[newtype]->type |= (cards_data[event_result].type | TYPE_CREATURE);
            cards_at_7c7000[newtype]->power = pow == -1 ? get_cmc(p, c) : pow;
            cards_at_7c7000[newtype]->toughness = tou == -1 ? get_cmc(p, c) : tou;
            instance->dummy3 = newtype;
            add_status(p, c, STATUS_ANIMATED);
          }
        }

        if (instance->dummy3 > 0) {
          event_result = instance->dummy3;
        }
      }
    }
    else {
      if (event == EVENT_POWER && affect_me(p, c)) {
        event_result += pow - get_base_power(p, c);
      }
      if (event == EVENT_TOUGHNESS && affect_me(p, c)) {
        event_result += tou - get_base_toughness(p, c);
      }
    }
  }

  if (custom_prompt) {
    return targeted_aura_custom_prompt(player, card, event, td, prompt);
  }
  return targeted_aura(player, card, event, td, prompt);
}

int generic_animating_aura(int player, int card, event_t event, target_definition_t *td,
                           const char *prompt, int pow, int tou, int key, int s_key)
{
  return generic_animating_aura_impl(player, card, event, td, prompt, pow, tou, key, s_key, 0);
}

int generic_animating_aura_custom_prompt(int player, int card, event_t event, target_definition_t *td,
                                         const char *prompt, int pow, int tou, int key, int s_key)
{
  return generic_animating_aura_impl(player, card, event, td, prompt, pow, tou, key, s_key, 1);
}

static int when_enchanted_permanent_dies_return_aura_to_hand_legacy(int player, int card, event_t event)
{
  card_instance_t* instance = get_card_instance(player, card);

  if (resolve_gfp_ability(player, card, event, RESOLVE_TRIGGER_MANDATORY)) {
    seek_grave_for_id_to_reanimate(player, card, instance->targets[0].player,
                                   cards_data[instance->targets[0].card].id, REANIMATE_RETURN_TO_HAND);
    kill_card(player, card, KILL_REMOVE);
  }

  return 0;
}

void when_enchanted_permanent_dies_return_aura_to_hand(int player, int card, event_t event)
{
  card_instance_t* instance = get_card_instance(player, card);

  if (!in_play(player, card) || !aura_has_attached_target(instance)) {
    return;
  }

  int t_player = instance->damage_target_player;
  int t_card = instance->damage_target_card;

  if (event != EVENT_GRAVEYARD_FROM_PLAY) {
    return;
  }

  if (affect_me(t_player, t_card) && in_play(t_player, t_card)) {
    card_instance_t* enchanted = get_card_instance(t_player, t_card);
    if (enchanted->kill_code > 0 && enchanted->kill_code < KILL_REMOVE) {
      set_special_flags3(player, card, SF3_ENCHANTED_PERMANENT_DYING);
    }
  }

  if (affect_me(player, card) && in_play(player, card)) {
    if (instance->kill_code > 0
        && instance->kill_code < KILL_REMOVE
        && check_special_flags3(player, card, SF3_ENCHANTED_PERMANENT_DYING)) {
      int legacy = create_legacy_effect(player, card, &when_enchanted_permanent_dies_return_aura_to_hand_legacy);
      card_instance_t* leg = get_card_instance(player, legacy);
      leg->targets[0].player = get_owner(player, card);
      leg->targets[0].card = get_original_internal_card_id(player, card);
      leg->targets[11].player = 66;
    }
  }
}

void cannot_be_enchanted_granted(int granting_player, int granting_card, event_t event, int t_player, int t_card)
{
  if (event != EVENT_STATIC_EFFECTS || !is_card_ref(t_player, t_card)) {
    return;
  }

  card_instance_t* aura;
  int i, count;
  for (i = 0; i < AURA_MAX_PLAYERS; ++i) {
    int active_count = bounded_active_cards_count(i);
    for (count = active_count - 1; count >= 0; --count) {
      if ((aura = in_play(i, count))
          && is_what(i, count, TYPE_ENCHANTMENT)
          && has_subtype(i, count, SUBTYPE_AURA)
          && aura->damage_target_player == t_player
          && aura->damage_target_card == t_card
          && (granting_player == -1 || !(i == granting_player && count == granting_card))) {
        kill_card(i, count, KILL_SACRIFICE);
      }
    }
  }
}

void cannot_be_enchanted(int player, int card, event_t event)
{
  cannot_be_enchanted_granted(-1, -1, event, player, card);
}
