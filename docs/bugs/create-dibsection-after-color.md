# Assertion after Choosing Start Color: `WM_CREATE CreateDIBSection`

## Status

| Area | Status |
| --- | --- |
| Static analysis | Performed on 2026-05-30 with `rg`, `file`, `strings`, and `objdump`. |
| Forum lead | Checked the Slightly Magic thread at `https://slightlymagic.net/forum/viewtopic.php?f=76&t=17786`; it reports the same assertion and points to permissions, paging file, FaceMaker resolution changes, and a Windows 7 vs Windows 8 difference. |
| Local file state | `Program/FaceMaker.exe` restored from `Program/FaceMaker-nores.exe`; FaceMaker support files copied from `Manalink3/Program/`. The known Korath/thread FaceMaker matches the no-resolution helper already in this checkout; active root and `Program` FaceMaker helpers are now patched beyond that no-resolution baseline. |
| Current active fix | Root `Shandalar.exe` and `Program/Shandalar.exe` were patched at file offset `0x1785b0` to pass `hSection = NULL` to `CreateDIBSection` instead of the game's file-mapping handle. Active root and `Program` `FaceMaker.exe` were also patched at file offset `0x5f40` for the same `hSection = NULL` change. Later Shandalar patches seed `mPlayer` at `0xa1a42`, bypass the fragile manual name editor at `0xa1acd`, and add an empty-name fallback through code cave `0x465170`/file offset `0x64570`. |
| Runtime testing | User testing reports the FaceMaker/no-resolution swap and the `MTG` app-default Win8/desktop/pagefile settings did not resolve the assertion, and the first Shandalar-only patch still reproduced for the user. After patching Shandalar locally, `Shandalar-Win8-Test` repo/C-drive paths and the older `MTG` C-drive path were launched; Wine `wscript` SendKeys drove Start New Game far enough to load post-color resources `advfac64.pic`, `magic3.map`, `magic4.map`, and `begin.spr` with no `Unhandled exception`, page fault, or `WM_CREATE CreateDIBSection` assertion in the logs. After the follow-up FaceMaker patch, direct `C:\Shandalar\FaceMaker.exe` in `MTG` rendered without that assertion. After the name-editor bypass/fallback patch, a visible 2026-05-31 `MTG` SendKeys run reached the adventure map for the default/first start-color path; the raw log also observed `FaceMaker-nores.exe /S`. |
| Native Windows testing | Not performed in this pass. |
| Wine/CrossOver testing | User-provided Wine/CrossOver crash log analyzed; the local `MTG` bottle registry was updated from `C:\pagefile.sys 27 77` to `C:\pagefile.sys 512 1024` and configured to launch `Shandalar.exe`/`Magic.exe`/`FaceMaker.exe` in a `1024x768` virtual desktop with app-specific Windows 8 compatibility, but user retesting still failed. Fresh `Shandalar-Win8-Test` then verified the patched repo `Shandalar.exe` past the reported crash point. Direct patched FaceMaker startup was verified, and S2 now records one visible default/first start-color path reaching the map in `MTG`; the other colors remain unverified. |

Primary evidence is preserved under
`docs/generated/create-dibsection-investigation/`.

## Forum Evidence

The linked Slightly Magic thread is useful because it mirrors this failure
shape, but it does not prove a single fix:

| Observation from thread | How it affects this investigation |
| --- | --- |
| The reported assertion appears after choosing difficulty/color, then character creation looks wrong. | This matches the local bug shape closely enough to treat the thread as relevant. |
| Korath lists permissions/admin and paging file availability as likely causes. | Keep install-folder writability and paging file size in the CrossOver/native test matrix. |
| Korath attached a FaceMaker build that avoids changing resolution. | Compare and preserve the no-resolution FaceMaker, but do not assume it fixes the crash. |
| The thread reporter says the no-resolution FaceMaker did not make a difference. | This matches current user retesting: FaceMaker replacement alone is not sufficient. |
| The same reporter later says the same procedure/files worked on a Windows 8 laptop after Windows 7 failed. | This is the reason for creating `Shandalar-Win8-Test` as a real Win8 bottle instead of only using app-default `win8` inside the older `MTG` bottle. |

## Local FaceMaker State

This checkout already contained a no-resolution FaceMaker executable:

| Path | SHA-256 |
| --- | --- |
| `Program/FaceMaker-nores.exe` | `43331d22d05787979af0d29cea1775fd3bcebf8acdb3c3be34524e9ca7762f4b` |
| `FaceMaker-nores.exe` | `43331d22d05787979af0d29cea1775fd3bcebf8acdb3c3be34524e9ca7762f4b` |

`Program/` originally did not have `FaceMaker.exe`, `FaceData.txt`, `FaceButtons.txt`, or
`FaceArt/`, while `Program/Shandalar.exe` contains a `\Facemaker.exe`
reference and the forum thread identifies FaceMaker resolution changes as a
plausible cause of this exact assertion.

The fix was:

```sh
cp -X Program/FaceMaker-nores.exe Program/FaceMaker.exe
cp -X Manalink3/Program/FaceButtons.txt Program/FaceButtons.txt
cp -X Manalink3/Program/FaceData.txt Program/FaceData.txt
cp -RX Manalink3/Program/FaceArt Program/FaceArt
```

Verified static result at that stage:

```sh
cmp -s Program/FaceMaker.exe Program/FaceMaker-nores.exe
shasum -a 256 Program/FaceMaker.exe Program/FaceMaker-nores.exe
cmp -s Program/FaceData.txt Manalink3/Program/FaceData.txt
cmp -s Program/FaceButtons.txt Manalink3/Program/FaceButtons.txt
diff -qr Program/FaceArt Manalink3/Program/FaceArt
```

That was a targeted runtime-file fix, not a source rebuild and not a patch to
`Program/Shandalar.exe`. It restored the helper and support files, but it did
not change FaceMaker's own DIB allocation code.

User retest: this FaceMaker/no-resolution state did not resolve the assertion.
Do not treat the no-resolution swap alone as the fix.

Because the current CrossOver `MTG` Shandalar path is root
`C:\Shandalar\Shandalar.exe`, keep the matching root `FaceMaker.exe`,
`FaceData.txt`, `FaceButtons.txt`, and root face-art folder intact as well.

## Korath / No-Resolution FaceMaker Comparison

The user mentioned `Facemaker-Korath.exe`, but no file by that name was visible
under `/Users/mdmoll/Shandalar` during this pass. The known downloaded thread helper at
`/private/tmp/FaceMaker-Korath-thread.exe` matches the no-resolution helper
already in the repo, so it is not a materially different third variant:

| Path | SHA-256 | Relationship |
| --- | --- | --- |
| `/private/tmp/FaceMaker-Korath-thread.exe` | `43331d22d05787979af0d29cea1775fd3bcebf8acdb3c3be34524e9ca7762f4b` | Known downloaded copy of the forum attachment. |
| `FaceMaker-nores.exe` | `43331d22d05787979af0d29cea1775fd3bcebf8acdb3c3be34524e9ca7762f4b` | Same bytes as the known thread helper. |
| `Program/FaceMaker-nores.exe` | `43331d22d05787979af0d29cea1775fd3bcebf8acdb3c3be34524e9ca7762f4b` | Same bytes as `FaceMaker-nores.exe` and the known thread helper. |
| `FaceMaker.exe` | `41f062874f94d732cc4feb40b568728b8462879fd3ec2bc55810f118e9c5f246` | Active helper, patched from the no-resolution baseline. |
| `Program/FaceMaker.exe` | `41f062874f94d732cc4feb40b568728b8462879fd3ec2bc55810f118e9c5f246` | Active Program helper, same patched bytes as root `FaceMaker.exe`. |
| `FaceMaker-Original.exe` | `0471afcd0288a07422355ff2af224c40f8b29dc0a864eed90b3399e285f42c7e` | Original helper, 9 byte positions differ from no-resolution/Korath. |
| `Manalink3/Program/FaceMaker.exe` | `0471afcd0288a07422355ff2af224c40f8b29dc0a864eed90b3399e285f42c7e` | Same bytes as `FaceMaker-Original.exe`. |
| `Facemaker/Facemaker.exe` | `c8d15f006bb9bffe4dedf543dcc7fefe82f2eb651e1a1c165186a5c133bc3841` | Separate older-looking tool build. |

The original and no-resolution/Korath helpers have the same relevant imports:
`ChangeDisplaySettingsA`, `CreateWindowExA`, and `CreateDIBSection`.

The byte-level difference is small and display-focused:

| Address / evidence | Original | Korath/no-resolution | Inference |
| --- | --- | --- | --- |
| `cmp -l FaceMaker-Original.exe FaceMaker-nores.exe` | 9 byte positions differ | 9 byte positions differ | This is a tiny display-mode patch, not a different FaceMaker build. |
| Disassembly around `0x004057a6` | Compares an argument character with `0x31` (`1`) before the 1024x768 mode path. | Compares with `0x53` (`S`). | The normal `1` argument no longer triggers that branch. |
| Disassembly around `0x004058cf` | Default branch jumps to the current path. | Default branch jumps into the 1024x768 assignment path. | The helper is nudged toward 1024x768 without the old mode-change branch. |
| Disassembly around `0x004061bd` | Full wrapper builds a display mode structure and calls `ChangeDisplaySettingsA`. | Function is stubbed to `xor eax,eax; leave; ret`. | The resolution-change call is skipped and reported as success. |

This does not remove the helper's GDI/DIB code, and user testing says it is not
sufficient to fix the Shandalar start-color assertion.

The `Facemaker/Facemaker.exe` build is significantly different by hash and
size, but it is not a cleaner candidate for this bug. Static strings show it
still uses `ChangeDisplaySettingsA`, `CreateDIBSection`,
`WM_CREATE CreateDIBSection`, and local support files under `Facemaker/`
rather than the active root/`Program` FaceMaker layout. Treat it as historical
comparison evidence until a copy-based launch test proves otherwise.

## FaceMaker DIBSection Patch

Because the user still saw the same issue after the Shandalar patch, the active
FaceMaker helper was inspected for the same `CreateDIBSection` wrapper pattern.
The active helper is involved in the new-game name/portrait screen, and it
contains the same `WM_CREATE CreateDIBSection` assertion text.

Patched files:

| Path | SHA-256 after patch |
| --- | --- |
| `FaceMaker.exe` | `41f062874f94d732cc4feb40b568728b8462879fd3ec2bc55810f118e9c5f246` |
| `Program/FaceMaker.exe` | `41f062874f94d732cc4feb40b568728b8462879fd3ec2bc55810f118e9c5f246` |

Reference no-resolution/Korath hash, preserved in `FaceMaker-nores.exe` and
`Program/FaceMaker-nores.exe`:
`43331d22d05787979af0d29cea1775fd3bcebf8acdb3c3be34524e9ca7762f4b`.

Patch bytes:

| File offset | Original no-resolution bytes | Patched bytes | Effect |
| --- | --- | --- | --- |
| `0x5f40` | `8b 55 04 51 57 50 8b 4d 10 51 52` | `6a 00 57 50 8b 4d 10 51 ff 75 04` | Replace `push hSection` with `push 0` while preserving the `CreateDIBSection` call argument order. |

Verification:

```sh
xxd -g1 -l 32 -s $((0x5f40)) FaceMaker.exe
xxd -g1 -l 32 -s $((0x5f40)) Program/FaceMaker.exe
shasum -a 256 FaceMaker.exe Program/FaceMaker.exe FaceMaker-nores.exe Program/FaceMaker-nores.exe
cmp -l FaceMaker-nores.exe FaceMaker.exe
```

Expected first bytes at the patch site:

```text
00005f40: 6a 00 57 50 8b 4d 10 51 ff 75 04 ff 15 c8 72 42
```

Local CrossOver copied installs patched in this pass:

| Path | Backup path |
| --- | --- |
| `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/FaceMaker.exe` | `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/FaceMaker.before-hsection-null-patch.exe` |
| `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/Program/FaceMaker.exe` | `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/Program/FaceMaker.before-hsection-null-patch.exe` |
| `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/Shandalar-Win8-Test/drive_c/Shandalar/FaceMaker.exe` | `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/Shandalar-Win8-Test/drive_c/Shandalar/FaceMaker.before-hsection-null-patch.exe` |
| `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/Shandalar-Win8-Test/drive_c/Shandalar/Program/FaceMaker.exe` | `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/Shandalar-Win8-Test/drive_c/Shandalar/Program/FaceMaker.before-hsection-null-patch.exe` |

Direct CrossOver verification:

```sh
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar" --cx-log /tmp/facemaker-direct-after-patch-cx.log --debugmsg +seh,+bitmap,+process,+file "C:\Shandalar\FaceMaker.exe"
```

Result: the patched FaceMaker UI rendered in bottle `MTG`; the log had no
`Unhandled exception`, page fault, or `WM_CREATE CreateDIBSection` assertion.
This proves direct helper startup, not the full Shandalar-spawned
character-creation path.

## Shandalar DIBSection Patch

The first working fix from this pass is a narrow binary patch in
`Shandalar.exe`.
Static disassembly showed the assertion-side `CreateDIBSection` wrapper passes
a file-mapping handle as the API's `hSection` argument. Under Wine/CrossOver,
that path can produce the post-color assertion/page fault. The patch forces
that one argument to `NULL`, letting GDI/Wine allocate normal DIB memory.

Patched files after the first Shandalar hSection-only patch:

| Path | SHA-256 after hSection-only patch |
| --- | --- |
| `Shandalar.exe` | `73aa1400ddc452462f4e714e349ff06d4564c133408cf2ab10e576ae65d441b9` |
| `Program/Shandalar.exe` | `73aa1400ddc452462f4e714e349ff06d4564c133408cf2ab10e576ae65d441b9` |

Current active files have additional name-entry seed, name-editor bypass/fallback,
same-arrow movement-stop, WinMM timer-callback, MagSnd update-message,
MagSnd init-disable, and MCIWndCreateA disable compatibility patches and now
hash to
`ebba01ad04aba5fb78841f37b6c264dfd17f1d6ca6ccfcc9851c2972b64f5f6b`.

Original hash for both files before this patch:
`82c9b659dd131097b29931f0ed266c91d560103bc864d7eb6b806691d0dc9739`.

Patch bytes:

| File offset | Original bytes | Patched bytes | Effect |
| --- | --- | --- | --- |
| `0x1785b0` | `8b 55 04 51 57 50 8b 4d 10 51 52` | `6a 00 57 50 8b 4d 10 51 ff 75 04` | Replace `push hSection` with `push 0` while preserving the `CreateDIBSection` call argument order. |

Verification:

```sh
xxd -g1 -l 32 -s $((0x1785b0)) Shandalar.exe
xxd -g1 -l 32 -s $((0x1785b0)) Program/Shandalar.exe
shasum -a 256 Shandalar.exe Program/Shandalar.exe
```

Expected first bytes at the patch site:

```text
001785b0: 6a 00 57 50 8b 4d 10 51 ff 75 04 ff 15 68 78 98
```

This is not a source rebuild. The exact `sidlib/lib.c` source remains absent
from this checkout, so the patch is documented as a binary compatibility fix
based on disassembly and CrossOver smoke testing.

Local CrossOver copied installs patched in this pass:

| Path | Backup path |
| --- | --- |
| `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/Shandalar.exe` | `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/Shandalar.before-hsection-null-patch.exe` |
| `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/Program/Shandalar.exe` | `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/Program/Shandalar.before-hsection-null-patch.exe` |
| `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/Shandalar-Win8-Test/drive_c/Shandalar/Shandalar.exe` | `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/Shandalar-Win8-Test/drive_c/Shandalar/Shandalar.before-hsection-null-patch.exe` |
| `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/Shandalar-Win8-Test/drive_c/Shandalar/Program/Shandalar.exe` | `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/Shandalar-Win8-Test/drive_c/Shandalar/Program/Shandalar.before-hsection-null-patch.exe` |

The backups hash to the original pre-patch value
`82c9b659dd131097b29931f0ed266c91d560103bc864d7eb6b806691d0dc9739`; the
interim hSection-only bottle copies hashed to
`73aa1400ddc452462f4e714e349ff06d4564c133408cf2ab10e576ae65d441b9`. The
name-entry-only interim hash was
`bd784cc248d08455270a6bfae5004ead8f9723d8017f8db152add113e8d3a9db`. The
seed-plus-movement hash before the name-editor bypass/fallback was
`155a668c72867bd1274410eb05ca05fbb7bd9bed843b42d1583ea536805a4aaf`. The
WinMM-only local `MTG` bottle copies hashed to
`92cca05b493c28f6c29c0cc4bd0018499acd9a8cbdce06f9230da59d5be0a0ef` before the
later MagSnd update-message compatibility patch. Current local `MTG` bottle
copies have the follow-up name-editor bypass/fallback, movement-stop, WinMM
timer-callback, MagSnd update-message, MagSnd init-disable, and MCIWndCreateA
disable patches and hash to
`ebba01ad04aba5fb78841f37b6c264dfd17f1d6ca6ccfcc9851c2972b64f5f6b`.

## Shandalar Name-Entry Patches

After the DIB assertion was bypassed, the name-entry screen still behaved
badly under Wine/CrossOver. Static disassembly showed that Shandalar launches
`Facemaker.exe`, then draws the name picker itself and copies 64 bytes from a
decoded `namepick.pic` graphics surface into the player-name buffer at
`0x591228`. That makes the default name depend on a fragile pixel/surface read.

The first narrow patch skips that surface copy and seeds the existing buffer
with `mPlayer`. The following original code still sees the leading `m`, records
the gender value, shifts it away, and leaves default name text `Player`.

User retesting still reported the same issue, so the active patchset now also
bypasses the fragile manual name editor and adds a final empty-name fallback.
This is intentionally conservative for progress through new-game setup:
custom name editing is not verified in Wine/CrossOver, and the patched path
should continue with default `Player`.

Patched files:

| Path | SHA-256 after current combined patch |
| --- | --- |
| `Shandalar.exe` | `ebba01ad04aba5fb78841f37b6c264dfd17f1d6ca6ccfcc9851c2972b64f5f6b` |
| `Program/Shandalar.exe` | `ebba01ad04aba5fb78841f37b6c264dfd17f1d6ca6ccfcc9851c2972b64f5f6b` |

Patch bytes:

| File offset | Original bytes | Patched bytes | Effect |
| --- | --- | --- | --- |
| `0xa1a42` | `6a 40 68 c8 00 00 00 6a 00 6a 02 68 28 12 59 00 e8 a9 79 0d 00 83 c4 14` | `c7 05 28 12 59 00 6d 50 6c 61 c7 05 2c 12 59 00 79 65 72 00 90 90 90 90` | Replace a graphics-surface copy into `0x591228` with direct initialization to `mPlayer\0`. |
| `0xa1acd` | `68 28 12 59 00 e8 b6 01 00 00 83 c4 04 89 85 a8 fe ff ff` | `31 c0 89 85 a8 fe ff ff 90 90 90 90 90 90 90 90 90 90 90` | Skip the manual name editor call and store a success return value. |
| `0xa1af2` | `68 28 12 59 00` | `e9 79 2a fc ff` | Jump to the fallback guard at `0x465170` before copying the final name into the global player-name buffer. |
| `0x64570` | `cc` bytes in unused code cave space | `80 3d 28 12 59 00 00 75 14 c7 05 28 12 59 00 50 6c 61 79 ...` | If `0x591228` is empty, write `Player\0`; then run the original `strcpy(0x7a0770, 0x591228)` sequence and return. |

Verification:

```sh
xxd -g1 -l 40 -s $((0xa1a42)) Shandalar.exe
xxd -g1 -l 40 -s $((0xa1a42)) Program/Shandalar.exe
xxd -g1 -l 32 -s $((0xa1acd)) Shandalar.exe
xxd -g1 -l 32 -s $((0xa1acd)) Program/Shandalar.exe
xxd -g1 -l 64 -s $((0x64570)) Shandalar.exe
xxd -g1 -l 64 -s $((0x64570)) Program/Shandalar.exe
shasum -a 256 Shandalar.exe Program/Shandalar.exe
```

Expected first bytes at the patch site:

```text
000a1a42: c7 05 28 12 59 00 6d 50 6c 61 c7 05 2c 12 59 00
000a1a52: 79 65 72 00 90 90 90 90
```

The old 24-byte surface-copy sequence was searched with `perl -0777` and was
not found in either active Shandalar executable. The patched files were copied
into both local CrossOver bottle installs:

| Bottle path | Recorded SHA-256 |
| --- | --- |
| `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/Shandalar.exe` | `ebba01ad04aba5fb78841f37b6c264dfd17f1d6ca6ccfcc9851c2972b64f5f6b` |
| `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/Program/Shandalar.exe` | `ebba01ad04aba5fb78841f37b6c264dfd17f1d6ca6ccfcc9851c2972b64f5f6b` |
| `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/Shandalar-Win8-Test/drive_c/Shandalar/Shandalar.exe` | `ad9ee80e0d377e7f1741e48aa0e33c3a8d7bd2873d43045e32bc42812aaa284b` |
| `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/Shandalar-Win8-Test/drive_c/Shandalar/Program/Shandalar.exe` | `ad9ee80e0d377e7f1741e48aa0e33c3a8d7bd2873d43045e32bc42812aaa284b` |

Needs testing: these patches are statically verified and deployed to local
bottle copies, but the actual Shandalar-spawned character-creation path has
not been visibly re-tested in this session because automation did not reliably
drive the main menu into Start New Game.

## Shandalar Same-Arrow Movement Stop Patch

After name entry was patched, the adventure map still had the original
"continue until collision" feel. A narrow binary patch now reuses the existing
Escape stop path for repeated movement keys.

Patch summary:

| Address | Effect |
| --- | --- |
| `0x44398c` | Function-entry hook initializes movement-completed flag `0x583a2c` to `0`, then preserves the original `0x78e2ac` initialization. |
| `0x444a2b` | Successful movement update hook writes the new X coordinate and sets `0x583a2c` to `1`, then returns to the original Y-coordinate update. |
| `0x444aa7` | Key-dispatch hook checks movement key codes `0x4700`, `0x4900`, `0x4f00`, and `0x5100`. |
| `0x46502d` | Code cave containing the hook bodies and same-key checks. |

Verification:

```sh
lldb -b \
  -o 'target create Shandalar.exe' \
  -o 'disassemble --start-address 0x44398c --end-address 0x4439a0' \
  -o 'disassemble --start-address 0x444a2b --end-address 0x444a38' \
  -o 'disassemble --start-address 0x444aa7 --end-address 0x444ab2' \
  -o 'disassemble --start-address 0x46502d --end-address 0x4651a4' \
  -o quit
```

Needs testing: the patch is statically verified and deployed to local bottle
copies, but the adventure-map feel still needs a visible/manual gameplay pass.

## Current Active CrossOver State

The failed `MTG` state is still useful comparison evidence. These values are
historical and have been superseded by the later `win7`/`Shandalar1440` retest
state.

| Setting | Historical failed value | Evidence |
| --- | --- | --- |
| Repo `Shandalar.ini` | `Window = 2` | `rg -n "^Window\\s*=" Shandalar.ini Program/Shandalar.ini`. |
| Repo `Program/Shandalar.ini` | `Window = 2` | Same command. |
| Bottle `C:\Shandalar\Shandalar.ini` | `Window = 2` | `rg -n "^Window\\s*=" "/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/Shandalar.ini" "/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/Program/Shandalar.ini"`. |
| Bottle `C:\Shandalar\Program\Shandalar.ini` | `Window = 2` | Same command; hashes now match the repo ini files. |
| CrossOver bottle `MTG` paging file | `C:\pagefile.sys 512 1024` | `rg -n "PagingFiles" "/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/system.reg"`. |
| CrossOver bottle `MTG` app-default desktop | `Shandalar`, `1024x768` | Recorded by the app-default virtual desktop commands below; superseded by `Shandalar1440=1440x1080`. |
| CrossOver bottle `MTG` app-default Windows version | `win8` for `FaceMaker.exe`, `Magic.exe`, and `Shandalar.exe` | Recorded by the app-default Windows-version commands below; superseded by app-default `win7`. |

User retest: this config-only state still reproduced the issue. Do not mark
the CrossOver settings alone as a fix.

The current `MTG` retest state is:

| Setting | Current value | Evidence |
| --- | --- | --- |
| App-default Windows version | `win7` for `FaceMaker.exe`, `Magic.exe`, and `Shandalar.exe` | `sed -n '790,825p;965,970p' "/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/user.reg"`. |
| App-default virtual desktop | `Shandalar1440=1440x1080` | Same `user.reg` check. |

The separate Win8 comparison state is:

| Setting | Current value | Evidence |
| --- | --- | --- |
| CrossOver bottle | `Shandalar-Win8-Test` | Created with `cxbottle --bottle Shandalar-Win8-Test --create --template win8`. |
| Bottle template / architecture | `Template=win8`, `WineArch=win32` | `rg -n "Template|WineArch" ".../Bottles/Shandalar-Win8-Test/cxbottle.conf"`. |
| Reported Windows version | Microsoft Windows 8, `CurrentVersion=6.2`, `CurrentBuild=9200` | `grep -aEn 'CurrentBuild|CurrentVersion|ProductName' ".../Bottles/Shandalar-Win8-Test/system.reg"`. |
| Paging file | `C:\pagefile.sys 512 1024` | `grep -aEn 'PagingFiles' ".../Bottles/Shandalar-Win8-Test/system.reg"`. |
| Install path | `C:\Shandalar` | Bottle-local copy exists at `/Users/mdmoll/Library/Application Support/CrossOver/Bottles/Shandalar-Win8-Test/drive_c/Shandalar`. |
| Last recorded Shandalar binary hash for this comparison bottle | `ad9ee80e0d377e7f1741e48aa0e33c3a8d7bd2873d43045e32bc42812aaa284b` | Historical `shasum -a 256` evidence for the `Shandalar-Win8-Test` copy before the later WinMM timer-callback patch; this comparison bottle was not present during the 2026-06-07 WinMM verification. |
| Preferred launch path | `Y:\Shandalar\Shandalar\Shandalar.exe` from `Y:\Shandalar\Shandalar` after the repo binary patch | `/tmp/shandalar-repo-patched-sendkeys-cx.log` shows the patched repo exe passed the post-color resource load point. |
| Bottle-local comparison path | `C:\Shandalar\Shandalar.exe` from `C:\Shandalar` | Startup-smoked before the repo patch; the bottle-local copies were later patched with backups. |
| Virtual desktop | `ShandalarTall=1024x800` | `user.reg` contains app-default desktop `ShandalarTall` for `Shandalar.exe`, `Magic.exe`, and `FaceMaker.exe`. |

The exact CrossOver command used:

```sh
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG reg add "HKLM\System\CurrentControlSet\Control\Session Manager\Memory Management" /v PagingFiles /t REG_MULTI_SZ /d "C:\pagefile.sys 512 1024" /f
```

This command completed successfully. The copied ini files inside the `MTG`
bottle were also updated to `Window = 2` because that bottle has its own
`C:\Shandalar` install tree.

The exact app-default virtual desktop commands used:

```sh
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG reg add "HKCU\Software\Wine\Explorer\Desktops" /v Shandalar /t REG_SZ /d 1024x768 /f
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG reg add "HKCU\Software\Wine\AppDefaults\Shandalar.exe\Explorer" /v Desktop /t REG_SZ /d Shandalar /f
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG reg add "HKCU\Software\Wine\AppDefaults\Magic.exe\Explorer" /v Desktop /t REG_SZ /d Shandalar /f
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG reg add "HKCU\Software\Wine\AppDefaults\FaceMaker.exe\Explorer" /v Desktop /t REG_SZ /d Shandalar /f
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG reg add "HKCU\Software\Wine\AppDefaults\Shandalar.exe" /v Version /t REG_SZ /d win8 /f
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG reg add "HKCU\Software\Wine\AppDefaults\Magic.exe" /v Version /t REG_SZ /d win8 /f
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG reg add "HKCU\Software\Wine\AppDefaults\FaceMaker.exe" /v Version /t REG_SZ /d win8 /f
```

Verified startup/log smoke tests after this change:

| Command | Result | Evidence |
| --- | --- | --- |
| Timed `C:\Shandalar\Shandalar.exe` launch | Stayed alive until the alarm; no startup page fault. | `/tmp/shandalar-mtg-win8-appdefault-cx.log` showed `wined3d`, `explorer.exe /desktop`, and 1024-wide 8bpp DIB sections. |
| Timed direct `C:\Shandalar\FaceMaker.exe` launch | Stayed alive until the alarm; no startup page fault. | `/tmp/facemaker-mtg-win8-appdefault-cx.log` showed `wined3d`, `explorer.exe /desktop`, and 1024x768 8bpp DIB sections. |
| Visual smoke launch of `C:\Shandalar\Shandalar.exe` | Reached the `Magic: Shandalar` main menu. | Screenshot captured at `/tmp/shandalar-gui-smoke-2.png`; synthetic keypresses were denied by macOS Accessibility, so the new-game flow was not driven from this session. |
| Timed `Y:\Shandalar\Shandalar\Shandalar.exe` launch in `Shandalar-Win8-Test` | Stayed alive until cleanup after the timed smoke; no startup page fault. | `/tmp/shandalar-win8test-startup-cx.log` showed `wined3d` and successful `NtGdiCreateDIBSection` calls. |
| Patched bottle-local test copy `C:\Shandalar\Shandalar-nosection-test.exe` in `Shandalar-Win8-Test` | Wine `wscript` SendKeys drove the flow past the start-color crash point; no original page fault/assertion appeared. | `/tmp/shandalar-win8test-nosection-sendkeys-cx.log` shows post-color loads of `advfac64.pic`, `magic3.map`, `magic4.map`, and `begin.spr`. |
| Patched repo `Y:\Shandalar\Shandalar\Shandalar.exe` in `Shandalar-Win8-Test` | Wine `wscript` SendKeys drove the flow past the start-color crash point; no original page fault/assertion appeared. | `/tmp/shandalar-repo-patched-sendkeys-cx.log` shows post-color loads from the repo path and no `Unhandled exception`. |
| Patched `C:\Shandalar\Shandalar.exe` in `Shandalar-Win8-Test` | Wine `wscript` SendKeys drove the normal copied-install path past the start-color crash point; no original page fault/assertion appeared. | `/tmp/shandalar-win8test-cdrive-patched-sendkeys-cx.log` shows post-color loads from the C-drive path and no `Unhandled exception`. |
| Patched `C:\Shandalar\Shandalar.exe` in `MTG` | Wine `wscript` SendKeys drove the older practical shortcut path past the start-color crash point; no original page fault/assertion appeared. | `/tmp/shandalar-mtg-cdrive-patched-sendkeys-cx.log` shows post-color loads from the C-drive path and no `Unhandled exception`. |

Needs testing: a visible/manual gameplay pass should confirm character
creation, naming, map control, save/load, and at least one duel. The automated
SendKeys pass verifies the crash point only.

## Symptoms

Reported flow:

1. `Shandalar.exe` launches successfully.
2. New game flow begins.
3. User chooses a starting color.
4. Assertion dialog appears.
5. Wine reports a page fault in `shandalar`.

## Observed Assertion

User-reported assertion:

```text
Assertion Error

File-> D:\NewMagic\sources\sldib\lib.c, Line-> 315
WM_CREATE CreateDIBSection
```

Static evidence in this checkout found a closely matching string with
`sidlib`, not `sldib`:

```text
D:\NewMagic\sources\sidlib\lib.c
WM_CREATE CreateDIBSection
```

That string appears in `Program/Shandalar.exe`, root `Shandalar.exe`, and
FaceMaker binaries. The older archived debug file
`archive/debug-evidence/assertFile.txt` also contains the `sidlib/lib.c`
variant. A screenshot or exact dialog capture should confirm whether the
runtime dialog says `sldib` or `sidlib`.

## Observed Wine Crash

Important exact details from the user-provided Wine report:

```text
Unhandled exception: page fault on write access to 0x0ffec000 in wow64 32-bit code (0x00579fea).
EIP:00579fea
EAX:00000000 EBX:10abe400 ECX:00000088 EDX:00000320
ESI:00982850 EDI:0ffec000
=>0 0x00579fea in shandalar (+0x179fea) (0x000001e6)
0x00579fea shandalar+0x179fea: rep movsl (%esi), %es:(%edi)
Wine build: wine-11.0-8720-g4351038808c
Platform: x86_64 (guest: i386)
Version: Windows 7
Host system: Darwin
Host version: 24.6.0
```

Address calculation:

| Field | Value |
| --- | --- |
| Module base | `0x00400000` |
| Fault address | `0x00579fea` |
| Likely RVA | `0x00179fea` |

## Why `D:\NewMagic\sources...` Is Probably Not the Runtime Path

The path appears alongside generic assertion strings:

```text
File-> %s, Line-> %d
Assertion Error
```

This is consistent with a C/C++ assertion macro embedding `__FILE__` and
`__LINE__` from the developer's build machine. Static search found these old
`D:\NewMagic` source paths in binaries and archived debug evidence, not in a
runtime config that would instruct the game to load files from that path.

Do not diagnose a missing `D:` drive unless a runtime trace such as Process
Monitor or Wine `+file` logging shows actual access to `D:\NewMagic`.

## What `CreateDIBSection` Implies

`CreateDIBSection` is a Windows GDI API for creating a device-independent
bitmap and returning a pointer to bitmap memory. Failures can involve invalid
bitmap dimensions, bit depth, palette/color table data, device context issues,
memory allocation, compatibility-layer behavior, or a bad resource that leads
to invalid bitmap parameters.

Both `Program/Shandalar.exe` and `Program/Magic.exe` import `CreateDIBSection`
from GDI. This is GDI evidence, not DirectDraw evidence; no direct `DDRAW.DLL`
import was found in the earlier runtime dependency pass.

## Address-Level Notes

`objdump -D -Mintel` shows the faulting instruction inside a copy routine:

```asm
579fe3: 03 f9       add edi, ecx
579fe5: 8b ca       mov ecx, edx
579fe7: c1 e9 02    shr ecx, 0x2
579fea: f3 a5       rep movsd dword ptr es:[edi], dword ptr [esi]
579fec: 8b ca       mov ecx, edx
579fef: 83 e1 03    and ecx, 0x3
579ff2: f3 a4       rep movsb byte ptr es:[edi], byte ptr [esi]
```

The crash registers suggest a copy into a computed destination pointer:

| Register | Observation |
| --- | --- |
| `EDI = 0x0ffec000` | Faulting write destination. |
| `ESI = 0x00982850` | Source pointer. |
| `EDX = 0x320` | `800`, likely the byte count for this copy routine. |
| `EBP = 0x1e6` | `486`, participates in destination offset calculation. |
| `ECX = 0x88` | `136` dwords, or 544 bytes remaining/chunk count at fault. |

The nearby copy code does not itself call `CreateDIBSection`. It is likely a
lower-level surface/bitmap copy that happens after a DIB or related graphics
buffer is created or initialized.

## Hypotheses

| Hypothesis | Evidence For | Evidence Against | Confidence | Next Test |
| --- | --- | --- | --- | --- |
| Compile-time source path, not runtime `D:` path. | `D:\NewMagic` strings are embedded in binaries and assertion text; old debug files contain similar paths. | No runtime file trace has been run yet. | High | Run Process Monitor or Wine `+file` logging and check for actual `D:\NewMagic` access. |
| GDI DIB creation or DIB initialization failure. | Assertion message is `WM_CREATE CreateDIBSection`; `Shandalar.exe` imports `CreateDIBSection`; crash is in graphics-adjacent copy code; forcing `hSection = NULL` at the `CreateDIBSection` call gets past the crash point in CrossOver smoke testing. | Exact `sidlib/lib.c:315` source is absent; full gameplay is not yet manually verified. | High | Manual gameplay pass, then native Windows comparison if possible. |
| Bad stride/height/bounds in bitmap copy. | Fault at `rep movsd`; `EDX=800`, `EBP=486`, and `EDI` faulting destination suggest surface row/offset math. | Register meaning is inferred; no symbols/source for this exact code. | Medium | Compare display sizes/bit depths and inspect row-copy arguments with debugger if possible. |
| Display mode, bit depth, or palette incompatibility under Wine/CrossOver. | Old GDI/palette code plus Wine 32-bit guest on macOS; failure occurs after a graphics transition. | Native Windows has not been tested. | Medium | Test 640x480/800x600 virtual desktop, high-DPI off, 16-bit/256-color modes, XP/Win7 bottles. |
| FaceMaker executable or support files missing or failing during character creation. | `Program/Shandalar.exe` contains `\Facemaker.exe`; `Program/` had only `FaceMaker-nores.exe`; the forum thread names FaceMaker resolution changes for this assertion; FaceMaker contains the same DIB assertion text. | User retest says the no-resolution/Korath FaceMaker and support files did not fix the issue. Direct patched FaceMaker startup is verified, but the Shandalar-spawned path is not. | Medium | Keep files present and patched, then run a visible Shandalar-spawned character-creation test. |
| Too-small paging file in Wine/CrossOver. | The forum thread calls out paging file/swap; local `MTG` bottle had only `C:\pagefile.sys 27 77`; the Wine fault is a write into graphics-adjacent memory after DIB setup. | User retest says the pagefile/desktop/app-default changes did not fix `MTG`; the binary patch did get past the crash point in `Shandalar-Win8-Test`. | Low-medium | Keep a reasonable pagefile, but do not treat it as the primary fix. |
| Missing or malformed resource loaded after color choice. | Crash occurs after a new-game transition; resource corruption could produce bad bitmap parameters. | No file trace identifies a missing `.pic`, `.spr`, `.dat`, palette, or art file. | Medium-low | Use ProcMon/Wine `+file` and compare the last resource operations before assertion. |
| Wrong working directory or adjacent DLL/asset layout. | Old games often depend on nearby relative assets; current `MTG` shortcut targets root `C:\Shandalar\Shandalar.exe`; direct `C:\Shandalar\Program\Shandalar.exe` first missed `Program\zlib.dll`, then reached `drawcardlib.dll` findings for missing Program CardArt, `Manalink.ini`, Program `DuelArt` configs through `Planeswalker.dat`, six hardcoded Program `TT*.ttf` font paths, and older Program card-data files. | Root and `Program` `Shandalar.exe` bytes match, and this checkout plus the local `MTG` copy now have matching root/Program `zlib.dll` plus Program config/font/CardArt/card-data files observed so far. The latest bounded Program-path log opened the Program card-data trio, `shandalar.dll`, and `Shandalar.ini` without the earlier fatal strings, so binary identity and loader progress alone do not prove behavior. | Medium | Use root `C:\Shandalar\Shandalar.exe` for the current `MTG` retest; compare `Program` only after an exact-path visible retest. |
| Color-specific starting deck/resource. | Trigger follows choosing a start color. | No color matrix has been run; all colors may hit the same post-selection screen. | Low | Test all five colors with identical settings. |
| Missing VC++ runtime. | Reddit/user lead mentioned redistributables. | Import scan shows `MSVCRT.dll` and no direct `VCRUNTIME`, `MSVCP`, or `MFC` imports in launch targets. | Low | Install runtimes only after an exact missing-DLL dialog/log. |

## Source Findings

Static search did not find `sldib/lib.c`, `sidlib/lib.c`, or source for the
exact assertion site in this checkout. It did find several `WM_CREATE` handlers
in `src/deck/deckdll.cpp`, but those are not evidence for this campaign
`Shandalar.exe` assertion.

The source path is therefore best treated as embedded debug/assertion metadata
from old binary builds until a runtime trace proves otherwise.

## Binary Findings

`binary-strings-results.txt` shows:

| Binary | Relevant findings |
| --- | --- |
| `Program/Shandalar.exe` | Contains `WM_CREATE CreateDIBSection`, `D:\NewMagic\sources\sidlib\lib.c`, `CreateDIBSection`, assertion format strings, and other old source paths. |
| Root `Shandalar.exe` | Contains the same relevant assertion/source-path strings. |
| `Program/Magic.exe` and root `Magic.exe` | Contain assertion strings and `CreateDIBSection`, but the observed new-game campaign crash is reported for `Shandalar.exe`. |
| `Program/Image.dll`, `Program/Drawcardlib.dll` | Contain or import `CreateDIBSection`. |
| FaceMaker binaries | Also contain `WM_CREATE CreateDIBSection` and `sidlib` strings. |

`executable-imports.txt` shows both `Program/Shandalar.exe` and
`Program/Magic.exe` import `CreateDIBSection` and `BitBlt` from GDI.

## Runtime Findings

The start-color crash point was captured after the binary patch using Wine's
own `wscript SendKeys` in CrossOver. This is stronger than the earlier startup
smokes, but still weaker than a full manual gameplay pass.

The reported environment was Wine build `wine-11.0-8720-g4351038808c`, i386
guest, reported Windows 7, host Darwin `24.6.0`.

Additional local CrossOver evidence from 2026-05-30:

| Test | Result |
| --- | --- |
| Earlier direct logged launch of `C:\Shandalar\Program\Shandalar.exe` in bottle `MTG` | Failed before gameplay because `Program\zlib.dll` was missing, cascading into `image.dll`, `DrawCardLib.dll`, and `DECKDLL.dll` import failures. The repo and local copied bottle layouts now have matching `Program/zlib.dll`; a bounded exact-path log loaded the Program DLL chain without the old fatal loader pattern. Later Program-path attempts reached `drawcardlib.dll` missing-asset findings for `Program\CARDART\ManaSymbols.pic`, `Program\CARDART\modern\Triggering.png`, `Program\CARDART\planeswalker\LoyaltyBase.png`, and `Program\CARDART\modern\CardOv_Nyx.png`; those Program CardArt assets are now present, but the copied bottle Program path has not had a visible gameplay retest after the `CardOv_Nyx.png` copy. |
| Timed logged launch of `C:\Shandalar\Shandalar.exe` in bottle `MTG` after app-default virtual desktop and `Version=win8` registry entries | Stayed alive until the alarm; log showed `wined3d`, `explorer.exe /desktop`, and 1024-wide 8bpp DIB sections. No startup page fault appeared in that log. |
| Direct logged launch of `C:\Shandalar\FaceMaker.exe` in bottle `MTG` after app-default virtual desktop and `Version=win8` registry entries | Stayed alive until the alarm; log showed `wined3d`, `explorer.exe /desktop`, and 1024x768 8bpp DIB sections. No startup page fault appeared in that log. |
| Direct logged launch of `Y:\Shandalar\Shandalar\Shandalar.exe` in fresh bottle `Shandalar-Win8-Test` | Stayed alive until cleanup after the timed smoke; log showed `wined3d` and successful `NtGdiCreateDIBSection` calls. No startup page fault appeared in that log. |
| Direct logged launch of `C:\Shandalar\Shandalar.exe` in fresh bottle `Shandalar-Win8-Test` after copying the checkout into the bottle and setting `ShandalarTall=1024x800` | Stayed alive until cleanup after the timed smoke; log used the C-drive path and showed no startup page fault. |
| Patched `C:\Shandalar\Shandalar-nosection-test.exe` in fresh bottle `Shandalar-Win8-Test` plus Wine `wscript` SendKeys | Passed the original crash point; log shows `advfac64.pic`, `magic3.map`, `magic4.map`, and `begin.spr` loaded after DIB creation, with no `Unhandled exception` or `WM_CREATE CreateDIBSection` assertion. |
| Patched repo `Y:\Shandalar\Shandalar\Shandalar.exe` in fresh bottle `Shandalar-Win8-Test` plus Wine `wscript` SendKeys | Passed the original crash point from the actual repo binary; log shows the same post-color resources loaded from `Y:\Shandalar\Shandalar`. |
| Patched bottle-local `C:\Shandalar\Shandalar.exe` in fresh bottle `Shandalar-Win8-Test` plus Wine `wscript` SendKeys | Passed the original crash point from the normal copied install path; log shows the same post-color resources loaded from `C:\Shandalar`. |
| Patched bottle-local `C:\Shandalar\Shandalar.exe` in existing bottle `MTG` plus Wine `wscript` SendKeys | Passed the original crash point from the older practical shortcut path; log shows the same post-color resources loaded from `C:\Shandalar`. |
| Patched `C:\Shandalar\FaceMaker.exe` in existing bottle `MTG` | Direct helper UI rendered; `/tmp/facemaker-direct-after-patch-cx.log` shows no `Unhandled exception`, page fault, or `WM_CREATE CreateDIBSection` assertion. |
| Visual launch of `C:\Shandalar\Shandalar.exe` in bottle `MTG` | Reached the `Magic: Shandalar` main menu. macOS denied synthetic keypresses from this session, so the new-game path was not driven. |

## Display / Bit Depth Findings

No display-mode tests were performed. The strongest static clue is the
combination of `CreateDIBSection`, GDI imports, and a faulting memory copy with
dimension-like values `800` and `486`.

Test 640x480, 800x600, 1024x768, 16-bit color, 24/32-bit color, and 256-color
compatibility where available. For CrossOver, also test virtual desktop mode
and high-DPI/Retina disabled.

## Color-Specific Testing

Not tested. Use the matrix in
`docs/generated/create-dibsection-investigation/runtime-test-notes.md`.

If only one color fails, prioritize color-specific deck/resource differences.
If all colors fail, prioritize the shared post-color screen, display mode, DIB
creation, or global resource path.

## Working Directory Testing

Not tested. Launch from `Program/` first, because the repo docs identify that
as the intended runtime directory. Then compare repo-root launch and any helper
script launch in a controlled copy.

## Native Windows Test Plan

Use `docs/generated/create-dibsection-investigation/procmon-plan.md`.

The key proof is whether native Windows reproduces the assertion and whether
the last file/registry operations identify a missing or malformed resource.
Also confirm whether any runtime operation tries to access `D:\NewMagic`.

## Wine/CrossOver Test Plan

Use `docs/generated/create-dibsection-investigation/wine-debug-notes.md`.

Minimum useful CrossOver evidence:

| Item | Capture |
| --- | --- |
| Bottle | Name, 32/64-bit, reported Windows version. |
| Launch path | Exact command and working directory. |
| Display settings | Virtual desktop, resolution, high-DPI/Retina, renderer/backend if visible. |
| Components | Any installed runtimes/codecs. |
| Logs | `+seh,+gdi,+bitmap,+loaddll,+file` or CrossOver equivalent. |

## Candidate Workarounds

| Workaround | Confidence | Notes |
| --- | --- | --- |
| Use the combined-patch root `Shandalar.exe` / `Program/Shandalar.exe`. | High for the original Shandalar crash point, medium for the name-entry glitch | The `0x1785b0` hSection patch was verified past the post-color crash point in CrossOver `Shandalar-Win8-Test` using both a bottle-local patched test copy and the patched repo binary. The later `0xa1a42`, `0xa1acd`, and `0xa1af2` name-entry patches are statically verified and deployed to bottle copies; S2 gives visible confirmation for the default/first start-color path only. |
| Use the patched active `FaceMaker.exe` / `Program/FaceMaker.exe` that passes `hSection = NULL` to its own `CreateDIBSection` wrapper. | Medium | Direct CrossOver helper startup rendered successfully. The S2 run also observed `FaceMaker-nores.exe /S`, so preserve and test both helper names rather than assuming only `FaceMaker.exe` matters. |
| Launch the current `MTG` copied install from root `C:\Shandalar\Shandalar.exe`. | Medium | This is the installed shortcut target and the logged path with root DLLs present. The local bottle copy now has the combined Shandalar patch. |
| Use a 32-bit XP or Windows 7 CrossOver bottle. | Medium | The crash came from a Wine Windows 7 environment; XP comparison is useful. |
| Set `Window = 2` in both Shandalar ini files. | Medium | The shipped comments say mode 2 keeps Adventure Mode, Deckbuilder, and Facemaker in the windowed path and generally works better. |
| Increase the CrossOver/Wine paging file. | Medium | The forum thread identifies paging-file absence as a trigger; local `MTG` was changed to `C:\pagefile.sys 512 1024`. |
| Disable high-DPI/Retina and use Wine virtual desktop. | Medium | Reduces old-GDI scaling surprises; the current `MTG` retest uses app-default `Version=win7` with desktop `Shandalar1440=1440x1080` for `Shandalar.exe`, `Magic.exe`, and `FaceMaker.exe`. |
| Try a real 32-bit Windows 8 bottle with a bottle-local `C:\Shandalar` install. | Medium | The linked forum thread reports the same install working on Windows 8 after failing on Windows 7; app-default Win8 inside `MTG` was not sufficient in user testing, so `Shandalar-Win8-Test` was created and given a C-drive install copy. |
| Try app-default Windows 8 compatibility for the launch chain. | Low | Already applied to `MTG` and user retesting still reproduced the issue. Keep only as comparison evidence. |
| Try 640x480, 800x600, and 1024x768 virtual desktops. | Medium | Register values include `800`; old game screens may assume fixed dimensions. |
| Try 16-bit or 256-color compatibility where available. | Medium-low | Palette/DIB code may be sensitive, but no test has proved this yet. |
| Install x86 VC++ runtime only after exact missing-DLL evidence. | Low | Current import evidence does not point to VC++ runtime as primary cause. |
| Avoid a specific start color. | Low | Only useful if color matrix proves one color fails. |
| Restore/replace a resource. | Low | Do only after file trace identifies a missing/corrupt file. |

## Recommended Next Steps

1. Manually launch the patched repo `Shandalar.exe` and confirm the remaining
   start colors, naming, map control, save/load, and at least one duel.
2. Copy or reinstall the patched `Shandalar.exe` into any other CrossOver
   `C:\Shandalar` bottle copy before retesting that bottle.
3. Test all five start colors in the same bottle/settings.
4. Run native Windows or VM Process Monitor if available.
5. Capture screenshot/text of any remaining assertion to resolve `sldib` vs
   `sidlib`.
6. If source for `sidlib/lib.c` is later found, inspect line 315 for
   `CreateDIBSection` parameters and add logging/error handling only in a
   separate patch-focused pass.

## Open Questions

| Question | Current answer |
| --- | --- |
| Does the assertion say `sldib` or `sidlib`? | User report says `sldib`; checkout strings and archived debug evidence say `sidlib`. Needs screenshot/exact capture. |
| Is the crash native-Windows reproducible? | Unknown. |
| Is the crash color-specific? | Unknown. |
| Is the destination pointer from a failed/partial DIB allocation? | Plausible. The successful patch changes the DIB allocation path, but exact source semantics remain unknown. |
| Does the larger CrossOver paging file affect the crash? | Probably not the primary fix; user retesting says the config-only path still failed. Keep a reasonable pagefile anyway. |
| Which resource loads immediately after choosing color? | Verified in patched CrossOver logs: `advfac64.pic`, `magic3.map`, `magic4.map`, and `begin.spr` appear after the DIB creation path. |
| Does `Magic.exe` hit the same graphics path? | Unknown; it imports/contains related GDI/assertion strings but does not use the campaign new-game flow. |
| Does the combined Shandalar plus active FaceMaker patch fix the user-visible character-creation repro? | Partially. Direct FaceMaker startup is verified, the name-entry seed/bypass/fallback patches are statically verified, and S2 reached the adventure map for the default/first start-color path. The other colors still need visible manual retests. |
