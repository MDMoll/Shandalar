# Cleanup Launch-Copy Test

Use this checklist before moving any remaining runtime-like, save-state, art,
deck, mod, or package-tree candidate. The goal is to prove behavior in a
disposable copy before using `git mv` in the real checkout.

## Scope

| Rule | Reason |
| --- | --- |
| Do not run candidate moves in the working tree first. | The root and `Program/` layouts may depend on nearby files and exact names. |
| Copy the whole working tree, not just executables. | Launch targets load adjacent DLLs, data, art, sounds, saves, and config. Git history is not needed for launch testing. |
| Test one candidate family at a time. | If the game breaks, the cause stays inspectable. |
| Record exact paths, commands, and visible behavior. | Cleanup evidence needs to distinguish verified facts from guesses. |

## Disposable Copy

Run from `/Users/mdmoll/Shandalar/Shandalar`:

```sh
git status --short --untracked-files=all
tools/verify-share-readiness.sh
tools/create-cleanup-test-copy.sh /private/tmp/shandalar-cleanup-test
```

If the copy is for CrossOver, copy `/private/tmp/shandalar-cleanup-test` into a
test bottle under a separate name such as `C:\Shandalar-Cleanup-Test`. Do not
overwrite the active `MTG` bottle's `C:\Shandalar` install until the copy
passes.

If `/private/tmp/shandalar-cleanup-test` already exists, choose a fresh path
instead of deleting an old test copy blindly.

The helper refuses to overwrite an existing destination, omits `.git` by
default, and runs `tools/verify-share-readiness.sh` first. Use `--dry-run` to
validate the destination and verifier state without creating a large copy. For
exploratory local copies only, it also supports `--allow-dirty`,
`--allow-ignored-local`, `--include-git`, and `--skip-verify`; record those
flags in the result table if you use them.

## Verified on this machine

| Date | Command | Result |
| --- | --- | --- |
| 2026-05-31 | `tools/create-cleanup-test-copy.sh --skip-verify /private/tmp/shandalar-cleanup-copy-smoke-fd6c275b` | Created a 2.0G disposable copy with `Shandalar.exe` present and `.git` omitted. The temp copy was removed after verification. |

## Candidate Families

| ID | Candidate family | Copy-only action | Required visible proof |
| --- | --- | --- | --- |
| C1 | `CardArtNew/Thumbs.db` | Move or delete only this thumbnail cache in the copy. | Shandalar launches, deck/card screens that use `CardArtNew/` still render. |
| C2 | Root save slots: `MAGIC*.SVE`, `MAGIC*.map`, `MAGIC*.fce` | Move the whole slot set out of root in the copy. | New game, load-game screen, save game, quit, relaunch, and load the newly saved game. |
| C3 | Derived save exports: `MAGIC5`, `CSV/MAGIC3/` through `CSV/MAGIC6/` | Move these text/export artifacts out of root in the copy. | Same as C2; also confirm no tool or launcher complains about missing CSV/export files. |
| C4 | Root local player state: `Savedescs`, `FaceMostRecent.txt`, `Screennames/` | Move these together in the copy. | Character creation, default-name path, save/load, and screen-name behavior are visibly recorded. |
| C5 | Package-tree local state: `Program/Savedescs`, `Manalink3/Program/ScreenNames/` | Test only inside a package-tree copy. | `Program/Magic.exe` and the `Manalink3/` package path still launch or fail with the same known dependency issue. |
| C6 | Duplicate deck/mod/art package trees | Remove one proposed duplicate family in the copy. | Launch paths, deck selection, mod launcher paths, and at least one representative duel/deck load still work. |

## CrossOver Commands

Use a separate copy path inside a bottle. Example for bottle `MTG`:

```sh
test ! -e "$HOME/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar-Cleanup-Test"
ditto /private/tmp/shandalar-cleanup-test "$HOME/Library/Application Support/CrossOver/Bottles/MTG/drive_c/Shandalar-Cleanup-Test"
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar-Cleanup-Test" "C:\Shandalar-Cleanup-Test\Shandalar.exe"
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar-Cleanup-Test" "C:\Shandalar-Cleanup-Test\Magic.exe"
/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine --bottle MTG --workdir "C:\Shandalar-Cleanup-Test\Program" "C:\Shandalar-Cleanup-Test\Program\Magic.exe"
```

If `C:\Shandalar-Cleanup-Test` already exists, choose a fresh folder name
instead of deleting an old test copy blindly.

## Result Template

| Candidate ID | Copy path | Move/delete tested | Launch target | Result | Evidence |
| --- | --- | --- | --- | --- | --- |
| C1 | Needs testing | Needs testing | Needs testing | Needs testing | Screenshot/log/path notes |

## Approval To Change The Real Repo

Only after a candidate family passes in a copy:

1. Ask for explicit approval to move the tracked paths in the real checkout.
2. Use `git mv`, not delete.
3. Update [reorganization.md](reorganization.md), [cleanup-audit.md](cleanup-audit.md), and [cleanup-move-plan.md](cleanup-move-plan.md).
4. Run `tools/verify-share-readiness.sh`.
5. Record the copy-test evidence in this file or in a focused cleanup note.
