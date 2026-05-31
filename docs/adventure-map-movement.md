# Adventure Map Movement Notes

This note documents the same-arrow stop patch for the adventure-map behavior
where the player keeps walking until collision. It is based on static
disassembly and byte/hash verification in this checkout.

Needs visible testing: the patch is installed in root `Shandalar.exe`,
`Program/Shandalar.exe`, and the two local CrossOver bottle copies, but the
actual adventure-map feel still needs a manual gameplay pass.

## Verified Static Facts

| Finding | Evidence |
| --- | --- |
| The adventure-map movement loop consumes a normalized key code from `0x414127`. | `lldb` disassembly of `0x444100` through `0x444b20`; call at `0x444183`. GNU `objdump -D -Mintel` can show the same when available. |
| The loop stores the active key code in `[ebp-0x518]` and dispatches at `0x444aa7`. | Same disassembly; comparisons against `0x4700`, `0x4900`, `0x4f00`, `0x5100`, and `0x1b`. |
| Escape already has an exit-style path. | Key code `0x1b` jumps to `0x444a96`, which sets `[ebp-0x34] = 0`; the loop continues only while `[ebp-0x34] != 0` at `0x444b04`. |
| Current map coordinates appear to live at `0x5a8e68` and `0x5a8e6c`. | Movement blocks read/write those globals before and after validation. |
| The movement direction/animation global appears to be `0x583a28`. | Direction blocks write values `3`, `5`, `7`, and `1` before validation. |
| The new movement-completed flag lives at `0x583a2c`. | Search found no pre-patch references to little-endian address bytes `2c 3a 58 00`; the patch now writes this dword from the code cave. |
| Bounds checks limit target coordinates to `x < 15` and `y < 13`. | Checks at `0x44424b` through `0x44426d`. |
| Target cells are validated against a grid at `0x78e2b0`. | Grid lookup and zero-cell rejection at `0x444273` through `0x44428c`. |
| Mouse/drag movement can enqueue the same movement key-code family. | Function `0x446b89` initializes `0x4800`, `0x4900`, `0x4d00`, `0x5100`, `0x5000`, `0x4f00`, `0x4b00`, and `0x4700`, and calls `0x41d24b`. |

## Installed Patch

| Location | Patch role | Verification cue |
| --- | --- | --- |
| `0x44398c` | Function-entry hook initializes `0x583a2c` to `0`, preserves the original `mov dword ptr [0x78e2ac],0`, then jumps back to `0x443996`. | Disassembles as `jmp 0x46502d`, followed by NOP fill. |
| `0x444a2b` | Successful movement update hook preserves `mov [0x5a8e68],eax`, sets `0x583a2c` to `1`, then jumps back to `0x444a33` for the original Y update. | Disassembles as `jmp 0x465050`, followed by NOP fill. |
| `0x444aa7` | Key-dispatch hook replaces the original movement/Escape comparisons with a jump to the code cave. | Disassembles as `jmp 0x465070`, followed by NOP fill. |
| `0x46502d` | Code cave containing entry initialization, completed-step marking, repeated-key checks, and branches back to original movement or stop paths. | Disassembly shows references to `0x583a2c`, `0x583a28`, movement key codes, and stop path `0x444a96`. |

The same-key checks intentionally reuse the existing stop path rather than
inventing a new loop-exit path. After at least one successful movement update,
pressing the currently active direction key should clear `0x583a2c` and jump
to `0x444a96`.

## Direction Mapping

| Key code | Direction value checked in `0x583a28` | Original target block |
| --- | ---: | --- |
| `0x4700` | `3` | `0x4441af` |
| `0x4900` | `5` | `0x4441d6` |
| `0x4f00` | `1` | `0x444224` |
| `0x5100` | `7` | `0x4441fd` |

This mapping is from static disassembly. The physical key labels still need a
visible/manual gameplay check because Wine, numpad, and arrow-key normalization
may vary.

## Verification Commands

Run from `/Users/mdmoll/Shandalar/Shandalar`:

```sh
shasum -a 256 Shandalar.exe Program/Shandalar.exe
cmp -s Shandalar.exe Program/Shandalar.exe

lldb -b \
  -o 'target create Shandalar.exe' \
  -o 'disassemble --start-address 0x44398c --end-address 0x4439a0' \
  -o 'disassemble --start-address 0x444a2b --end-address 0x444a38' \
  -o 'disassemble --start-address 0x444aa7 --end-address 0x444ab2' \
  -o 'disassemble --start-address 0x46502d --end-address 0x4651a4' \
  -o quit
```

GNU `objdump -D -Mintel` is an acceptable substitute on systems where it is
available. The default Xcode `objdump` on this Mac does not support the same
options for this PE binary.

Expected hash for both repo copies:

```text
ad9ee80e0d377e7f1741e48aa0e33c3a8d7bd2873d43045e32bc42812aaa284b
```

Expected hook shape:

```text
44398c: e9 9c 16 02 00    jmp 0x46502d
444a2b: e9 20 06 02 00    jmp 0x465050
444aa7: e9 c4 05 02 00    jmp 0x465070
```

The code cave should include:

```text
mov dword ptr [0x583a2c],0
mov dword ptr [0x583a2c],1
cmp dword ptr [0x583a28],3
cmp dword ptr [0x583a28],5
cmp dword ptr [0x583a28],1
cmp dword ptr [0x583a28],7
jmp 0x444a96
```

## Needs Testing

| Question | Next test |
| --- | --- |
| Does pressing the same arrow stop ongoing map movement? | Launch a visible new or saved game, start moving, wait for at least one completed step, press the same direction key, and record whether movement stops. |
| Does Escape still stop movement without side effects? | Compare Escape behavior before and after one movement step. |
| Which physical keys map to the four dispatch codes? | Test arrow keys and numpad keys while observing direction. |
| Does mouse/drag movement still behave normally? | Click or drag on the map if supported and confirm the code cave does not make mouse movement feel stuck. |

## Caution

This is a binary compatibility patch, not a source rebuild. The direction-state
interpretation is an inference from static disassembly, so keep the original
pre-patch backup paths and hash evidence in the docs until a full visible
gameplay pass confirms the behavior.
