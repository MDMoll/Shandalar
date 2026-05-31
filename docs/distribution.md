# Distribution Notes

This repo has been cleaned and documented for local maintenance and controlled
git sharing. That is not the same as proving it is safe for public
redistribution.

The current branch sharing decision is recorded in
[release-scope.md](release-scope.md): treat this as a controlled maintenance
branch only unless a patch/docs-only package or rights-verified public bundle is
prepared later.

## Verified on this machine

Run from `/Users/mdmoll/Shandalar/Shandalar` on 2026-05-31.

| Check | Result |
| --- | --- |
| `find . -maxdepth 2 \( -iname 'LICENSE*' -o -iname 'COPYING*' -o -iname '*copyright*' -o -iname '*legal*' \) -print` | No repository-level license, copying, copyright, or legal file was found. |
| `rg -n -i "registered trademark|official licensee|all other trademarks|copyright" UIStrings.txt Program/UIStrings.txt Text.res Program/Text.res Manalink3/Program/Text.res` | Bundled game strings contain Wizards of the Coast, MicroProse, trademark, official-licensee, and 1997 code/rightsholder notices. |
| Full duplicate-audit inventory | The checkout contains 52,045 non-`.git` files, including bundled binaries, card art, decks, sounds, videos, archives, and legacy resource files. |

## Practical Policy

| Topic | Current position |
| --- | --- |
| Public redistribution | Not verified. Do not claim that this repo can be published publicly as-is. |
| Private maintenance branch | Current branch scope; use only when the recipient already has the same rights context and understands the bundled-game nature of the tree. |
| Patches and documentation | These are easier to share than bundled proprietary assets, but no separate patch-only package has been prepared yet. |
| Runtime redistributables | Do not download or commit third-party runtime installers into this repo. Install them into Windows/CrossOver bottles instead. |
| Security claims | Separate issue. See [security-scan.md](security-scan.md); no malware scanner result is recorded yet. |

## If Preparing a Public Release

1. Decide whether the release should be patch/docs-only instead of a full asset
   bundle.
2. Identify which files are original game/runtime assets, fan patch files,
   generated evidence, and local-only state.
3. Verify rights or permission for any redistributed binaries, art, card data,
   deck packs, sounds, videos, and archives.
4. Remove or replace local save/player state if it is not intentional public
   fixture data.
5. Record the final scope in this file and in [share-readiness.md](share-readiness.md).

This file is a repo hygiene note, not legal advice.
