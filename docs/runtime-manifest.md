# Runtime Manifest

Generated from local inspection on 2026-05-31 in
`/Users/mdmoll/Shandalar/Shandalar`.

This manifest is for identity, review, and scan handoff. It is not a malware
scan, a license grant, or proof of gameplay stability.

## Active Patched Runtime Files

| Path | SHA-256 | File type | Notes |
| --- | --- | --- | --- |
| `Shandalar.exe` | `ad9ee80e0d377e7f1741e48aa0e33c3a8d7bd2873d43045e32bc42812aaa284b` | PE32 GUI Intel 80386 | Root CrossOver launch target; patched for DIB, name, and movement behavior. |
| `Program/Shandalar.exe` | `ad9ee80e0d377e7f1741e48aa0e33c3a8d7bd2873d43045e32bc42812aaa284b` | PE32 GUI Intel 80386 | Same bytes as root; direct `MTG` launch is still deferred because `Program/zlib.dll` is absent. |
| `Magic.exe` | `5bf518d66342d79562efb1106449413ada06814a6c14818a1e3101fd470c82d1` | PE32 GUI Intel 80386 | Root duel executable; patched for declared-attacker undo. |
| `Program/Magic.exe` | `0fb8b87fe35c8be037ae3419a9b9cd70a27df840ae6af6c7488c2685046a74fa` | PE32 GUI Intel 80386 | Manalink launcher target; patched for declared-attacker undo. |
| `FaceMaker.exe` | `41f062874f94d732cc4feb40b568728b8462879fd3ec2bc55810f118e9c5f246` | PE32 GUI Intel 80386 | Active patched FaceMaker helper. |
| `Program/FaceMaker.exe` | `41f062874f94d732cc4feb40b568728b8462879fd3ec2bc55810f118e9c5f246` | PE32 GUI Intel 80386 | Same bytes as root active FaceMaker helper. |
| `ManalinkEh.dll` | `6a5fd8057d456d691fb87810eee8dbe1680b18d1c4c79530cbe036cb443df1eb` | PE32 DLL Intel 80386 | Root DLL patched for the damage-prevention activation freeze. |
| `Program/ManalinkEh.dll` | `7fc7ad86b5a3eaaa8879c76814dc454917f2e4b58acf15530e42fdcc78da2517` | PE32 DLL Intel 80386 | Program DLL patched at its own offset for the same damage-prevention guard. |

## Preserved Reference Executables

| Path | SHA-256 | File type | Why preserved |
| --- | --- | --- | --- |
| `FaceMaker-Original.exe` | `0471afcd0288a07422355ff2af224c40f8b29dc0a864eed90b3399e285f42c7e` | PE32 GUI Intel 80386 | Original FaceMaker reference for patch lineage. |
| `FaceMaker-nores.exe` | `43331d22d05787979af0d29cea1775fd3bcebf8acdb3c3be34524e9ca7762f4b` | PE32 GUI Intel 80386 | Korath/no-resolution helper; also observed as `FaceMaker-nores.exe /S` in the 2026-05-31 visible S2 new-game path. |
| `Program/FaceMaker-nores.exe` | `43331d22d05787979af0d29cea1775fd3bcebf8acdb3c3be34524e9ca7762f4b` | PE32 GUI Intel 80386 | Program-tree copy of the same no-resolution helper; preserve until all character-creation paths are understood. |

## Protected Cleanup False Positives

| Path | SHA-256 | File type | Why protected |
| --- | --- | --- | --- |
| `MENUBAK.PIC` | `34b3acb232d2a40c7e807b70cfcd3c9b95fe47a36e2061e41a6cc0d91c42b335` | data | Backup-looking name, but `.PIC` is a runtime resource family here. |
| `Program/MENUBAK.PIC` | `34b3acb232d2a40c7e807b70cfcd3c9b95fe47a36e2061e41a6cc0d91c42b335` | data | Same bytes as root. |
| `WINBAK01.PIC` | `982ac2bf1d968f3852616750f77a38e3ed246fe24ab6262c87c825fe9d2fd251` | data | Runtime-looking window backdrop/resource. |
| `Program/WINBAK01.PIC` | `982ac2bf1d968f3852616750f77a38e3ed246fe24ab6262c87c825fe9d2fd251` | data | Same bytes as root. |
| `WINBAK02.PIC` | `1a36a138907ebd13e947138961ec106efb342de99c01d09dc30ea27c0087c1fc` | data | Runtime-looking window backdrop/resource. |
| `Program/WINBAK02.PIC` | `1a36a138907ebd13e947138961ec106efb342de99c01d09dc30ea27c0087c1fc` | data | Same bytes as root. |
| `WORLBAK1.PIC` | `41757f0eeeea7f967869a1c9b0673a5ef822f634a38b2a7349c2da4c1b9d18fa` | data | Runtime-looking world backdrop/resource. |
| `Program/WORLBAK1.PIC` | `41757f0eeeea7f967869a1c9b0673a5ef822f634a38b2a7349c2da4c1b9d18fa` | data | Same bytes as root. |

## Generated or Archived Evidence Files

| Path | SHA-256 | File type | Status |
| --- | --- | --- | --- |
| `CardArtNew/Thumbs.db` | `d613ed811f078af12887dfb5d056373606c29d036574ee31315c427bf5f101ea` | Composite Document File V2 Document | Generated-looking, but still in place because it lives in an art folder. |
| `archive/generated-local/Duel.GID` | `e246ee3cf6bc430731dad14e6d1e0e84819418bd97450bb33707b7eec39c7ff6` | MS Windows help Bookmark | Archived generated help index. |
| `archive/generated-local/shandalar_dll.log` | `53e66f9ee0a7845250d3b5c19bcd4f2899dae05f5d855f529c21549d6e09e286` | CSV text | Archived runtime/debug log evidence. |
| `archive/backups/Rogues_Org_BAK.csv` | `61cc4cfe359ff156949366a3ab8b17a39adb766ab54e733ffeaa1f9cb0c169b8` | CSV text | Archived historical backup. |

## Refresh Commands

Run from `/Users/mdmoll/Shandalar/Shandalar`:

```sh
shasum -a 256 Shandalar.exe Program/Shandalar.exe Magic.exe Program/Magic.exe FaceMaker.exe Program/FaceMaker.exe ManalinkEh.dll Program/ManalinkEh.dll FaceMaker-Original.exe FaceMaker-nores.exe Program/FaceMaker-nores.exe
file Shandalar.exe Program/Shandalar.exe Magic.exe Program/Magic.exe FaceMaker.exe Program/FaceMaker.exe ManalinkEh.dll Program/ManalinkEh.dll FaceMaker-Original.exe FaceMaker-nores.exe Program/FaceMaker-nores.exe
tools/verify-share-readiness.sh
```

`tools/verify-share-readiness.sh` parses this manifest and checks every listed
SHA-256 row against the current file bytes.
