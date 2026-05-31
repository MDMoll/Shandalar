# Patch Package Plan

This branch is currently a controlled maintenance branch, not a public release.
A patch/docs-only package may be easier to share than a full bundled game tree,
but it still needs a restoration test before anyone should call it ready.

## Current Status

| Item | Status | Evidence |
| --- | --- | --- |
| Branch delta inventory | Prepared | `tools/list-branch-delta.sh` lists current branch changes relative to `master` as TSV. |
| Patch package helper | Prepared | `tools/create-patch-package.sh` creates a binary patch plus `.sha256` sidecar in a temp path. |
| Patch package file | Not committed | No `.patch`, installer, or release archive is committed. Generate temp artifacts only until a release format is chosen. |
| Restoration test | Partially automatable | `tools/create-patch-package.sh --verify-apply` can prove the patch applies to the base tree, but visible runtime testing is still separate. |
| Public redistribution | Not approved | See [distribution.md](distribution.md) and [release-scope.md](release-scope.md). |

## Inventory Command

Run from `/Users/mdmoll/Shandalar/Shandalar`:

```sh
tools/list-branch-delta.sh > /private/tmp/shandalar-branch-delta.tsv
tools/list-branch-delta.sh --summary
```

Useful review summaries:

```sh
tools/list-branch-delta.sh --summary
git diff --stat master...HEAD
```

The TSV columns are:

| Column | Meaning |
| --- | --- |
| `status` | Git change status, including rename scores such as `R100`. |
| `path` | Current path on this branch. |
| `old_path` | Previous path for renames/copies, otherwise `-`. |
| `kind` | Coarse review category such as `documentation`, `repo-metadata`, `archive-evidence`, `shell-tool`, `source`, `pe-executable`, or `art-resource`. |
| `bytes` | Current blob size on this branch, or `-` for deletes. |
| `sha256` | SHA-256 of the current branch blob, or `-` for deletes. |

## Candidate Patch Creation

Use a temp path, not the repository, until the format is chosen:

```sh
tools/create-patch-package.sh --dry-run
tools/create-patch-package.sh --verify-apply
```

That patch may contain binary deltas for patched executables, DLLs, and added
FaceMaker support assets. It is a technical transfer artifact, not proof of
permission to redistribute those bytes.

The helper verifies patch restoration against the Git index, not a checked-out
working tree. That avoids platform-specific working-tree trouble from the
case-only `readme.md` to `README.md` rename on case-insensitive filesystems.

## Required Restoration Test

Before describing a patch/docs-only package as ready:

| Step | Required proof |
| --- | --- |
| Clean base | Start from a clean checkout at the expected base commit or tag. |
| Apply patch | Apply the candidate patch without rejects. `tools/create-patch-package.sh --verify-apply` proves the Git tree result in a disposable clone. |
| Verify repo | Run `tools/verify-share-readiness.sh` and `tools/verify-handoff-readiness.sh`. |
| Verify runtime | Run the manual gameplay checklist in [manual-gameplay-verification.md](manual-gameplay-verification.md). |
| Verify security | Record a named scanner/version/result in [security-scan.md](security-scan.md). |
| Verify scope | Update [release-scope.md](release-scope.md) if the release decision changes. |

## Do Not Include Casually

| Item | Why |
| --- | --- |
| Generated scanner reports or handoff bundles | These are ignored local artifacts and should be regenerated as needed. |
| CrossOver bottle registry files | Bottle settings are machine-local; keep reproducible commands in docs. |
| Extra cleanup moves | Remaining cleanup candidates need explicit approval and launch-copy testing. |
