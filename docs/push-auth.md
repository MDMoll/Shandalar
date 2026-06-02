# Push Authentication Runbook

Use this runbook when the current branch needs to be pushed and Git credentials
are missing or stale. At this update, `tools/print-share-status.sh` reports the
active branch as `codex/fix-runtime-path-zlib` with no upstream.

Do not commit tokens, private keys, credential-helper dumps, or screenshots that
show credentials.

## Current State

| Field | Value |
| --- | --- |
| Repo path | `/Users/mdmoll/Shandalar/Shandalar` |
| Branch | Run `git branch --show-current`; active branch at this update is `codex/fix-runtime-path-zlib`. |
| Current remote | `https://github.com/MDMoll/Shandalar.git` |
| Current push state | Use `tools/print-share-status.sh`; active branch currently needs `git push -u origin codex/fix-runtime-path-zlib` before remote parity can be claimed. |
| Earlier recorded push failure here | `fatal: could not read Username for 'https://github.com': Device not configured` |

Before any future push attempt:

```sh
cd /Users/mdmoll/Shandalar/Shandalar
git status --short --untracked-files=all
tools/print-share-status.sh
tools/verify-share-readiness.sh
```

Expected `git status --short --untracked-files=all` output is empty.

## Option A: GitHub CLI / Credential Manager

This is usually the easiest HTTPS path on a personal Mac if `gh` is installed:

```sh
gh auth login
gh auth status
git push -u origin "$(git branch --show-current)"
```

During `gh auth login`, choose GitHub.com, HTTPS, and allow GitHub CLI to
authenticate Git when prompted. If `gh` is not installed or you do not want to
use it, use a personal access token with the existing HTTPS remote instead.

## Option B: Personal Access Token Over HTTPS

The current remote already uses HTTPS. Create or use a GitHub token with write
access to `MDMoll/Shandalar`, then push:

```sh
git remote -v
git push -u origin "$(git branch --show-current)"
```

When Git asks for credentials, enter your GitHub username, then enter the token
as the password. If an old or wrong credential is cached, update it in the macOS
Keychain or your configured Git credential manager, then retry the same push.

## Option C: SSH Remote

Use this only after your public SSH key is added to GitHub and `ssh -T` confirms
that GitHub accepts it:

```sh
ssh -T git@github.com
git remote set-url origin git@github.com:MDMoll/Shandalar.git
git push -u origin "$(git branch --show-current)"
```

If you prefer to keep the repo on HTTPS after pushing, switch it back:

```sh
git remote set-url origin https://github.com/MDMoll/Shandalar.git
```

## If Auth Is Still Blocked

Use the bundle fallback to move the branch as Git history without pushing from
this machine:

```sh
tools/create-git-handoff-bundle.sh --replace
```

The helper prints the checksum, `git bundle verify`, and `git fetch` commands
for the receiver. For a stronger local proof before handoff, run:

```sh
tools/verify-handoff-readiness.sh --verify-bundle-import
```

## References

| Topic | Official reference |
| --- | --- |
| HTTPS pushes and personal access tokens | <https://docs.github.com/en/get-started/getting-started-with-git/about-remote-repositories> |
| Managing personal access tokens | <https://docs.github.com/en/authentication/keeping-your-account-and-data-secure/managing-your-personal-access-tokens> |
| GitHub CLI credential caching | <https://docs.github.com/en/get-started/git-basics/caching-your-github-credentials-in-git> |
| SSH keys for GitHub | <https://docs.github.com/en/authentication/connecting-to-github-with-ssh/adding-a-new-ssh-key-to-your-github-account> |
| Changing remote URLs | <https://docs.github.com/en/get-started/git-basics/managing-remote-repositories> |
