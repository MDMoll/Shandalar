#include "manalink.h"

/*
 * Damage-card glue.
 *
 * Shandalar represents damage as invisible effect cards.  This file creates
 * those damage cards, lets prevention/replacement effects inspect them, and
 * finally applies the damage during EVENT_DEAL_DAMAGE.  The important contract
 * is that a damage card may outlive or outscope its original source, so code
 * here should prefer data copied onto the damage card over re-reading the
 * source unless the source is known to be a valid player/card pair.
 */

enum {
  DAMAGE_MAX_CARD_SLOTS = 150,
  DAMAGE_SOURCE_TARGETS_TO_COPY = 5,
  DAMAGE_TRACKED_TARGET_FIRST_SLOT = 2,
  DAMAGE_TRACKED_TARGET_LIMIT = 10
};

static int damage_player_is_valid(int player)
{
  return player >= HUMAN && player <= AI;
}

static int damage_card_slot_is_valid(int card)
{
  return card >= 0 && card < DAMAGE_MAX_CARD_SLOTS;
}

static int damage_card_ref_is_valid(int player, int card)
{
  return damage_player_is_valid(player) && damage_card_slot_is_valid(card);
}

static int bounded_damage_active_cards_count(int player)
{
  return damage_player_is_valid(player) ? MIN(active_cards_count[player], DAMAGE_MAX_CARD_SLOTS) : 0;
}

static card_instance_t* current_damage_card_instance(event_t event)
{
  if ((event != EVENT_DEAL_DAMAGE && event != EVENT_PREVENT_DAMAGE)
      || !damage_card_ref_is_valid(affected_card_controller, affected_card)) {
    return NULL;
  }

  card_instance_t* damage = get_card_instance(affected_card_controller, affected_card);
  return damage->internal_card_id == damage_card ? damage : NULL;
}

static int damage_source_ref_is_valid(const card_instance_t* damage)
{
  return damage && damage_card_ref_is_valid(damage->damage_source_player, damage->damage_source_card);
}

static int damage_target_ref_is_valid(const card_instance_t* damage)
{
  return damage && damage_card_ref_is_valid(damage->damage_target_player, damage->damage_target_card);
}

static int damage_is_combat(const card_instance_t* damage)
{
  return damage && (damage->token_status & (STATUS_COMBAT_DAMAGE | STATUS_FIRST_STRIKE_DAMAGE));
}

static int damage_amount_from_source_marker(card_instance_t* damage)
{
  if (!damage) {
    return 0;
  }

  if (damage->info_slot > 0) {
    return damage->info_slot;
  }

  if (!damage_source_ref_is_valid(damage)) {
    return 0;
  }

  card_instance_t* source = get_card_instance(damage->damage_source_player, damage->damage_source_card);
  return source->targets[16].player > 0 ? source->targets[16].player : 0;
}

static int damage_source_color(card_instance_t* damage)
{
  if (!damage) {
    return 0;
  }

  /* The copied color is safer than re-querying a source spell or legacy that
   * may have left the stack or battlefield by the time replacement/prevention
   * effects run. */
  if (!damage_source_ref_is_valid(damage) || !in_play(damage->damage_source_player, damage->damage_source_card)) {
    return damage->initial_color;
  }

  return get_color(damage->damage_source_player, damage->damage_source_card);
}

static int damage_source_has_subtype(card_instance_t* damage, subtype_t subtype)
{
  return damage_source_ref_is_valid(damage)
         && in_play(damage->damage_source_player, damage->damage_source_card)
         && has_subtype(damage->damage_source_player, damage->damage_source_card, subtype);
}

static void remember_damaged_target(card_instance_t* instance, int cursor_target, int first_slot, int max_slot, int t_player, int t_card)
{
  if (!instance) {
    return;
  }

  if (instance->targets[cursor_target].player < first_slot) {
    instance->targets[cursor_target].player = first_slot;
  }

  int pos = instance->targets[cursor_target].player;
  if (pos < max_slot) {
    instance->targets[pos].player = t_player;
    instance->targets[pos].card = t_card;
    ++instance->targets[cursor_target].player;
  }
}

void damage_effects(int player, int card, event_t event)
{
  /*
   * Legacy card-specific damage hooks.
   *
   * Most of this belongs in the individual card functions, or in
   * effect_damage() for global trap conditions.  Until that split happens,
   * keep this routine defensive: never inspect the damage source or target
   * unless the damage card says that reference is meaningful.
   */
  if (event != EVENT_DEAL_DAMAGE || !damage_card_ref_is_valid(player, card)) {
    return;
  }

  card_instance_t* instance = get_card_instance(player, card);
  card_instance_t* damage = current_damage_card_instance(event);
  if (!damage) {
    return;
  }

  enum {
    MODE_NONE = 0,
    MODE_FREEZE_WHEN_DAMAGE = 128,
  } mode = MODE_NONE;

  enum {
    GLOBALMODE_NONE = 0,
    GLOBALMODE_SHRIVELING_ROT = 64,
  } global_mode = GLOBALMODE_NONE;

  int csvid = instance->internal_card_id >= 0 ? get_id(player, card) : -1;

  if (check_for_special_ability(player, card, SP_KEYWORD_FREEZE_WHEN_DAMAGE)
      || csvid == CARD_ID_KUMANO_MASTER_YAMABUSHI) {
    mode |= MODE_FREEZE_WHEN_DAMAGE;
  }

  if (is_what(player, card, TYPE_EFFECT) && instance->targets[2].card == CARD_ID_SHRIVELING_ROT) {
    global_mode |= GLOBALMODE_SHRIVELING_ROT;
  }

  if (damage->damage_source_player == player && damage->damage_source_card == card && damage->info_slot > 0) {
    if (damage->damage_target_card != -1) {
      if ((mode & MODE_FREEZE_WHEN_DAMAGE)
          && damage_target_ref_is_valid(damage)
          && in_play(damage->damage_target_player, damage->damage_target_card)) {
        remember_damaged_target(instance, 1, DAMAGE_TRACKED_TARGET_FIRST_SLOT, DAMAGE_TRACKED_TARGET_LIMIT,
                                damage->damage_target_player, damage->damage_target_card);
      }
    }
    else if (csvid == CARD_ID_SZADEK_LORD_OF_SECRETS
             && damage_is_combat(damage)) {
      add_1_1_counters(player, card, damage->info_slot);
      mill(damage->damage_target_player, damage->info_slot);
      damage->info_slot = 0;
    }

    return;
  }

  if (csvid == CARD_ID_DRALNU_LICH_LORD
      && damage->damage_target_player == player
      && damage->damage_target_card == card) {
    int good = damage_amount_from_source_marker(damage);

    if (good > 0) {
      if (instance->targets[9].player < 0) {
        instance->targets[9].player = 0;
      }
      instance->targets[9].player += good;
    }
    damage->info_slot = 0;
  }

  if (csvid == CARD_ID_LIVING_ARTIFACT
      && damage->damage_target_player == player
      && damage->damage_target_card == -1
      && damage->info_slot > 0) {
    add_counters(player, card, COUNTER_VITALITY, damage->info_slot);
  }

  if (csvid == CARD_ID_WAR_ELEMENTAL
      && damage->damage_target_player == 1-player
      && damage->damage_target_card == -1
      && damage->info_slot > 0) {
    add_1_1_counters(player, card, damage->info_slot);
  }

  if (csvid == CARD_ID_NIGHT_DEALINGS
      && damage->damage_target_player == 1-player
      && damage->damage_source_player == player
      && damage->info_slot > 0) {
    add_counters(player, card, COUNTER_THEFT, damage->info_slot);
  }

  if (is_what(player, card, TYPE_EFFECT) && instance->targets[2].card == CARD_ID_DRAIN_LIFE
      && damage->damage_target_player == instance->targets[0].player
      && damage->damage_target_card == instance->targets[0].card) {
    int max = damage->info_slot;
    if (max > 0) {
      if (damage->damage_target_card == -1) {
        if (life[instance->targets[0].player] < max) {
          max = life[instance->targets[0].player];
        }
      }
      else if (damage_card_ref_is_valid(instance->targets[0].player, instance->targets[0].card)
               && in_play(instance->targets[0].player, instance->targets[0].card)) {
        int target_toughness = get_toughness(instance->targets[0].player, instance->targets[0].card);
        if (target_toughness < max) {
          max = target_toughness;
        }
      }
      else {
        max = 0;
      }

      if (max > 0) {
        gain_life(player, max);
      }
    }
    kill_card(player, card, KILL_REMOVE);
  }

  if (is_what(player, card, TYPE_EFFECT) && instance->targets[2].card == CARD_ID_BRIGHTFLAME
      && damage->display_pic_csv_id == CARD_ID_BRIGHTFLAME
      && damage->info_slot > 0) {
    gain_life(player, damage->info_slot);
  }

  if (is_what(player, card, TYPE_EFFECT) && instance->targets[1].card == CARD_ID_SIMIC_BASILISK
      && damage->damage_source_player == instance->targets[0].player
      && damage->damage_source_card == instance->targets[0].card
      && damage->info_slot > 0
      && damage->damage_target_card != -1) {
    remember_damaged_target(instance, 1, DAMAGE_TRACKED_TARGET_FIRST_SLOT, DAMAGE_TRACKED_TARGET_LIMIT,
                            damage->damage_target_player, damage->damage_target_card);
  }

  if ((csvid == CARD_ID_SPROUTING_PHYTOHYDRA || csvid == CARD_ID_VOLATILE_RIG || csvid == CARD_ID_DEEP_SLUMBER_TITAN)
      && damage->damage_target_player == player
      && damage->damage_target_card == card
      && damage->info_slot > 0) {
    if (instance->targets[1].player < 0) {
      instance->targets[1].player = 0;
    }
    ++instance->targets[1].player;
  }

  if (csvid == CARD_ID_CIRCLE_OF_AFFLICTION
      && damage->damage_target_player == player
      && damage->damage_target_card == -1
      && damage->info_slot > 0
      && (damage_source_color(damage) & instance->info_slot)) {
    if (instance->targets[1].player < 0) {
      instance->targets[1].player = 0;
    }
    ++instance->targets[1].player;
  }

  if (csvid == CARD_ID_GREATBOW_DOYEN
      && damage->damage_source_player == player
      && damage->damage_target_card != -1
      && damage_target_ref_is_valid(damage)
      && in_play(damage->damage_target_player, damage->damage_target_card)
      && damage_source_has_subtype(damage, SUBTYPE_ARCHER)) {
    remember_damaged_target(instance, 1, DAMAGE_TRACKED_TARGET_FIRST_SLOT, DAMAGE_TRACKED_TARGET_LIMIT,
                            damage->damage_target_player, damage->info_slot);
  }

  if (csvid == CARD_ID_OONAS_BLACKGUARD
      && damage->damage_source_player == player
      && damage->damage_target_card == -1
      && damage_source_ref_is_valid(damage)
      && in_play(damage->damage_source_player, damage->damage_source_card)
      && count_1_1_counters(damage->damage_source_player, damage->damage_source_card) > 0
      && damage_is_combat(damage)) {
    if (instance->targets[1].player < 0) {
      instance->targets[1].player = 0;
    }
    ++instance->targets[1].player;
  }

  if (csvid == CARD_ID_DEUS_OF_CALAMITY
      && damage->damage_source_player == player
      && damage->damage_source_card == card
      && damage->damage_target_card == -1
      && damage->info_slot > 5) {
    instance->targets[1].player = 2 + damage->damage_target_player;
  }

  if (is_what(player, card, TYPE_EFFECT)
      && (instance->targets[2].card == CARD_ID_RUNESWORD || instance->targets[2].card == CARD_ID_BONE_SHAMAN)
      && damage->damage_source_player == instance->damage_source_player
      && damage->damage_source_card == instance->damage_source_card
      && damage->damage_target_card >= 0
      && damage_target_ref_is_valid(damage)
      && in_play(damage->damage_target_player, damage->damage_target_card)) {
    remember_damaged_target(instance, 2, 3, DAMAGE_TRACKED_TARGET_LIMIT,
                            damage->damage_target_player, damage->damage_target_card);
  }

  if ((global_mode & GLOBALMODE_SHRIVELING_ROT)
      && damage->damage_target_card != -1
      && damage_target_ref_is_valid(damage)
      && in_play(damage->damage_target_player, damage->damage_target_card)) {
    kill_card(damage->damage_target_player, damage->damage_target_card, KILL_DESTROY);
  }
}

int damage_creature(int tgt_player, int tgt_card, int32_t amt, int src_player, int src_card)
{
  // 0x4a66c0
  // This is the only place where damage cards are created.

  if (!damage_player_is_valid(tgt_player)
      || !damage_player_is_valid(src_player)
      || (tgt_card != -1 && !damage_card_slot_is_valid(tgt_card))
      || (src_card != -1 && !damage_card_slot_is_valid(src_card))
      || amt <= 0) {
    return -1;
  }

  int whose_bf = tgt_card == -1 ? tgt_player : src_player;

  int dmg_card = add_card_to_hand(whose_bf, damage_card);
  if (dmg_card == -1) {
    return -1;
  }

  land_can_be_played |= LCBP_PENDING_DAMAGE_CARDS;

  if (tgt_card != -1) {
    card_instance_t* tgt_inst = get_card_instance(tgt_player, tgt_card);
    if ((tgt_inst->token_status & STATUS_SPECIAL_BLOCKER)
        && cards_data[tgt_inst->internal_card_id].code_pointer == 0x401010) { // card_multiblocker()
      tgt_player = tgt_inst->damage_source_player;
      tgt_card = tgt_inst->damage_source_card;
      if (!damage_card_ref_is_valid(tgt_player, tgt_card)) {
        return -1;
      }
    }
  }

  card_instance_t* dmg_inst = get_card_instance(whose_bf, dmg_card);
  dmg_inst->state |= STATE_IN_PLAY;
  --hand_count[whose_bf];
  dmg_inst->damage_target_player = tgt_player;
  dmg_inst->damage_target_card = tgt_card;
  dmg_inst->info_slot = amt;
  dmg_inst->damage_source_player = src_player;
  dmg_inst->damage_source_card = src_card;

  if (src_card == -1) {
    dmg_inst->display_pic_csv_id = CARD_ID_SWAMP;
    dmg_inst->display_pic_num = 0;
    return dmg_card;
  }

  card_instance_t* src_inst = get_card_instance(src_player, src_card);
  card_instance_t* src_src_inst = src_inst;

  int src_iid;
  int src_typ;
  int attacking;

  if (src_inst->internal_card_id == -1) {
    src_iid = src_inst->backup_internal_card_id;
    if (src_iid < 0) {
      return dmg_card;
    }

    src_typ = get_type_with_iid(src_player, src_card, src_iid);
    attacking = src_inst->state & STATE_ATTACKING;

    int csvid = cards_data[src_iid].id;
    dmg_inst->display_pic_csv_id = csvid;
    dmg_inst->display_pic_num = get_card_image_number(csvid, src_player, src_card);
  }
  else if (src_inst->internal_card_id == activation_card) {
    src_iid = src_inst->original_internal_card_id;
    if (src_iid < 0) {
      src_iid = src_inst->backup_internal_card_id;
    }
    if (src_iid < 0) {
      return dmg_card;
    }

    if (damage_card_ref_is_valid(src_inst->parent_controller, src_inst->parent_card)) {
      src_src_inst = get_card_instance(src_inst->parent_controller, src_inst->parent_card);
      src_typ = get_type_with_iid(src_inst->parent_controller, src_inst->parent_card, src_iid);
    }
    else {
      src_src_inst = src_inst;
      src_typ = cards_data[src_iid].type;
    }

    attacking = 0;

    dmg_inst->display_pic_csv_id = src_inst->display_pic_csv_id;
    dmg_inst->display_pic_num = src_inst->display_pic_num;

    dmg_inst->damage_source_player = src_inst->parent_controller;
    dmg_inst->damage_source_card = src_inst->parent_card;
  }
  else if (cards_data[src_inst->internal_card_id].id == 902
           || cards_data[src_inst->internal_card_id].id == 903
           || cards_data[src_inst->internal_card_id].id == 907) {
    /* Legacy/effect damage.  The copied damage-card data is authoritative if
     * the original source is gone or no longer has a meaningful slot. */
    src_iid = src_inst->original_internal_card_id;
    if (src_iid < 0) {
      src_iid = src_inst->backup_internal_card_id;
    }
    if (src_iid < 0) {
      return dmg_card;
    }

    if (damage_card_ref_is_valid(src_inst->damage_source_player, src_inst->damage_source_card)) {
      src_src_inst = get_card_instance(src_inst->damage_source_player, src_inst->damage_source_card);
      src_typ = get_type_with_iid(src_inst->damage_source_player, src_inst->damage_source_card, src_iid);
    }
    else {
      src_src_inst = src_inst;
      src_typ = cards_data[src_iid].type;
    }

    attacking = 0;

    dmg_inst->display_pic_csv_id = src_inst->display_pic_csv_id;
    dmg_inst->display_pic_num = src_inst->display_pic_num;
  }
  else {
    src_iid = src_inst->internal_card_id;
    src_src_inst = src_inst;

    src_typ = get_type_with_iid(src_player, src_card, src_iid);
    attacking = src_inst->state & STATE_ATTACKING;

    int csvid = cards_data[src_iid].id;
    dmg_inst->display_pic_csv_id = csvid;
    dmg_inst->display_pic_num = get_card_image_number(csvid, src_player, src_card);
  }

  int src_colorbits = damage_card_ref_is_valid(src_player, src_card) ? get_color(src_player, src_card) : 0;
  if (src_typ & TYPE_ARTIFACT) {
    src_colorbits |= COLOR_TEST_ARTIFACT;
  }

  dmg_inst->initial_color = dmg_inst->color = src_colorbits;

  dmg_inst->eot_toughness = dmg_card;

  if (attacking) {
    if (current_phase == PHASE_FIRST_STRIKE_DAMAGE) {
      dmg_inst->token_status |= STATUS_FIRST_STRIKE_DAMAGE;
    }

    if (current_phase == PHASE_NORMAL_COMBAT_DAMAGE) {
      dmg_inst->token_status |= STATUS_COMBAT_DAMAGE;
    }
  }

  dmg_inst->targets[3].player = src_typ;
  dmg_inst->targets[3].card = 0;

  if (((!(src_typ & TYPE_PERMANENT) || is_humiliated_by_instance(src_src_inst))
       && cards_data[src_iid].id != CARD_ID_PUNCTURE_BLAST)) {
    int sgm_flag = 0;
    int i, active_count = bounded_damage_active_cards_count(src_player);
    for (i = 0; i < active_count; ++i) {
      if (in_play(src_player, i) && get_id(src_player, i) == CARD_ID_SOULFIRE_GRAND_MASTER && !is_humiliated(src_player, i)) {
        sgm_flag = 1;
        break;
      }
    }
    if (!sgm_flag) {
      dmg_inst->regen_status = 0;
      dmg_inst->targets[16].card = 0;
    }
  }
  else {
    dmg_inst->regen_status = src_src_inst->regen_status & ~KEYWORD_NONABILITIES;
    if (src_src_inst->targets[16].card == -1) {
      dmg_inst->targets[16].card = 0;
    }
    else {
      dmg_inst->targets[16].card = src_src_inst->targets[16].card;
    }
  }

  int copied_targets = MIN(src_inst->number_of_targets, DAMAGE_SOURCE_TARGETS_TO_COPY);
  int k;
  for (k = 0; k < copied_targets; ++k) {
    dmg_inst->targets[k + 5] = src_inst->targets[k];
  }

  return dmg_card;
}

// Convenient frontend to damage_creature().  Deals dmg damage to targets[0].player/card.  Promotes to parent if an activation card.  Does not validate.
int damage_target0(int player, int card, int dmg)
{
  if (!damage_card_ref_is_valid(player, card)) {
    return -1;
  }

  card_instance_t* instance = get_card_instance(player, card);
  if (instance->internal_card_id == activation_card) {
    player = instance->parent_controller;
    card = instance->parent_card;
  }
  return damage_creature(instance->targets[0].player, instance->targets[0].card, dmg, player, card);
}

static int remove_indestructible_until_eot(int player, int card, event_t event)
{
  card_instance_t* instance = get_card_instance(player, card);
  if (instance->damage_target_player > -1) {
    int p = instance->damage_target_player;
    int c = instance->damage_target_card;

    if (event == EVENT_ABILITIES && affect_me(p, c)) {
      remove_special_ability(p, c, SP_KEYWORD_INDESTRUCTIBLE);
    }
  }

  if (event == EVENT_CLEANUP) {
    kill_card(player, card, KILL_REMOVE);
  }

  return 0;
}

int effect_damage(int player, int card, event_t event)
{
  // 0x4a0f00

  /* Local data:
   * damage_source_player/damage_source_card: source of damage
   * damage_target_player/damage_target_card: object being damaged.  (This about the *only* card or effect where those four names are correct.)
   * info_slot: amount of damage to be dealt
   * initial_color: color of damage source at the time it dealt damage.  Querying the source is usually slightly more accurate, occasionally much less.
   * state & STATE_TAPPED: this damage card has applied its damage.
   * display_pic_csv_id: csvid of damage source at the time it dealt damage.
   * regen_status: keywords of damage source at the time it dealt damage.  This effect is special-cased in get_abilities to never recalculate.
   * targets[3].player: types of damage source at the time it dealt damage.
   * targets[3].card: damage flags for the target.
   * targets[4].player/card: Planeswalker to deal damage to instead of player.
   * targets[5]-targets[9]: copy of the damage source's first five targets. Needed for Bronze Horse.
   * targets[16].card: special keywords of damage source at the time it dealt damage.
   */

  card_instance_t* instance;
  int abils;

  if ((land_can_be_played & LCBP_DAMAGE_PREVENTION)
      && affect_me(player, card)
      && (instance = get_card_instance(player, card))
      && instance->damage_target_card != -1
      && damage_card_ref_is_valid(instance->damage_target_player, instance->damage_target_card)
      && ((abils = get_abilities(instance->damage_target_player, instance->damage_target_card, EVENT_ABILITIES, -1))
          & (KEYWORD_PROT_SORCERIES | KEYWORD_PROT_INTERRUPTS | KEYWORD_PROT_INSTANTS | KEYWORD_SHROUD | KEYWORD_PROT_ARTIFACTS | KEYWORD_PROT_COLORED))) {
    int source_clr = instance->initial_color & COLOR_TEST_ANY_COLORED;
    if (damage_card_ref_is_valid(instance->damage_source_player, instance->damage_source_card)
        && in_play(instance->damage_source_player, instance->damage_source_card)) {
      source_clr = get_color(instance->damage_source_player, instance->damage_source_card) & COLOR_TEST_ANY_COLORED;
    }

    if ((source_clr & (abils >> 10))
        || ((abils & KEYWORD_PROT_ARTIFACTS)
            && (instance->targets[3].player & TYPE_ARTIFACT))) {
      instance->info_slot = 0;
      kill_card(player, card, KILL_BURY);
    }
  }

  if (event == EVENT_DEAL_DAMAGE
      && affect_me(player, card)
      && (instance = get_card_instance(player, card))
      && !(instance->state & STATE_TAPPED)) {
    instance->state |= STATE_TAPPED;
    instance->token_status |= STATUS_INVISIBLE_FX;

    if (instance->info_slot <= 0) {
      return 0;
    }

    card_instance_t* damaged;
    if (instance->targets[4].player < 0 || instance->targets[4].card < 0) {
      instance->targets[4].player = instance->damage_target_player;
      instance->targets[4].card = instance->damage_target_card;
      if (instance->targets[4].card == -1) {
        damaged = NULL;
      }
      else if (!damage_card_ref_is_valid(instance->targets[4].player, instance->targets[4].card)
               || !(damaged = in_play(instance->targets[4].player, instance->targets[4].card))) {
        return 0;
      }
    }
    else {
      if (!damage_card_ref_is_valid(instance->targets[4].player, instance->targets[4].card)
          || !(damaged = in_play(instance->targets[4].player, instance->targets[4].card))
          || !is_planeswalker(instance->targets[4].player, instance->targets[4].card)) {
        return 0;
      }
    }

    if (instance->info_slot > get_trap_condition(player, TRAP_MAX_DAMAGE_DEALT)
        || instance->info_slot > get_trap_condition(1-player, TRAP_MAX_DAMAGE_DEALT)) {
      set_trap_condition(player, TRAP_MAX_DAMAGE_DEALT, instance->info_slot);
      set_trap_condition(1-player, TRAP_MAX_DAMAGE_DEALT, instance->info_slot);
    }

    if (instance->targets[4].card == -1) {
      play_sound_effect(WAV_LIFELOSS);

      int damaged_player = instance->targets[4].player;
      int src_p = instance->damage_source_player;
      int src_c = instance->damage_source_card;
      int damage_dealt = instance->info_slot;

      increase_trap_condition(damaged_player, TRAP_DAMAGE_TAKEN, damage_dealt);

      if (instance->targets[3].player & TYPE_ARTIFACT) {
        increase_trap_condition(damaged_player, TRAP_ARTIFACT_DAMAGE_TAKEN, damage_dealt);
      }

      if ((instance->targets[3].player & TYPE_PERMANENT)
          && damage_card_ref_is_valid(src_p, src_c)
          && in_play(src_p, src_c)) {
        if (damaged_player == 0) {
          set_special_flags2(src_p, src_c, SF2_HAS_DAMAGED_PLAYER0);
        }
        if (damaged_player == 1) {
          set_special_flags2(src_p, src_c, SF2_HAS_DAMAGED_PLAYER1);
        }
        if ((instance->targets[3].player & TYPE_CREATURE) && damage_is_combat(instance)) {
          increase_trap_condition(damaged_player, TRAP_DAMAGED_BY_CREATURE, 1);

          if (in_play(src_p, src_c) && has_subtype(src_p, src_c, SUBTYPE_ROGUE)) {
            increase_trap_condition(src_p, TRAP_PROWL_ROGUE, 1);
          }

          if (in_play(src_p, src_c) && has_subtype(src_p, src_c, SUBTYPE_GOBLIN)) {
            increase_trap_condition(src_p, TRAP_PROWL_GOBLIN, 1);
          }

          if (in_play(src_p, src_c) && has_subtype(src_p, src_c, SUBTYPE_FAERIE)) {
            increase_trap_condition(src_p, TRAP_PROWL_FAERIE, 1);
          }
        }
      }

      if (damage_card_ref_is_valid(src_p, src_c)
          && damaged_player != src_p
          && (instance->initial_color & COLOR_TEST_RED)
          && (instance->targets[3].player & (TYPE_SPELL | TARGET_TYPE_PLANESWALKER))) {
        increase_trap_condition(src_p, TRAP_CHANDRAS_PHOENIX, 1);
      }

      if (instance->targets[16].card & SP_KEYWORD_LIFELINK) {
        gain_life(instance->damage_source_player, damage_dealt);
      }

      EXE_DWORD_PTR(0x4EF1EC)[damaged_player] += damage_dealt; // damage_taken_this_turn[]

      if (instance->regen_status & KEYWORD_INFECT) {
        poison(damaged_player, damage_dealt);
      }
      else {
        int new_life = life[damaged_player] - damage_dealt;

        if (new_life < 7) {
          new_life = dispatch_event_with_initial_event_result(player, card, EVENT_DAMAGE_REDUCTION, new_life);
        }

        if (new_life < life[damaged_player]) {
          lose_life(damaged_player, life[damaged_player] - new_life);
        }
        else if (new_life > life[damaged_player]) {
          gain_life(damaged_player, new_life - life[damaged_player]);
        }
      }

      if (instance->display_pic_csv_id == CARD_ID_CHANDRA_ROARING_FLAME
          && instance->targets[10].card == CARD_ID_CHANDRA_ROARING_FLAME) {
        token_generation_t token;
        default_token_definition(player, card, CARD_ID_CHANDRA_ROARING_FLAME_EMBLEM, &token);
        token.t_player = damaged_player;
        generate_token(&token);
      }

      if (instance->display_pic_csv_id == CARD_ID_AURELIAS_FURY
          && instance->targets[10].card == CARD_ID_AURELIAS_FURY) {
        int card_added = generate_reserved_token_by_id(player, CARD_ID_SPECIAL_EFFECT);
        card_instance_t* legacy = get_card_instance(player, card_added);
        legacy->targets[2].player = 128;
        legacy->targets[3].player = damaged_player;
        legacy->targets[2].card = CARD_ID_AURELIAS_FURY;
        create_card_name_legacy(player, card_added, CARD_ID_AURELIAS_FURY);
      }
    }
    else {
      if (instance->targets[16].card & SP_KEYWORD_LIFELINK) {
        gain_life(instance->damage_source_player, instance->info_slot);
      }

      if (!is_what(instance->targets[4].player, instance->targets[4].card, TYPE_CREATURE)) {
        if (is_planeswalker(instance->targets[4].player, instance->targets[4].card)) {
          remove_counters(instance->targets[4].player, instance->targets[4].card, COUNTER_LOYALTY, instance->info_slot);
        }
      }
      else {
        if (instance->targets[3].card & DMG_MUST_ATTACK_IF_DEALT_DAMAGE_THIS_WAY) {
          pump_ability_until_eot(instance->damage_source_player, instance->damage_source_card,
                                 instance->targets[4].player, instance->targets[4].card, 0, 0, 0, SP_KEYWORD_MUST_ATTACK);
        }

        if (instance->targets[3].card & DMG_TAP_IF_DEALT_DAMAGE_THIS_WAY) {
          tap_card(instance->targets[4].player, instance->targets[4].card);
        }

        if (instance->targets[3].card & DMG_EXILE_IF_LETHALLY_DAMAGED_THIS_WAY) {
          set_special_flags(instance->targets[4].player, instance->targets[4].card, SF_LETHAL_DAMAGE_EXILE);
          exile_if_would_be_put_into_graveyard(instance->damage_source_player, instance->damage_source_card,
                                               instance->targets[4].player, instance->targets[4].card, 1);
        }

        if (instance->targets[3].card & DMG_CANT_REGENERATE_IF_DEALT_DAMAGE_THIS_WAY) {
          cannot_regenerate_until_eot(instance->damage_source_player, instance->damage_source_card,
                                      instance->targets[4].player, instance->targets[4].card);
        }

        if (instance->targets[16].card & SP_KEYWORD_DEATHTOUCH) {
          if (instance->display_pic_csv_id == CARD_ID_PHAGE_THE_UNTOUCHABLE
              && damage_is_combat(instance)) {
            set_special_flags(instance->targets[4].player, instance->targets[4].card, SF_LETHAL_DAMAGE_BURY);
          }
          else {
            set_special_flags(instance->targets[4].player, instance->targets[4].card, SF_LETHAL_DAMAGE_DESTROY);
          }
        }

        if (instance->targets[16].card & SP_KEYWORD_RFG_WHEN_DAMAGE) {
          set_special_flags(instance->targets[4].player, instance->targets[4].card, SF_EXILE_IF_DAMAGED);
        }

        if (instance->targets[10].card == CARD_ID_PATHWAY_ARROWS
            && get_color(instance->targets[4].player, instance->targets[4].card) == 0) {
          tap_card(instance->targets[4].player, instance->targets[4].card);
        }

        if (instance->targets[10].card == CARD_ID_BURN_FROM_WITHIN
            && damage_source_ref_is_valid(instance)) {
          create_targetted_legacy_effect(instance->damage_source_player, instance->damage_source_card, &remove_indestructible_until_eot,
                                         instance->targets[4].player, instance->targets[4].card);
        }

        if (get_id(instance->targets[4].player, instance->targets[4].card) == CARD_ID_ILLUSORY_AMBUSHER) {
          draw_cards(instance->targets[4].player, instance->info_slot);
        }

        if ((instance->targets[16].card & SP_KEYWORD_WITHER)
            || (instance->regen_status & KEYWORD_INFECT)) {
          ++hack_silent_counters;
          add_minus1_minus1_counters(instance->targets[4].player, instance->targets[4].card, instance->info_slot);
          --hack_silent_counters;
        }
        else {
          damaged->damage_on_card += instance->info_slot;
        }
      }

      if (ai_is_speculating != 1) {
        play_sound_effect(WAV_DAMAGE);
        EXE_FN(void, 0x437e20, void)();
      }
    }
  }
  return 0;
}

void after_damage(void)
{
  // 0x477340

  int player, card;
  for (player = 0; player <= 1; ++player) {
    int active_count = bounded_damage_active_cards_count(player);
    for (card = 0; card < active_count; ++card) {
      card_instance_t* creature = in_play(player, card);
      if (creature && is_what(player, card, TYPE_CREATURE)) {
        int kill_mode = 0;
        int flags = check_special_flags(player, card, SF_LETHAL_DAMAGE_DESTROY | SF_LETHAL_DAMAGE_BURY | SF_LETHAL_DAMAGE_EXILE | SF_EXILE_IF_DAMAGED);
        if (flags) {
          remove_special_flags(player, card, SF_LETHAL_DAMAGE_DESTROY | SF_LETHAL_DAMAGE_BURY | SF_LETHAL_DAMAGE_EXILE | SF_EXILE_IF_DAMAGED);
        }

        if ((flags & SF_EXILE_IF_DAMAGED)
            || ((flags & SF_LETHAL_DAMAGE_EXILE) && (int)creature->damage_on_card >= get_toughness(player, card))) {
          kill_mode = KILL_REMOVE;
        }
        else if (flags & SF_LETHAL_DAMAGE_BURY) {
          kill_mode = KILL_BURY;
        }
        else if ((flags & SF_LETHAL_DAMAGE_DESTROY)
                 || (int)creature->damage_on_card >= get_toughness(player, card)) {
          kill_mode = KILL_DESTROY;
        }

        if (kill_mode) {
          kill_card(player, card, kill_mode);
        }
        else if (check_special_flags3(player, card, SF3_ARCHANGEL_OF_THUNE_COUNTER)) {
          int amount = creature->eot_toughness;
          if (amount) {
            creature->eot_toughness = 0;
            add_1_1_counters(player, card, amount);
          }
        }
      }
    }
  }

  for (player = 0; player <= 1; ++player) {
    int active_count = bounded_damage_active_cards_count(player);
    for (card = 0; card < active_count; ++card) {
      if (in_play(player, card)) {
        dispatch_event_with_attacker_to_one_card(player, card, EVENT_AFTER_DAMAGE, 1-player, -1);
      }
    }
  }
}

// Returns a damage card during EVENT_PREVENT_DAMAGE if it's combat damage and doing more than 0.
card_instance_t* combat_damage_being_prevented(event_t event)
{
  card_instance_t* damage = current_damage_card_instance(event);
  return damage && damage_is_combat(damage) && damage->info_slot > 0 ? damage : NULL;
}

// Returns a damage card during EVENT_PREVENT_DAMAGE if it's noncombat damage and doing more than 0.
card_instance_t* noncombat_damage_being_prevented(event_t event)
{
  card_instance_t* damage = current_damage_card_instance(event);
  return damage && !damage_is_combat(damage) && damage->info_slot > 0 ? damage : NULL;
}

// Returns a damage card during EVENT_PREVENT_DAMAGE if it's doing more than 0.
card_instance_t* damage_being_prevented(event_t event)
{
  card_instance_t* damage = current_damage_card_instance(event);
  return damage && damage->info_slot > 0 ? damage : NULL;
}

/* Returns a damage card during EVENT_DEAL_DAMAGE if it's combat damage and doing more than 0.  damage->targets[16].player will hold the damage dealt if it's
 * determinable, even in the case of wither/infect damage; this is no more robust than the previous workarounds were. */
card_instance_t* combat_damage_being_dealt(event_t event)
{
  card_instance_t* damage = current_damage_card_instance(event);
  if (damage && damage_is_combat(damage)) {
    int amount = damage_amount_from_source_marker(damage);
    if (amount > 0) {
      damage->targets[16].player = amount;
      return damage;
    }
  }

  return NULL;
}

/* Returns a damage card during EVENT_DEAL_DAMAGE if it's noncombat damage and doing more than 0.  damage->targets[16].player will hold the damage dealt if it's
 * determinable, even in the case of wither/infect damage; this is no more robust than the previous workarounds were. */
card_instance_t* noncombat_damage_being_dealt(event_t event)
{
  card_instance_t* damage = current_damage_card_instance(event);
  if (damage && !damage_is_combat(damage)) {
    int amount = damage_amount_from_source_marker(damage);
    if (amount > 0) {
      damage->targets[16].player = amount;
      return damage;
    }
  }

  return NULL;
}

/* Returns a damage card during EVENT_DEAL_DAMAGE if it's doing more than 0.  damage->targets[16].player will hold the damage dealt if it's determinable, even
 * in the case of wither/infect damage; this is no more robust than the previous workarounds were. */
card_instance_t* damage_being_dealt(event_t event)
{
  card_instance_t* damage = current_damage_card_instance(event);
  if (damage) {
    int amount = damage_amount_from_source_marker(damage);
    if (amount > 0) {
      damage->targets[16].player = amount;
      return damage;
    }
  }

  return NULL;
}

// Returns nonzero if this damage card is being dealt to a planeswalker.
int damage_is_to_planeswalker(card_instance_t* damage)
{
  if (!damage) {
    return 0;
  }

  if (damage->damage_target_card == -1) {
    return damage_is_combat(damage)
           && damage_source_ref_is_valid(damage)
           && check_special_flags(damage->damage_source_player, damage->damage_source_card, SF_ATTACKING_PWALKER);
  }

  return damage_target_ref_is_valid(damage) && is_planeswalker(damage->damage_target_player, damage->damage_target_card);
}

static int if_attached_creature_dies_do_something(int player, int card, event_t event)
{
  card_instance_t* instance = get_card_instance(player, card);
  int p = instance->damage_target_player;
  int c = instance->damage_target_card;

  int mode = instance->targets[1].card;
  // modes
  // 1<<0 --> Controller of source gains life equal to attached creature toughness [Abattoir Ghoul]
  // 1<<1 --> Controller of source may draw a card [Rot Wolf]

  if (damage_card_ref_is_valid(p, c) && event == EVENT_GRAVEYARD_FROM_PLAY) {
    card_instance_t* attached = in_play(p, c);
    if (attached
        && affect_me(p, c)
        && attached->kill_code > 0
        && attached->kill_code < KILL_REMOVE
        && !check_special_flags2(p, c, SF2_WILL_BE_EXILED_IF_PUTTED_IN_GRAVE)) {
      if (mode & 1) {
        instance->targets[1].player = get_toughness(p, c);
      }
      instance->targets[11].player = 1; // Otherwise resolve_gfp_ability() won't trigger.
      instance->damage_target_player = instance->damage_target_card = -1;
    }
  }

  // Keep the legacy around through combat damage so EVENT_GRAVEYARD_ABILITY can still see it if the source dies in combat.
  if (damage_card_ref_is_valid(instance->targets[0].player, instance->targets[0].card)
      && current_phase != PHASE_NORMAL_COMBAT_DAMAGE
      && current_phase != PHASE_FIRST_STRIKE_DAMAGE
      && other_leaves_play(player, card, instance->targets[0].player, instance->targets[0].card, event)) {
    kill_card(player, card, KILL_REMOVE);
  }

  if (resolve_gfp_ability(player, card, event, RESOLVE_TRIGGER_MANDATORY)) {
    if ((mode & 1) && instance->targets[1].player > 0) {
      gain_life(instance->targets[0].player, instance->targets[1].player);
    }
    if (mode & 2) {
      if (do_dialog(instance->targets[0].player, player, card, -1, -1, " Draw a card\n Pass", 0) == 0) {
        draw_cards(instance->targets[0].player, 1);
      }
    }
    kill_card(player, card, KILL_REMOVE);
  }

  if (current_phase == PHASE_CLEANUP) {
    kill_card(player, card, KILL_REMOVE);
  }

  return 0;
}

void if_a_creature_damaged_by_me_dies_do_something(int player, int card, event_t event, int mode)
{
  // modes
  // 1<<0 --> Controller of source gains life equal to attached creature toughness [Abattoir Ghoul]
  // 1<<1 --> Controller of source may draw a card [Rot Wolf]

  if (!damage_card_ref_is_valid(player, card)) {
    return;
  }

  card_instance_t* instance = get_card_instance(player, card);

  if (!in_play(player, card) || is_humiliated(player, card)) {
    return;
  }

  if (event == EVENT_DEAL_DAMAGE) {
    card_instance_t* damage = current_damage_card_instance(event);
    if (damage
        && damage->damage_target_card != -1
        && damage->damage_source_player == player
        && damage->damage_source_card == card
        && damage->info_slot > 0
        && damage_target_ref_is_valid(damage)) {
      if (instance->info_slot < 0) {
        instance->info_slot = 0;
      }
      if (instance->info_slot < DAMAGE_TRACKED_TARGET_LIMIT) {
        instance->targets[instance->info_slot].player = damage->damage_target_player;
        instance->targets[instance->info_slot].card = damage->damage_target_card;
        ++instance->info_slot;
      }
    }
  }

  if (instance->info_slot > 0
      && trigger_condition == TRIGGER_DEAL_DAMAGE
      && affect_me(player, card)
      && reason_for_trigger_controller == player) {
    int i;
    for (i = 0; i < instance->info_slot; ++i) {
      if (damage_card_ref_is_valid(instance->targets[i].player, instance->targets[i].card)) {
        int legacy = create_targetted_legacy_effect(player, card, &if_attached_creature_dies_do_something,
                                                    instance->targets[i].player, instance->targets[i].card);
        if (legacy != -1) {
          card_instance_t* leg = get_card_instance(player, legacy);
          leg->targets[0].player = player;
          leg->targets[0].card = card;
          leg->number_of_targets = 1;
          leg->targets[1].card = mode;
        }
      }
    }
    instance->info_slot = 0;
  }
}
