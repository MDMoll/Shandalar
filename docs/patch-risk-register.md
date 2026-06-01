# Patch Risk Register

Binary patching is high risk. This register records known patch areas and
verification commands; it does not authorize more patching.

Generated evidence:
[generated/code-audit/patch-related-files.txt](generated/code-audit/patch-related-files.txt)
and
[generated/code-audit/binary-patch-sites.tsv](generated/code-audit/binary-patch-sites.tsv).

| Patch/Area | Target Binary | Offset/Symbol | Purpose | Current Evidence | Risk | Verification Command |
| --- | --- | ---: | --- | --- | --- | --- |
| Shandalar DIB `hSection = NULL` | `Shandalar.exe`; `Program/Shandalar.exe` | `0x1785b0` file offset | Avoid start-color `CreateDIBSection` crash path | documented bytes and hashes in AGENTS/bugs docs | high | `xxd -g1 -l 32 -s $((0x1785b0)) Shandalar.exe Program/Shandalar.exe` |
| Shandalar name seed | `Shandalar.exe`; `Program/Shandalar.exe` | `0xa1a42` | Seed default player name | documented bytes/hashes | high | AGENTS name-entry `xxd` command |
| Shandalar name-editor bypass/fallback | `Shandalar.exe`; `Program/Shandalar.exe` | `0xa1acd`; `0xa1af2`; cave `0x465170` | Avoid fragile manual name editor and empty-name copy | documented bytes/hashes | high | AGENTS bypass/fallback `xxd` commands |
| Shandalar same-arrow movement stop | `Shandalar.exe`; `Program/Shandalar.exe` | hooks `0x44398c`, `0x444a2b`, `0x444aa7`; cave `0x46502d` | Allow repeated direction key to stop map movement | documented disassembly expectation | high | AGENTS `lldb` disassembly command |
| FaceMaker DIB `hSection = NULL` | `FaceMaker.exe`; `Program/FaceMaker.exe` | `0x5f40` | Avoid helper-side DIB assertion path | documented bytes/hashes | high | `xxd -g1 -l 32 -s $((0x5f40)) FaceMaker.exe Program/FaceMaker.exe` |
| Damage-prevention healer guard | `ManalinkEh.dll`; `Program/ManalinkEh.dll` | `0x3bb035`; `0x381a25` | Gate Samite/Femeref/Kithkin activation on damage-prevention window | documented bytes/hashes; manual retest pending | high | AGENTS healer `xxd` and `shasum` commands |
| Declared-attacker undo | `Magic.exe`; `Program/Magic.exe` | hook `0x43c303`; cave `0x459bc8` | Let user clear `STATE_ATTACKING` before Done | documented bytes/hashes; manual edge-case retest pending | high | AGENTS attacker undo `xxd` and `shasum` commands |
| Patch scripts | `src/patches/*`; `Program/src/patches/*` | many offsets | Historical and current binary patch tooling | static inventory only | high | Read script, hash target, backup target, dry-run plan first |

## Do Not Patch Unless

All of these are true: target binary is identified, current SHA-256 is recorded,
expected bytes are verified, backup and rollback bytes exist, test plan exists,
and the user explicitly asks for the binary patch.
