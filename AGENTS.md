# AGENTS.md

This repo is a mixed game/runtime/assets/source/history checkout. Future agents
should keep the working tree safe and evidence-driven.

## Guardrails

| Rule | Why |
| --- | --- |
| Do not delete, move, or normalize binaries, art, data, decks, sounds, or historical files without explicit user approval. | Old game launch paths often depend on nearby files and exact names. |
| Treat `.exe`, `.dll`, `.dat`, `.pic`, `.spr`, `.csv`, `.dck`, `.cat`, `.res`, `.wav`, `.ogg`, `.avi`, and art/deck folders as potentially runtime-critical until proven otherwise. | Many of these are proprietary resource formats or runtime data. |
| Keep CrossOver/Wine notes separate from native Windows notes. | Bottle behavior is not the same as Windows behavior. |
| Preserve case and spacing in filenames. | Windows behavior, legacy scripts, and old resource lookups may rely on case-insensitive or short-name assumptions. |
| Do not assume `src/` rebuilds the shipped binaries. | Current evidence shows source and patch tooling, but not an end-to-end reproducible build. |
| Prefer docs and audit reports over cleanup. | The current repo needs mapping more than destructive pruning. |
| Record every launch/build command and its working directory. | Working directory is likely significant for both executables. |
| Do not download redistributable installers into the repo. | Runtime packages should be installed into Windows/CrossOver bottles by the user. |
| Do not declare binaries safe unless a named scanner was actually run and recorded. | Old game binaries can trigger false positives, but that is not proof of safety. |
| Put scan guidance/results in [docs/security-scan.md](docs/security-scan.md). | Safety claims need scanner name, version, path, hash, date, and result. |
| Do not casually replace the patched `Shandalar.exe` binaries with older copies. | Root and `Program/Shandalar.exe` contain documented patches at `0x1785b0` for the start-color `CreateDIBSection` crash, `0xa1a42` for the name-entry default buffer, `0xa1acd`/`0xa1af2` for the name-editor bypass and empty-name fallback, and movement hooks at `0x44398c`, `0x444a2b`, and `0x444aa7` using code cave `0x46502d`. |
| Do not casually replace the active `FaceMaker.exe` binaries with the older no-resolution copies. | Root and `Program/FaceMaker.exe` are now no-resolution/Korath-derived plus an additional documented `CreateDIBSection` compatibility patch. |

## Archive Policy

| Rule | Why |
| --- | --- |
| `archive/` is preserved evidence/history, not a deletion staging area. | Archived files may still explain old packaging, debugging, or local behavior. |
| It is acceptable to add clearly non-runtime evidence to `archive/` with documentation. | This keeps the root navigable without losing context. |
| Do not move runtime-like assets into `archive/` without explicit approval and a launch-copy test plan. | Many root files mirror or differ from `Program/` files in ways that are not fully understood. |
| When moving a tracked file, use `git mv` and update [docs/reorganization.md](docs/reorganization.md). | History and path evidence need to remain inspectable. |

## Default Launch Context

| Target | Working directory | Notes |
| --- | --- | --- |
| Root `Shandalar.exe` | `/Users/mdmoll/Shandalar/Shandalar` or `C:\Shandalar` in copied CrossOver bottle `MTG` | Current repo copy is patched for the start-color `CreateDIBSection` crash, default-name handling with name-editor bypass/fallback, and same-arrow map stop, hashes to `ad9ee80e0d377e7f1741e48aa0e33c3a8d7bd2873d43045e32bc42812aaa284b`, and has the same SHA-256 as `Program/Shandalar.exe`; local `MTG` and `Shandalar-Win8-Test` copied installs were patched too, with `Shandalar.before-hsection-null-patch.exe` backups preserving the original pre-DIB-patch bytes. |
| Root `Magic.exe` | `/Users/mdmoll/Shandalar/Shandalar` or `C:\Shandalar` in copied CrossOver bottle `MTG` | Logged root Shandalar startup opens this copy. It differs by SHA-256 from `Program/Magic.exe`. |
| `Program/Shandalar.exe` | `/Users/mdmoll/Shandalar/Shandalar/Program` | Same patched SHA-256 as root `Shandalar.exe`, but direct `C:\Shandalar\Program\Shandalar.exe` currently fails in bottle `MTG` because `Program\zlib.dll` is absent. Do not use it as the primary `MTG` retest path until the adjacent DLL layout is resolved. |
| `Program/Magic.exe` | `/Users/mdmoll/Shandalar/Shandalar/Program` | Manalink launcher path. `Program/Magic.exe` and root `Magic.exe` differ by SHA-256, so test and document the path used. |
| `FaceMaker.exe` | Same folder as the active Shandalar launch | New-game character creation expects this helper plus `FaceData.txt`, `FaceButtons.txt`, and face art. Active root and `Program` EXEs are patched from the no-resolution/Korath helper and no longer match `FaceMaker-nores.exe`. |
| `Manalink_Launcher.cmd` | repo root on Windows | The batch file sets `mlDir` to `Program` and starts `Magic.exe`. |

## Useful Inspection Commands

Run from `/Users/mdmoll/Shandalar/Shandalar` unless noted.

| Task | Command |
| --- | --- |
| List tracked docs | `git ls-files | rg -i '^(readme|agents|docs/)'` |
| Check PE architecture | `file Program/Shandalar.exe Program/Magic.exe Program/*.dll` |
| Inspect imports | `objdump -p Program/Magic.exe | rg "DLL Name:"` |
| Hash launch targets before scan reports | `shasum -a 256 Program/Shandalar.exe Program/Magic.exe Shandalar.exe Magic.exe` |
| Verify the Shandalar DIB patch | Run `xxd -g1 -l 32 -s $((0x1785b0)) Shandalar.exe` and `xxd -g1 -l 32 -s $((0x1785b0)) Program/Shandalar.exe`; expect `6a 00 57 50 8b 4d 10 51 ff 75 04` at the start of each dump. |
| Verify the Shandalar name-entry patch | Run `xxd -g1 -l 40 -s $((0xa1a42)) Shandalar.exe` and `xxd -g1 -l 40 -s $((0xa1a42)) Program/Shandalar.exe`; expect bytes that write `mPlayer` into `0x591228`, starting `c7 05 28 12 59 00 6d 50 6c 61`. |
| Verify the Shandalar name-editor bypass | Run `xxd -g1 -l 32 -s $((0xa1acd)) Shandalar.exe` and `xxd -g1 -l 32 -s $((0xa1acd)) Program/Shandalar.exe`; expect `31 c0 89 85 a8 fe ff ff` followed by NOPs, which skips the fragile manual name editor and treats the default name as accepted. |
| Verify the Shandalar empty-name fallback | Run `xxd -g1 -l 64 -s $((0x64570)) Shandalar.exe` and `xxd -g1 -l 64 -s $((0x64570)) Program/Shandalar.exe`; expect code that compares byte `[0x591228]` with zero and writes `Player` before copying to `0x7a0770` if needed. |
| Verify the Shandalar movement-stop patch | On this Mac, use `lldb -b -o 'target create Shandalar.exe' -o 'disassemble --start-address 0x44398c --end-address 0x4439a0' -o 'disassemble --start-address 0x444a2b --end-address 0x444a38' -o 'disassemble --start-address 0x444aa7 --end-address 0x444ab2' -o 'disassemble --start-address 0x46502d --end-address 0x4651a4' -o quit`; expect jumps into code cave `0x46502d`, flag storage at `0x583a2c`, and same-key stop branches to `0x444a96`. GNU `objdump -D -Mintel` is fine when available. |
| Verify the active Shandalar hash | `shasum -a 256 Shandalar.exe Program/Shandalar.exe`; both should be `ad9ee80e0d377e7f1741e48aa0e33c3a8d7bd2873d43045e32bc42812aaa284b` until a later documented patch changes them. |
| Verify the active FaceMaker state | `xxd -g1 -l 32 -s $((0x5f40)) FaceMaker.exe Program/FaceMaker.exe`; expect `6a 00 57 50 8b 4d 10 51 ff 75 04`. Then run `shasum -a 256 FaceMaker.exe Program/FaceMaker.exe FaceMaker-nores.exe Program/FaceMaker-nores.exe` and keep the patched active hash distinct from the reference no-resolution hash. |
| Verify FaceMaker support files | `cmp -s Program/FaceData.txt Manalink3/Program/FaceData.txt && cmp -s Program/FaceButtons.txt Manalink3/Program/FaceButtons.txt && diff -qr Program/FaceArt Manalink3/Program/FaceArt` |
| Verify the active start-color config | `rg -n "^Window\\s*=" Shandalar.ini Program/Shandalar.ini`; both should be `Window = 2` unless a later test deliberately changes them. |
| If testing CrossOver bottle `MTG`, verify its copied install too | `rg -n "^Window\\s*=" "/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/Shandalar.ini" "/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar/Program/Shandalar.ini"` |
| Verify the current `MTG` app-default desktop/version settings | `sed -n '795,825p' "/Users/mdmoll/Library/Application Support/CrossOver/Bottles/MTG/user.reg"`; expect `FaceMaker.exe`, `Magic.exe`, and `Shandalar.exe` app-default `Version=win7` and desktop `Shandalar1440`; also verify `Shandalar1440=1440x1080` under `HKCU\Software\Wine\Explorer\Desktops`. |
| Verify the fresh Win8 bottle test state | `ls -ld "/Users/mdmoll/Library/Application Support/CrossOver/Bottles/Shandalar-Win8-Test/drive_c/Shandalar"` and `rg -n "ShandalarTall|Desktop" "/Users/mdmoll/Library/Application Support/CrossOver/Bottles/Shandalar-Win8-Test/user.reg"`; expect a `C:\Shandalar` copy and app-default desktop `ShandalarTall=1024x800`. |
| Search docs/scripts for stale paths | `rg -n -i --glob '*.txt' --glob '*.md' --glob '*.cmd' --glob '*.bat' --glob '*.ini' --glob '*.pl' -e 'https?://|www\.|[A-Z]:\\|c:\\|d:\\' .` |
| Review limited reorg history | `sed -n '1,220p' docs/reorganization.md` |
| Count files by top directory | `find . -type f | sed 's#^./##' | awk 'BEGIN{FS="/"} {print $1}' | sort | uniq -c | sort -nr | head -40` |
| Count extensions | `find . -type f | awk 'BEGIN{IGNORECASE=1} {n=$0; sub(/^.*\//,"",n); if (n !~ /\./) ext="[none]"; else {ext=n; sub(/^.*\./,"",ext); ext=tolower(ext)}; count[ext]++} END{for (e in count) print count[e], e}' | sort -nr | head -40` |
| Build dry run | `cd src && make -n` |

## CrossOver/Wine Commands to Record

Use only when the tool and bottle are available. Always record the exact bottle,
working directory, command, exit code, and any visible UI/log result.

```sh
cd /Users/mdmoll/Shandalar/Shandalar

wine Shandalar.exe --help
wine Shandalar.exe

cd /Users/mdmoll/Shandalar/Shandalar/Program
wine Magic.exe

/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar" "C:\Shandalar\Shandalar.exe"
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar" "C:\Shandalar\Magic.exe"
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar" "C:\Shandalar\FaceMaker.exe"
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar\Program" "C:\Shandalar\Program\Magic.exe"
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test --workdir "C:\Shandalar" "C:\Shandalar\Shandalar.exe"
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle Shandalar-Win8-Test --workdir "Y:\Shandalar\Shandalar" "Y:\Shandalar\Shandalar\Shandalar.exe"
```

If a launch opens a GUI and cannot be observed from automation, mark it "needs
manual visual test" rather than guessing.

For the start-color assertion, the strongest current smoke logs are
`/tmp/shandalar-repo-patched-sendkeys-cx.log`,
`/tmp/shandalar-win8test-cdrive-patched-sendkeys-cx.log`, and
`/tmp/shandalar-mtg-cdrive-patched-sendkeys-cx.log`, which used Wine `wscript`
SendKeys against the patched repo and C-drive binaries and got past the
post-color resource load point. Treat them as crash-point verification, not
full gameplay proof.
