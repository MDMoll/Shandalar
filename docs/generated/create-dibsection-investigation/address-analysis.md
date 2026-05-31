# Address Analysis

Observed Wine fault:

| Field | Value |
| --- | --- |
| Fault address | `0x00579fea` |
| Module | `shandalar` |
| Module base | `0x00400000` |
| RVA | `0x00179fea` |
| Instruction | `rep movsl (%esi), %es:(%edi)` |
| Faulting destination | `EDI = 0x0ffec000` |
| Source | `ESI = 0x00982850` |
| Width-like value | `EDX = 0x320 = 800` |
| Height/row-like value | `EBP = 0x1e6 = 486` |
| Copy count at fault | `ECX = 0x88` dwords, or 544 bytes |

## Disassembly Result

`objdump -d` did not decode the PE `.text` section because the section is
marked as `DATA`. Re-running with `objdump -D -Mintel` produced useful output
in `disassembly-around-00579fea.txt`.

The faulting instruction is in a small copy routine:

```asm
579fc7: 8b 4b 20        mov ecx, dword ptr [ebx + 0x20]
579fca: 8b 43 2c        mov eax, dword ptr [ebx + 0x2c]
579fcd: 03 c8           add ecx, eax
579fcf: 8b 6c 24 20     mov ebp, dword ptr [esp + 0x20]
579fd3: 8b 73 18        mov esi, dword ptr [ebx + 0x18]
579fd6: 8b 7c 24 1c     mov edi, dword ptr [esp + 0x1c]
579fda: 0f af cd        imul ecx, ebp
579fdd: 03 ce           add ecx, esi
579fdf: 8b 74 24 14     mov esi, dword ptr [esp + 0x14]
579fe3: 03 f9           add edi, ecx
579fe5: 8b ca           mov ecx, edx
579fe7: c1 e9 02        shr ecx, 0x2
579fea: f3 a5           rep movsd dword ptr es:[edi], dword ptr [esi]
579fec: 8b ca           mov ecx, edx
579fef: 83 e1 03        and ecx, 0x3
579ff2: f3 a4           rep movsb byte ptr es:[edi], byte ptr [esi]
```

## Interpretation

The fault is at a block copy into a computed destination pointer. The code adds
a row/offset calculation to a base destination and then copies `EDX` bytes in
dword and byte chunks. With the crash registers, the copy length appears to be
`EDX = 800` bytes total, with `ECX = 136` dwords remaining at the reported
instruction.

The nearby code does not itself call `CreateDIBSection`; it looks like a
lower-level pixel/surface copy routine that can run after a DIB or graphics
buffer has been created or initialized. Therefore the assertion and page fault
may be two symptoms in the same graphics path rather than the exact same call
site.

`objdump` labels the surrounding symbol as `SetCardInDeck`, but this is not
strong evidence of the real function name. PE symbols are sparse and labels may
come from the nearest known symbol rather than a precise source function.

## CreateDIBSection Call-Site Patch

A later pass found the assertion-side `CreateDIBSection` call in
`Shandalar.exe` near virtual address `0x005791bb` / file offset `0x1785b0`.
The original bytes set up a normal six-argument `CreateDIBSection` call while
passing a non-null file-mapping handle as `hSection`:

```asm
mov edx, dword ptr [ebp + 0x04]  ; HDC
push ecx                         ; hSection
push edi                         ; ppvBits
push eax                         ; usage
mov ecx, dword ptr [ebp + 0x10]  ; BITMAPINFO
push ecx
push edx
call dword ptr [CreateDIBSection]
```

The compatibility patch changes only that call setup so `hSection` is `NULL`:

```asm
push 0
push edi
push eax
mov ecx, dword ptr [ebp + 0x10]
push ecx
push dword ptr [ebp + 0x04]
call dword ptr [CreateDIBSection]
```

Patch bytes:

| File offset | Original bytes | Patched bytes |
| --- | --- | --- |
| `0x1785b0` | `8b 55 04 51 57 50 8b 4d 10 51 52` | `6a 00 57 50 8b 4d 10 51 ff 75 04` |

After patching both root `Shandalar.exe` and `Program/Shandalar.exe`, both
files have SHA-256
`73aa1400ddc452462f4e714e349ff06d4564c133408cf2ab10e576ae65d441b9`.

CrossOver smoke testing with Wine `wscript SendKeys` then reached the
post-color resource load path without the original assertion/page fault. Logs:

| Log | Evidence |
| --- | --- |
| `/tmp/shandalar-win8test-nosection-sendkeys-cx.log` | Bottle-local patched test binary loaded `advfac64.pic`, `magic3.map`, `magic4.map`, and `begin.spr`. |
| `/tmp/shandalar-repo-patched-sendkeys-cx.log` | Patched repo `Y:\Shandalar\Shandalar\Shandalar.exe` loaded the same post-color resources from the repo path. |

This supports the theory that Wine/CrossOver dislikes the original file-backed
DIB section path more than the source-resource files themselves. It does not
prove full gameplay stability.

## Current Confidence

| Finding | Confidence | Reason |
| --- | --- | --- |
| Faulting instruction is a memory copy into a computed destination. | High | Direct disassembly at `0x00579fea`. |
| `EDX = 800` is the copy byte count for this routine. | Medium | Code uses `edx` to derive dword and byte copy counts. |
| `EBP = 486` participates in destination offset calculation. | Medium | Code multiplies a stride-like value by the stack argument loaded into `ebp`. |
| The crash could be a DIB/surface stride, height, or bounds issue. | Medium-high | Register values and assertion both point to graphics buffers, and the `hSection = NULL` DIB patch gets past the crash point in CrossOver smoke testing. |
| The exact source for `sidlib/lib.c:315` is absent from this checkout. | High | Static search found no `sldib/lib.c` or `sidlib/lib.c` source file. |
