#!/bin/env perl

# Updates Magic.exe in-place.
# Adds a conservative attacker-selection undo: during the human declare-attackers
# picker, selecting an already-declared attacker clears STATE_ATTACKING before the
# player presses Done.
#
# Run from the directory containing the Magic.exe copy to patch.

use strict;
use warnings;
use Manalink::Patch;

#######################################
# In the human choose-attackers path. #
#######################################
#
# Original code rejects the chosen card if state has STATE_ATTACKING,
# STATE_TAPPED, or another disqualifying bit:
#
# 43c303: f7 46 08 14 00 02 00  test dword [esi+0x8], 0x20014
# 43c30a: 0f 85 5a 02 00 00     jne 0x43c56a
#
# Redirect to a code cave so STATE_ATTACKING becomes an undo action instead of
# a no-op.  The normal rejection remains unchanged for the other bits.
patch("Magic.exe", 0x43c303,
      "e9 c0 d8 01 00",          # jmp 0x459bc8
      (0x90) x 8);               # nop

###########################
# Code cave at 0x459bc8. #
###########################
#
# Entry state is the same as the original rejection check:
#   esi = get_card_instance(player, selected_card)
#
# Undo intentionally avoids remove_from_combat(), because that helper sets
# STATE_ATTACKED, which is wrong for "I changed my mind before Done".
patch(0x459bc8,
      "f7 46 08 04 00 00 00",    # test dword [esi+0x8], STATE_ATTACKING
      "0f 85 12 00 00 00",       # jne 0x459be7
      "f7 46 08 14 00 02 00",    # original test dword [esi+0x8], 0x20014
      "0f 85 88 29 fe ff",       # jne 0x43c56a
      "e9 29 27 fe ff",          # jmp 0x43c310

      # Undo selected attacker.
      "83 3d 70 f1 4e 00 00",    # cmp dword [0x4ef170], 0
      "7e 06",                   # jle 0x459bf6
      "ff 0d 70 f1 4e 00",       # dec dword [0x4ef170]
      "83 66 08 fb",             # and dword [esi+0x8], ~STATE_ATTACKING
      "c6 46 24 ff",             # mov byte [esi+0x24], -1
      "c7 05 bc ad 56 00 02 00 00 00", # mov dword [0x56adbc], 2
      "e9 5d 29 fe ff");         # jmp 0x43c56a
