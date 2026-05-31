# Git Handoff

This branch is ready locally, but it has not been pushed from this environment
because GitHub authentication is unavailable here.

## Current Branch State

| Field | Value |
| --- | --- |
| Branch | `codex/shandalar-crossover-updates` |
| Remote | `origin` |
| Remote URL | `https://github.com/MDMoll/Shandalar.git` |
| Latest local commit | Run `git log --oneline -1` |
| Push status | Needs authenticated local terminal |

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

## Push Command

```sh
git push -u origin codex/shandalar-crossover-updates
```

Do not push directly to `master` for this cleanup/runtime branch.

If credentials are not already configured, use [push-auth.md](push-auth.md) for
HTTPS token, GitHub CLI, and SSH setup paths. Do not commit tokens, private
keys, or credential-helper output.

## Bundle Fallback

If authenticated push is still unavailable but you need to move the branch as
Git history, create a bundle from the clean checkout:

```sh
tools/create-git-handoff-bundle.sh --dry-run
tools/create-git-handoff-bundle.sh
```

The default bundle path is `/private/tmp/<branch>-<sha>.bundle`, and the helper
also writes `/private/tmp/<branch>-<sha>.bundle.sha256`. By default the bundle is
incremental against `master`, so the receiver should already have the repo's
`master` branch. Use `--full` for a larger bundle that also includes the base
ref. This fallback does not replace pushing the branch to GitHub; it is a
handoff path when credentials are the only blocker.

To import the incremental bundle into a clone that already has `master`, run
the receiver commands printed by the helper. The command shape is:

```sh
cd /path/to/directory-containing-bundle
shasum -a 256 -c codex-shandalar-crossover-updates-<sha>.bundle.sha256
git bundle verify /path/to/codex-shandalar-crossover-updates-<sha>.bundle
git fetch /path/to/codex-shandalar-crossover-updates-<sha>.bundle refs/heads/codex/shandalar-crossover-updates:refs/heads/codex/shandalar-crossover-updates
git switch codex/shandalar-crossover-updates
```

This flow was verified locally in a disposable `master`-only clone with an
incremental bundle and checksum sidecar produced by
`tools/create-git-handoff-bundle.sh`.
To repeat that verification for the current commit, run:

```sh
tools/verify-handoff-readiness.sh --verify-bundle-import
```

## Current Push Failure In This Environment

The repeated push failure from this Codex environment is:

```text
fatal: could not read Username for 'https://github.com': Device not configured
```

An SSH push attempt also failed earlier with public-key authentication. Treat
this as an environment authentication problem, not a branch readiness failure.
See [push-auth.md](push-auth.md) for the owner-side remediation paths.

## After Pushing

Use [branch-summary.md](branch-summary.md), [share-readiness.md](share-readiness.md),
and [completion-audit.md](completion-audit.md) for the branch description and
remaining gates. Do not describe the branch as a public release; see
[release-scope.md](release-scope.md).
