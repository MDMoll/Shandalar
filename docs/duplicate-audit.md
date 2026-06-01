# Duplicate Audit

This audit records exact byte duplicates in the current checkout. It is cleanup
evidence only. An exact SHA-256 match proves two files have identical bytes; it
does not prove either path is unused or safe to remove.

## Verified on this machine

Run from `/Users/mdmoll/Shandalar/Shandalar` on 2026-05-31.

| Check | Result |
| --- | ---: |
| Non-`.git` files scanned | 52,045 |
| Non-`.git` bytes scanned | 1,983,719,478 |
| Duplicate SHA-256 groups | 10,852 |
| Files inside duplicate groups | 26,189 |
| Redundant files if one copy per hash were kept | 15,337 |
| Theoretical duplicate bytes if one copy per hash were kept | 456,259,116 bytes, about 435.1 MiB |

## Safe Duplicate Cleanup Pass

A later tracked-only cleanup pass on branch `codex/safe-duplicate-cleanup`
used `git ls-files` so only current tracked working-tree files were considered
for deletion. It removed one tracked OS cache file:
`CardArtNew/Thumbs.db`.

| Metric | Before | After | Delta |
| --- | ---: | ---: | ---: |
| Tracked files scanned | 52,010 | 52,009 | -1 |
| Duplicate SHA-256 groups | 10,797 | 10,797 | 0 |
| Files inside duplicate groups | 26,071 | 26,071 | 0 |
| Redundant files if one copy per hash were kept | 15,274 | 15,274 | 0 |
| Theoretical duplicate bytes if one copy per hash were kept | 454,817,805 | 454,817,805 | 0 |
| Files removed | 0 | 1 | +1 |
| Bytes removed | 0 | 524,800 | +524,800 |

The duplicate numbers did not change because `CardArtNew/Thumbs.db` was not
part of a duplicate hash group. See [cleanup-removed-files.md](cleanup-removed-files.md)
and generated evidence under [generated/safe-cleanup/](generated/safe-cleanup/).

## Largest Duplicate Families

These are grouped by the top-level directories that participate in each exact
duplicate hash group.

| Top-level directories | Duplicate groups | Files in groups | Theoretical duplicate bytes | Cleanup confidence |
| --- | ---: | ---: | ---: | --- |
| `Manalink3`, `Mods` | 6,134 | 14,657 | 126.0 MiB | Medium: packaged mod archives and mod trees may intentionally mirror each other. |
| `Program`, `Statwin` | 119 | 238 | 111.5 MiB | Low: `Statwin` files are runtime-looking UI/video assets and need launch-copy testing. |
| `Mods`, `Program` | 252 | 578 | 60.3 MiB | Low: duplicates cross mod staging and runtime bundle paths. |
| `CardArtManalink`, `CardArtNew` | 1,320 | 2,651 | 35.9 MiB | Low: card art stores may be alternate lookup trees. |
| `Manalink3`, `Mods`, `Sound` | 20 | 60 | 19.2 MiB | Low: sound/mod layout may be intentional. |
| `Program`, root | 113 | 226 | 9.4 MiB | Low: root and `Program/` launch surfaces are both still relevant. |
| `Program`, `Spr1024` | 103 | 206 | 7.3 MiB | Low: resolution-specific sprite folders need visual tests. |
| `Duelsounds`, `Manalink3`, `Mods` | 73 | 219 | 6.9 MiB | Low: sound assets are runtime-like. |

## Largest Individual Groups

| SHA-256 | Size | Count | Example paths | Cleanup confidence |
| --- | ---: | ---: | --- | --- |
| `40918e8c11cc883dc7c96bd6b04f9957cec3c3eff1a505cf5d3d8886c01f9c9b` | 33.3 MiB | 2 | `Mods/Art/Default Sonic 2014.7z`; `Manalink3/Mods/Art/Default Sonic 2014.7z` | Medium: exact duplicate archive pair. |
| `0e9d2214c919049c2b364b2a18dde0dc71e60d985f4be0ae074f999650c1fdde` | 791.4 KiB | 30 | `Mods/Art/_undo/.../DuelArt/Terr_BlackMana.bmp`; related terrain BMP files | Medium: `_undo` staging evidence, but still inside mod rollback layout. |
| `35f8da3703b83edb5ce07b1cd264ef010bb63c07acef9299077fd30701f67965` | 20.0 MiB | 2 | `Mods/Art/Duel_Shell Sarlack 2013.7z`; `Manalink3/Mods/Art/Duel_Shell Sarlack 2013.7z` | Medium: exact duplicate archive pair. |
| `d9471da4723f8f5e3de487e0568513a9ea960c37a2be03b3a737beefb578d655` | 9.7 MiB | 2 | `Mods/Art/Duel_Shell Salbei 2010.7z`; `Manalink3/Mods/Art/Duel_Shell Salbei 2010.7z` | Medium: exact duplicate archive pair. |
| `a2ebc7afb3d104c7bdddf3dfe2991a92bd44018f3da55590501d317e7686ebec` | 1.4 MiB | 6 | `Program/ShellArt/WinBk_Ladder16.bmp`; `Shellart/WinBk_Ladder16.bmp`; mod `_undo` copies | Low: crosses runtime and mod rollback paths. |
| `1d5111ef8a2fb08f538e0c234ae4ebef05d12cec621a956ab3f32eb6bf02b450` | 3.0 MiB | 2 | `Program/ManalinkEx.dll`; `Manalink3/Program/ManalinkEx.dll` | Low: DLL duplicate, but both package layouts may be meaningful. |
| `7f73a3df97ed4c9cc509c32e7bde651eeb5790ea8722d295d2e1c7c309242ef8` | 2.9 MiB | 2 | `Program/statwin/Water.avi`; `Statwin/Water.avi` | Low: runtime-looking UI video asset. |

## Cleanup Interpretation

| Finding | Meaning |
| --- | --- |
| Many duplicate groups cross `Manalink3/`, `Mods/`, and `Program/`. | The repo contains overlapping package/runtime/mod layouts, not just stray copies. |
| The largest high-confidence byte duplicates are archive pairs under `Mods/` and `Manalink3/Mods/`. | These are good future cleanup candidates, but still need an approved mod-distribution decision. |
| A tracked OS cache file was removed in the safe cleanup pass. | This reduced tracked file count by one and removed 524,800 bytes, but did not reduce duplicate-byte totals. |
| Runtime-like duplicates in `Program/`, root, `Statwin/`, sprite folders, sound folders, and DLLs remain low-confidence cleanup candidates. | Do not remove or archive them without launch-copy tests. |
| The theoretical 435.1 MiB savings is not an achievable safe cleanup target yet. | It assumes one path per hash can be kept, which is not proven for legacy runtime lookups. |

## Re-run Command

The audit used a Python SHA-256 scan over all files outside `.git`, grouping
only files with matching sizes before hashing. Re-run a summarized version with:

```sh
python3 - <<'PY'
from pathlib import Path
from collections import defaultdict
import hashlib

files = [p for p in Path('.').rglob('*') if '.git' not in p.parts and p.is_file()]
by_size = defaultdict(list)
for path in files:
    by_size[path.stat().st_size].append(path)

by_hash = defaultdict(list)
for paths in by_size.values():
    if len(paths) < 2:
        continue
    for path in paths:
        h = hashlib.sha256()
        with path.open('rb') as handle:
            for chunk in iter(lambda: handle.read(1024 * 1024), b''):
                h.update(chunk)
        by_hash[h.hexdigest()].append(path)

dups = {h: paths for h, paths in by_hash.items() if len(paths) > 1}
recoverable = sum(paths[0].stat().st_size * (len(paths) - 1) for paths in dups.values())
print('total_files', len(files))
print('duplicate_hashes', len(dups))
print('files_in_duplicate_groups', sum(len(paths) for paths in dups.values()))
print('redundant_file_count_if_one_kept', sum(len(paths) - 1 for paths in dups.values()))
print('recoverable_mib', round(recoverable / 1024 / 1024, 1))
PY
```
