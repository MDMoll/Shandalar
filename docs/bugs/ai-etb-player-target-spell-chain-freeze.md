# AI Player-Target Prompt And Spell Chain Freeze

## Symptom

The CrossOver `MTG` duel can stop with the `Spell Chain` box still movable when
an opponent-controlled land enters the battlefield and immediately targets a
player. The first reported case was Piranha Marsh; Bojuka Bog exposed the same
pattern.

A later manual test froze at the `Witch activates...` announcement for Augur of
Skulls. That card is not a land ETB trigger; it uses the generic activated
ability path with `TARGET_ZONE_PLAYERS`. That retest showed the problem class
also includes AI-controlled player-only activated abilities in Shandalar
adventure duels.

Another manual test on 2026-06-07 froze after the human player cast Glowing
Anemone, selected a target land, and reached the `Trigger` / `Decline` prompt.
That scenario is not an AI target-selection path; it broadens the active
symptom to the shared prompt/message layer after a human target choice succeeds.

Another 2026-06-07 manual test froze after an AI Warlock cast Thoughtseize on
its first turn. The `Spell Chain` pane was still movable and its Thoughtseize
card render was corrupted, while the separate cast announcement overlay rendered
the same card cleanly. Thoughtseize's AI discard selection is automatic after
resolution, so this case moves the likely failing layer earlier, around
Shandalar's Spell Chain/prompt/render/sound-service path rather than the
card-specific discard code.

This note records static source/runtime evidence plus copied-install parity. It
is not visible gameplay proof that the Piranha Marsh or Bojuka Bog scenarios are
fixed.

## Manual Retest Result

Manual gameplay testing showed that Bojuka Bog was **not** fixed by the earlier
selector-side mitigation, was **not** fixed by the later preselection-only
mitigation, was **not** fixed by resolving the effect immediately during trigger
discovery, and was **not** fixed by suppressing the trigger during
`EVENT_TRIGGER` then resolving it during `EVENT_END_TRIGGER`. Later manual
testing also showed that simply routing the lands through the engine-native
`comes_into_play_mode(..., RESOLVE_TRIGGER_AI(player))` trigger path was still
not enough for Bojuka Bog. Later resolve-spell/sentinel, `duh_mode()`-guarded
resolve-trigger, and strict raw-AI card-callsite wrapper mitigations also failed
manual gameplay testing. A later Bojuka Bog manual retest still froze after the
ManalinkEh-only resolver stack-bypass patch; live-process inspection showed the
Shandalar adventure duel was loading `Shandalar.dll`, not `ManalinkEh.dll`, for
the relevant resolver/card code.

The current mitigation moves the fix one layer higher, into `resolve_trigger()`.
For non-speculating AI-owned lands whose trigger globals identify
`TRIGGER_COMES_INTO_PLAY`, the resolver bypasses `put_card_or_activation_onto_stack()`
and dispatches `EVENT_RESOLVE_TRIGGER` directly to the land before the Spell
Chain processing dialog can open. It then restores the trigger globals, clears
the processing/triggering flags it set, dispatches `EVENT_TRIGGER_RESOLVED`, and
returns from `resolve_trigger()`. Human, speculative, trace-mode, and non-land
triggers keep the original stack/dialog resolver path.

On 2026-06-07, Augur of Skulls froze manually after the Shandalar land-CIP
resolver patch, proving the previous fix was still too narrow. The follow-up
Shandalar DLL patch now also hooks `Target::real_select_target()` and skips the
old target-presentation selector for non-speculating AI pure player targets
after legal candidates have already been built. Fresh manual Augur, Piranha
Marsh, and Bojuka Bog retests are still required.

On 2026-06-11, after the Loam Larva retest passed, a Merfolk Sovereign AI
activation froze at the Merfolk Shaman opponent announcement. That ability
targets an in-play Merfolk creature, so the Shandalar C++ targeter hook was
broadened to skip the old selector for non-speculating AI pure
`TARGET_ZONE_IN_PLAY` targets as well as pure player targets. Fresh Merfolk
Sovereign retesting is required after restart.

The later Glowing Anemone human `Trigger` / `Decline` freeze showed that the
prompt/button instability cannot be treated as only AI targeting. Wine Debugger
output from the same broader failure class showed a page fault at
`EIP == EDX == 0xfff50de4`, outside all loaded game modules. Static inspection
found that the previous WinMM timer patch removed one callback-thread call into
the legacy sound/service wrapper at `0x56d476`, but one shell/window-procedure
caller remained at `0x4ce62e` for the private MagSnd message `0x10101010`. The
current Shandalar executable patch NOPs that remaining `UpdateSnd` message call
while leaving card trigger logic unchanged.

The later Thoughtseize freeze proved the two call-site NOPs were still not a
complete mitigation. Wine Debugger again showed a page fault with
`EIP == EDX == 0xfff50de4`; the live process had loaded `MagSnd.dll`; and static
inspection showed `Shandalar.exe` still calls a `LoadLibraryA("MagSnd.dll")`
initializer wrapper at VA `0x56cf20` before storing sound function pointers.
The current executable mitigation patches that wrapper to return
sound-unavailable (`mov eax, 4; ret`) before `MagSnd.dll` can load or arm its own
timer/callback state. Fresh visible prompt/button stability retesting is still
required; this is a targeted callback/sound-initialization mitigation, not
gameplay proof, and adventure-mode MagSnd audio may be unavailable.

The follow-up Augur of Skulls retest still froze at the `Witch activates...`
announcement after the MagSnd initializer patch. Live `lsof` evidence confirmed
`MagSnd.dll` was no longer loaded, but `Statwin.dll` and `magvid.dll` were loaded.
Wine Debugger again showed `EIP == EDX == 0xfff50de4`. Static inspection found
that `Statwin.dll` dynamically loads `magvid.dll`, resolves its video exports,
and the MagVid path creates a worker thread plus stores video callback pointers.
The current Statwin mitigation patches its dynamic MagVid loader at
VA `0x10003610` / file offset `0x2a10` to return video-unavailable
(`mov eax, 7; ret`) before `magvid.dll` can load or create worker threads. Fresh
visible Augur/prompt retesting remains required; this is static/bottle evidence,
not gameplay proof, and old StatWin/AVI video behavior may be unavailable.

The later priestess pre-card duel-start freeze showed the same poisoned
callback fault could still happen after both sound and video helper DLLs were
out of the process. Live inspection of the frozen `C:\Shandalar\Shandalar.exe`
process found `MagSnd.dll` absent and `magvid.dll` absent, while Wine Debugger
again reported `EIP == EDX == 0xfff50de4` in a worker thread. The remaining
Shandalar-owned recurring async path was the `timeSetEvent()` callback at
VA `0x4ce8cd`, whose required role is only to increment the tick counter at
`0x589df0`, but whose first-run block also touched thread handles and priority.
The current executable mitigation rewrites the callback entry at file offset
`0xcdccd` to `inc dword [0x589df0]; ret 0x14`, preserving timer waits while
removing the extra callback-thread bookkeeping. Fresh visible duel-start and
prompt/button stability retesting remains required.

A later Thorn Thallid activation froze when Spell Chain appeared after the AI
targeted the player with the three-spore-counter damage ability. This moved the
failure out of the ETB-land-only path. Live `lsof` evidence from the frozen
`C:\Shandalar\Shandalar.exe` process showed `MagSnd.dll` and `magvid.dll` were
still absent, `Drawcardlib.dll`, `CardArtLib.dll`, and GDI+ were loaded, and
`CardArtManalink/Thorn Thallid.jpg` was open. A macOS sample showed the UI
thread idle in the host event loop while a Wine thread repeatedly sampled in
`ntdll.so`; Wine Debugger again reported the same invalid execution target,
`EIP == EDX == 0xfff50de4`. Static inspection then found Drawcardlib's
`GdiplusStartup()` used `SuppressBackgroundThread = 0`, while CardArtLib still
started GDI+ from `DllMain` and retained loaded `Gdiplus::Image` objects in its
cache. The current Drawcardlib source and rebuilt root/Program DLLs now set
`SuppressBackgroundThread = 1` and `SuppressExternalCodecs = 1`, call
`NotificationHook()`, check hook status, and unhook before `GdiplusShutdown()`.
They also route high-frequency GDI+ image creation, clone, graphics-context,
lock, draw, and image-size operations through checked wrappers so failed GDI+
status returns stop the draw path instead of cascading through null/invalid
objects.
A 2026-06-08 CrossOver root launch review then found the
first rebuilt `DrawCardLib.dll` crashed during process attach because it
carried `DYNAMIC_BASE`/`NX_COMPAT` startup characteristics; the accepted
rebuilt DLL keeps PE `DllCharacteristics` at `0x0000` via the Drawcardlib
Makefile's legacy-safe linker flags.

A later Brilliant Halo freeze showed the MCI disable and Drawcardlib hardening
were still insufficient. The copied install already matched the repo, live
`lsof` showed `CardArtManalink/Brilliant Halo.jpg` open with CardArtLib/GDI+
loaded, and Wine Debugger again reported `EIP == EDX == 0xfff50de4`. CardArtLib
has now been rebuilt so GDI+ starts lazily outside `DllMain`, uses explicit
notification hook/unhook handling, suppresses external codecs, validates
`Gdiplus::Image` load status before caching, and removes the old Boost
filesystem dependency. Fresh visible Spell Chain/card-rendering retesting
remains required.

## Finding

Piranha Marsh and Bojuka Bog have mandatory enters-the-battlefield triggers that
target a player. Augur of Skulls has an activated sacrifice ability that targets
a player during upkeep. Earlier mitigations changed the land target picker,
short-circuited the Manalink generic AI-only `TARGET_ZONE_PLAYERS` selection,
and wrapped the individual land `comes_into_play()` calls. Piranha Marsh still
froze manually after those card-specific wrapper attempts, so the land patch
treats the Spell Chain insertion itself as one failing layer.

The source snapshots now add an AI land CIP stack-bypass guard to
`resolve_trigger()`. The individual Piranha Marsh and Bojuka Bog bodies are back
to normal `comes_into_play(player, card, event) && pick_player_duh(...)`
resolution. The guard is constrained to `player == AI`, non-speculation,
`TRIGGER_COMES_INTO_PLAY`, matching trigger cause globals, and `TYPE_LAND`.

The runtime patch mirrors that source intent in both duel DLL families. The
ManalinkEh patch restores the Piranha Marsh and Bojuka Bog callsites to their
original `comes_into_play()` calls, then hooks `resolve_trigger()` before the
normal stack insertion. The Shandalar patch adds a small executable `.cdxai`
section to root and Program `Shandalar.dll`, hooks the adventure-duel
`resolve_trigger()` entry, and performs the same
AI/non-speculating/CIP/cause/land checks before directly resolving the land
trigger without opening Spell Chain. All non-matching contexts fall back to the
original resolver path.

The Augur failure showed that Shandalar adventure duels also need the generic
player-target selector hardening in `Shandalar.dll`, not only `ManalinkEh.dll`.
The follow-up Merfolk Sovereign failure showed the same Shandalar C++ targeter
could also freeze for pure in-play targets. The current Shandalar patch hooks
the C++ targeter at `0xcb16`, writes chosen candidate index `0` only when
`[this+0x8] == AI` and `[this+0x14]` is either `TARGET_ZONE_PLAYERS` or
`TARGET_ZONE_IN_PLAY`, and otherwise calls the original selector. The guard is
narrower than the whole targeter: mixed creature/player targets, human targets,
speculation, and candidate construction keep the original path.

The source snapshots also harden the graveyard/exile helper used by Bojuka Bog:
they bound exile-zone scans, validate player/deck inputs, and clear only the
affected player's `graveyard_source` row. That helper hardening is source-only
until a future rebuilt DLL or dedicated helper patch exists; the shipped DLL
change in this pass is the resolver cave described above.

## Remediation

| Area | Source paths | Runtime paths |
| --- | --- | --- |
| Piranha Marsh resolution target | `src/cards/zendikar.c`; `Program/src/cards/zendikar.c` | inline replacement at `0x3fe7a0`; Program `0x3c4930` |
| Bojuka Bog resolution target | `src/cards/worldwake.c`; `Program/src/cards/worldwake.c` | inline replacement at `0x3f63e0`; Program `0x3bc630` |
| Generic AI target selector | `src/functions/targets.c`; `Program/src/functions/targets.c` | ManalinkEh player-only hook/cave `0x469583`/`0x495ad0`; Program `0x429453`/`0x452cd0`; Shandalar.dll C++ pure player/in-play targeter hook/cave `0xcb16`/`0x1174920` in root and Program |
| AI land CIP resolver stack-bypass | `src/functions/events.c`; `Program/src/functions/events.c` | ManalinkEh resolver hook/cave `0x429acf`/`0x495b00`; Program `0x3ec7cf`/`0x452d00`; restored Piranha/Bojuka calls at `0x3fe77d`/`0x3f63bd` and Program `0x3c490d`/`0x3bc60d`; Shandalar.dll resolver hook `.cdxai` `0x94d34`/`0x1174800` in root and Program |
| Shandalar MagSnd update-message callback | n/a binary compatibility patch | root and Program `Shandalar.exe` call at VA `0x4ce62e` / file offset `0xcda2e` |
| Shandalar minimal WinMM timer callback | n/a binary compatibility patch | root and Program `Shandalar.exe` callback entry at VA `0x4ce8cd` / file offset `0xcdccd` |
| Shandalar MagSnd initialization disable | n/a binary compatibility patch | root and Program `Shandalar.exe` wrapper at VA `0x56cf20` / file offset `0x16c320` |
| Shandalar MCIWndCreateA disable | n/a binary compatibility patch | root and Program `Shandalar.exe` thunk at VA `0x578c10` / file offset `0x178010` |
| Statwin MagVid loader disable | n/a binary compatibility patch | root and Program `Statwin.dll` wrapper at VA `0x10003610` / file offset `0x2a10` |
| Drawcardlib GDI+ lifecycle and checked-wrapper hardening | `src/drawcardlib/*.c`; `src/drawcardlib/drawcardlib.h`; `Program/src/drawcardlib/*.c`; `Program/src/drawcardlib/drawcardlib.h`; `src/drawcardlib/Makefile`; `Program/src/drawcardlib/Makefile` | rebuilt root and Program `Drawcardlib.dll`; root and local `MTG` copied-install copies hash to `7b585c3f2c57bdbda66ceaeecdd48a8e97d67bb32010c1455165fcb44cd2966c`, suppress external codecs, check notification-hook status, route high-frequency GDI+ render operations through checked wrappers, treat interpolation setup as nonfatal for valid Wine/CrossOver dialog HDCs, and verify with PE `DllCharacteristics` `0x0000` |
| Source-only exile helper hardening | `src/functions/deck.c`; `Program/src/functions/deck.c` | source snapshots only; no shipped DLL helper patch |

The Shandalar `.cdxai` section is shared. The land-CIP resolver cave owns
`0x1174800..0x117491f`; the player-target selector cave starts at
`0x1174920`. `tools/patch-ai-land-cip-trigger-stack-bypass.py` must compare and
write only the resolver-owned prefix so it does not clobber the player-target
selector cave in already-patched DLLs.

The local `MTG` copied install was patched too. Backups were preserved as
`ManalinkEh.before-ai-land-cip-stack-bypass-patch.dll` in the root and Program
install folders and as `Shandalar.before-ai-land-cip-stack-bypass-patch.dll`,
`Shandalar.before-ai-player-target-selection-patch.dll`, and
`Shandalar.before-ai-in-play-target-selection-patch.dll` beside the root and
Program Shandalar helper DLLs.

## Current Hashes

| File | SHA-256 |
| --- | --- |
| `Shandalar.exe` | `ebba01ad04aba5fb78841f37b6c264dfd17f1d6ca6ccfcc9851c2972b64f5f6b` |
| `Program/Shandalar.exe` | `ebba01ad04aba5fb78841f37b6c264dfd17f1d6ca6ccfcc9851c2972b64f5f6b` |
| `Shandalar.dll` | `3a20ba36dabef6f5ff9be3a1990d8e959570764d4dff2ff88de0cea01d534f41` |
| `Program/Shandalar.dll` | `3a20ba36dabef6f5ff9be3a1990d8e959570764d4dff2ff88de0cea01d534f41` |
| `Statwin.dll` | `f1428cf548810f85df6f26b913d10dca16bc0f06a609a94c0cb0f0308347b0cf` |
| `Program/Statwin.dll` | `f1428cf548810f85df6f26b913d10dca16bc0f06a609a94c0cb0f0308347b0cf` |
| `Drawcardlib.dll` | `7b585c3f2c57bdbda66ceaeecdd48a8e97d67bb32010c1455165fcb44cd2966c` |
| `Program/Drawcardlib.dll` | `7b585c3f2c57bdbda66ceaeecdd48a8e97d67bb32010c1455165fcb44cd2966c` |
| `ManalinkEh.dll` | `0a2d77aa15fd18648a99398d7bb45b97e47a1a8ea6c01dc3e22433851940a507` |
| `Program/ManalinkEh.dll` | `e715b92495677abf940b3bfda438477d66896532b24c1b05a4ea2bc2179c2e22` |

These hashes include the earlier damage-prevention, AI decision-time, raw-mana
snapshot, Piranha Marsh, Bojuka Bog, generic AI target selector, AI land CIP
resolver stack-bypass handling, and Loam Larva basic-land selector patches. The
Shandalar DLL hash is the one used by the copied CrossOver `MTG`
adventure-duel launch path, and includes the land-CIP resolver hook, the C++
targeter pure player/in-play hook, and the basic-land subtype hook.

## Verification

```sh
python3 tools/patch-piranha-marsh-trigger-target.py
python3 tools/patch-bojuka-bog-trigger-target.py
python3 tools/patch-ai-player-target-selection.py
python3 tools/patch-ai-etb-player-target-preselect.py
python3 tools/patch-ai-land-cip-trigger-stack-bypass.py --apply
python3 tools/patch-statwin-disable-magvid-loader.py
python3 tools/patch-shandalar-disable-magsnd-init.py
python3 tools/patch-shandalar-magsnd-update-callback.py
python3 tools/patch-shandalar-minimal-winmm-timer-callback.py
tools/check-source-snapshot-parity.sh
tools/verify-install-tree.sh
tools/verify-crossover-mtg-state.sh
shasum -a 256 Shandalar.exe Program/Shandalar.exe Shandalar.dll Program/Shandalar.dll Statwin.dll Program/Statwin.dll Drawcardlib.dll Program/Drawcardlib.dll ManalinkEh.dll Program/ManalinkEh.dll
objdump -p Drawcardlib.dll Program/Drawcardlib.dll | rg -A2 'DllCharacteristics|DYNAMIC_BASE|NX_COMPAT'
```

Representative AI land CIP resolver byte checks:

```sh
xxd -p -l 5 -s $((0xcda2e)) Shandalar.exe
xxd -p -l 5 -s $((0xcda2e)) Program/Shandalar.exe
xxd -p -l 16 -s $((0xcdccd)) Shandalar.exe
xxd -p -l 16 -s $((0xcdccd)) Program/Shandalar.exe
xxd -p -l 6 -s $((0x16c320)) Shandalar.exe
xxd -p -l 6 -s $((0x16c320)) Program/Shandalar.exe
xxd -p -l 6 -s $((0x2a10)) Statwin.dll
xxd -p -l 6 -s $((0x2a10)) Program/Statwin.dll
xxd -p -l 10 -s $((0x429acf)) ManalinkEh.dll
xxd -p -l 5 -s $((0x3f63bd)) ManalinkEh.dll
xxd -p -l 5 -s $((0x3fe77d)) ManalinkEh.dll
xxd -p -l 247 -s $((0x495b00)) ManalinkEh.dll
xxd -p -l 10 -s $((0x3ec7cf)) Program/ManalinkEh.dll
xxd -p -l 5 -s $((0x3bc60d)) Program/ManalinkEh.dll
xxd -p -l 5 -s $((0x3c490d)) Program/ManalinkEh.dll
xxd -p -l 247 -s $((0x452d00)) Program/ManalinkEh.dll
xxd -p -l 9 -s $((0x94d34)) Shandalar.dll
xxd -p -l 269 -s $((0x1174800)) Shandalar.dll
xxd -p -l 9 -s $((0x94d34)) Program/Shandalar.dll
xxd -p -l 269 -s $((0x1174800)) Program/Shandalar.dll
xxd -p -l 18 -s $((0xcb16)) Shandalar.dll
xxd -p -l 64 -s $((0x1174920)) Shandalar.dll
xxd -p -l 18 -s $((0xcb16)) Program/Shandalar.dll
xxd -p -l 64 -s $((0x1174920)) Program/Shandalar.dll
```

The root hook, restored calls, and cave bytes should be:

```text
e92ccc06009090909090
e8ceed0300
e80e6a0300
83fb017542833d74857200017439813d782d7a00db000000752d391d1c7e73007525391d7cc16200751d3935209a730075156a015653e8d5c4f9ff83c40c85c07405e90f000000c705f8e9600000000000e98333f9ffc705f8e9600000000000ff35782d7a00ff35a4025902ff357cc16200ff35209a73005653e8f16cf8ff83c40889c7814f08000100106affba0100000029da526a7e5653e8122af9ff83c4148b0424a3209a73008b442404a37cc162008b442408a3a40259028b44240ca3782d7a00816708fffeffef68000300005653e8d9e7f9fd83c40c8f05209a73008f057cc162008f05a40259028f05782d7a00e94734f9ff
```

The Program hook, restored calls, and cave bytes should be:

```text
e92c6f06009090909090
e81ead0300
e81e2a0300
83fb017542833d74857200017439813d782d7a00db000000752d391d1c7e73007525391d7cc16200751d3935209a730075156a015653e8251afaff83c40c85c07405e90f000000c705f8e9600000000000e98390f9ffc705f8e9600000000000ff35782d7a00ff3524055402ff357cc16200ff35209a73005653e8f1e7f8ff83c40889c7814f08000100106affba0100000029da526a7e5653e89289f9ff83c4148b0424a3209a73008b442404a37cc162008b442408a3240554028b44240ca3782d7a00816708fffeffef68000300005653e8d917fefd83c40c8f05209a73008f057cc162008f05240554028f05782d7a00e94791f9ff
```

The root and Program section virtual-size header at file offset `0x1a8` should
now be `00020000`, mapping the shared cave slice that starts at `0x495b00` or
`0x452d00`.

The Shandalar AI target selector hook and cave bytes should be:

```text
e905fad40090909090909090909090909090
898d3cfdffff837f0801751e817f14001000007409817f1400020000750c31c0a36cd49400e9f5052bffb8f8424c00ffd0a16cd49400e9cd052bff0000000000
```

Manual proof still requires replaying duels where the opponent activates Augur
of Skulls, activates Merfolk Sovereign, casts Thoughtseize, and plays the
reported Piranha Marsh and Bojuka Bog lands, then confirming the duel remains
interactive. At least one additional AI-controlled land ETB trigger, one
additional AI player-target activated ability, one additional AI in-play target
activated ability, and one human `Trigger` / `Decline` prompt should still be
tested separately because these patches now cover both the land-CIP resolver
layer and the broader Shandalar prompt/sound-initialization layer.
