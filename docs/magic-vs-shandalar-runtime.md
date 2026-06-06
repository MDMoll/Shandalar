# Magic.exe vs Shandalar.exe Runtime Notes

This page compares the observed `CreateDIBSection` start-color bug with the two
primary runtime targets. It does not prove native Windows behavior.

## Current Finding

| Target | Evidence | Current conclusion |
| --- | --- | --- |
| `Program/Shandalar.exe` | User-reported crash occurs here; binary contains `WM_CREATE CreateDIBSection`, `D:\NewMagic\sources\sidlib\lib.c`, and imports `CreateDIBSection`; now patched at `0x1785b0` to pass `hSection = NULL`, at `0xa1a42`/`0xa1acd`/`0xa1af2` for default-name seed, name-editor bypass, and empty-name fallback, and with same-arrow movement hooks around `0x444aa7`. | First-class affected target for this bug; patched but needs full visible gameplay/default-name/movement testing. |
| Root `Shandalar.exe` | Same relevant strings as `Program/Shandalar.exe`; same active SHA-256 as Program copy; current CrossOver `MTG` shortcut targets root `C:\Shandalar\Shandalar.exe`. | Patched repo root, fresh C-drive, and `MTG` shortcut-path copies passed the reported crash point in CrossOver smoke testing before the later name-bypass and movement patches; both follow-up behaviors still need visible manual verification. |
| Root `FaceMaker.exe` | No-resolution/Korath-derived helper, now additionally patched at file offset `0x5f40` so its own `CreateDIBSection` wrapper passes `hSection = NULL`; direct CrossOver `MTG` launch rendered its UI with no unhandled exception/page fault/assertion in the log. | Strongly relevant to character creation, but direct launch does not prove the Shandalar-spawned path. |
| `Facemaker/Facemaker.exe` | Different older-looking helper build by hash; still imports/contains `CreateDIBSection`, `ChangeDisplaySettingsA`, and `WM_CREATE CreateDIBSection`; uses its own `Facemaker/` support-file layout. | Comparison evidence only, not the preferred active helper swap. |
| Root `Magic.exe` | Logged root Shandalar startup opens `C:\Shandalar\Magic.exe`; root and Program `Magic.exe` differ by SHA-256; related GDI/assertion strings/imports exist. | Must be tested separately from `Program/Magic.exe`; do not assume equivalence. |
| `Program/Magic.exe` | Imports `CreateDIBSection` and contains assertion/source-path strings, but does not run the campaign new-game color flow. | Related GDI/assertion code exists, but this exact bug is not proven to affect it. |
| Root and `Program/ManalinkEh.dll` | The shared Samite/Femeref/Kithkin healer handler, generic activated damage-prevention helper, `_check_timer_for_ai_speculation`, `_ai_decision_phase`, Piranha Marsh/Bojuka Bog implementations, and `select_target_impl()` live here for duel gameplay. Both root and `Program/` DLLs are patched to require `LCBP_DAMAGE_PREVENTION` before offering damage-prevention activations, to clamp `AiDecisionTime` so missing, invalid, or higher configured values use 270, to save both players' raw-mana rows during AI speculation, to route the Piranha Marsh and Bojuka Bog ETB triggers through `pick_player_duh()`, and to short-circuit AI player-only targets before the generic selector. | Relevant to duel-freeze reports, not the campaign start-color assertion. Copied CrossOver installs need their adjacent DLLs updated separately from the repo; the local `MTG` bottle now matches the repo Manalink hashes. |
| `Program/Image.dll`, `Program/Drawcardlib.dll` | Contain or import `CreateDIBSection`. | Shared graphics helpers may matter, but the reported fault module is `shandalar`. |

## What Appears Shared

| Shared area | Evidence |
| --- | --- |
| GDI bitmap creation | Both `Program/Shandalar.exe` and `Program/Magic.exe` import `CreateDIBSection`. |
| Bitmap blitting | Both import `BitBlt`. |
| Old assertion/source-path style | Both contain assertion strings and old `D:\NewMagic` source paths. |
| Runtime helper DLLs | Both rely on nearby DLLs such as deck/card/rendering helpers, though exact imports differ. |

## What Appears Shandalar-Specific

| Area | Evidence |
| --- | --- |
| Campaign/new-game color flow | The report happens after choosing a start color in `Shandalar.exe`. |
| FaceMaker handoff | `Shandalar.exe` contains a `\Facemaker.exe` string, and FaceMaker binaries contain the same `WM_CREATE CreateDIBSection` assertion text. |
| Observed fault module | Wine reports the fault in `shandalar` at `0x00579fea`. |
| `WM_CREATE CreateDIBSection` string | Found directly in `Program/Shandalar.exe`; not shown in the `Program/Magic.exe` string excerpt generated in this pass. |

## Test Matrix

| Test | Expected value |
| --- | --- |
| `Program/Shandalar.exe` new game, each start color | Determines color-specific vs shared post-color crash. |
| Root `Shandalar.exe` from root | Patched repo path passed the crash-point smoke test in `Shandalar-Win8-Test`; next proof is full visible gameplay including default-name character creation and same-arrow map stop. |
| Root `FaceMaker.exe` from root | Patched helper startup is verified in `MTG`; compare with Shandalar-spawned character creation. |
| `Program/Shandalar.exe` after matching `Program/zlib.dll`, adjacent Program config/font/card-data files, and Program CardArt assets are present in the copied install | Determines whether identical binary behaves differently with different working directory/assets/config; latest bounded log opened the Program card-data trio, `shandalar.dll`, and `Shandalar.ini` without the earlier fatal strings, but visible gameplay is still unproven. |
| `Program/Magic.exe` launch to graphics-heavy screen | Determines whether related GDI path fails outside campaign. |
| Root `Magic.exe` launch | Required because root and Program binaries differ by hash. |

## Current Status

Only `Shandalar.exe` is proven affected by the user report. The patched
Shandalar binaries pass the reported CrossOver crash point in smoke testing,
and now include follow-up default-name and same-arrow movement-stop patches.
`Magic.exe` remains relevant for comparison because it imports related GDI
functions and contains assertion strings, but this pass did not prove the
start-color bug affects `Magic.exe`.
