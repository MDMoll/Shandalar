# Git Handoff

This page describes how to hand off the current branch. Use
`tools/print-share-status.sh` for live branch, commit, upstream, and artifact
paths. At this update, the active branch is `codex/fix-runtime-path-zlib` and
has no upstream yet.

## Current Branch State

| Field | Value |
| --- | --- |
| Branch | `codex/fix-runtime-path-zlib` at this update; confirm with `git branch --show-current`. |
| Remote | `origin` |
| Remote URL | `https://github.com/MDMoll/Shandalar.git` |
| Latest local commit | Run `git log --oneline -1` |
| Push status | Pending for `codex/fix-runtime-path-zlib` until `git push -u origin "$(git branch --show-current)"` succeeds and `tools/print-share-status.sh` reports a matching upstream. |

## Pre-Push Check

Run from `/Users/mdmoll/Shandalar/Shandalar`:

```sh
git status --short --untracked-files=all
tools/verify-share-readiness.sh
git log --oneline -10
```

Expected `git status --short --untracked-files=all` output is empty. If local
ignored files such as `.DS_Store`, `scan-targets.tsv`, or scanner reports are
present, remove them before pushing.

## Push Command For New Commits

```sh
git push -u origin "$(git branch --show-current)"
```

Do not push directly to `master` for this cleanup/runtime branch.

If future pushes hit credentials again, use [push-auth.md](push-auth.md) for
HTTPS token, GitHub CLI, and SSH setup paths. Do not commit tokens, private
keys, credential-helper output, or `.codex/` environment files.

## Bundle Fallback

If authenticated push is unavailable in a future environment but you need to
move the branch as Git history, create a bundle from the clean checkout:

```sh
tools/create-git-handoff-bundle.sh --dry-run
tools/create-git-handoff-bundle.sh --replace
tools/create-patch-package.sh --replace --verify-apply
tools/verify-handoff-artifacts.sh
```

The default bundle path is `/private/tmp/<branch>-<sha>.bundle`, and the helper
also writes `/private/tmp/<branch>-<sha>.bundle.sha256`. By default the bundle is
incremental against `master`, so the receiver should already have the repo's
`master` branch. Use `--full` for a larger bundle that also includes the base
ref. This fallback does not replace pushing the branch to GitHub; it is a
handoff path when credentials are the only blocker.

The `--replace` option is intentionally narrow: it only overwrites the exact
default artifact path and checksum sidecar when they already exist as regular
files. It refuses directories and symlinks.

To import the incremental bundle into a clone that already has `master`, run
the receiver commands printed by the helper. The command shape is:

```sh
cd /path/to/directory-containing-bundle
shasum -a 256 -c codex-shandalar-crossover-updates-<sha>.bundle.sha256
git bundle verify /path/to/codex-shandalar-crossover-updates-<sha>.bundle
git fetch /path/to/<current-branch>-<sha>.bundle refs/heads/<current-branch>:refs/heads/<current-branch>
git switch <current-branch>
```

This flow was verified locally in a disposable `master`-only clone with an
incremental bundle and checksum sidecar produced by
`tools/create-git-handoff-bundle.sh`.
To repeat that verification for the current commit, run:

```sh
tools/verify-handoff-readiness.sh --verify-bundle-import
```

If the default bundle and patch artifacts already exist for the current commit,
include them in the same handoff pass:

```sh
tools/verify-handoff-readiness.sh --verify-bundle-import --verify-artifacts
```

After creating the default bundle and patch artifacts for the current commit,
run `tools/verify-handoff-artifacts.sh`. It checks both checksum sidecars,
imports the bundle into a disposable `master` clone, and applies the patch to a
base index to confirm the resulting Git tree matches the branch tip. If you pass
an explicit branch, use the same ref spelling you used when creating the
artifacts, because the default artifact path includes the branch/ref text.

## Earlier Push Failure In This Environment

Earlier push attempts from this Codex environment failed with:

```text
fatal: could not read Username for 'https://github.com': Device not configured
```

An SSH push attempt also failed earlier with public-key authentication. The
older `codex/shandalar-crossover-updates` branch was later pushed successfully,
but the active `codex/fix-runtime-path-zlib` branch still needs its own push or
verified bundle handoff.

## After Pushing

Use [branch-summary.md](branch-summary.md), [share-readiness.md](share-readiness.md),
and [completion-audit.md](completion-audit.md) for the branch description and
remaining gates. Do not describe the branch as a public release; see
[release-scope.md](release-scope.md).
