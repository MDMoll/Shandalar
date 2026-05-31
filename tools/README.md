# Tools

Small repository-maintenance helpers live here. They are not game runtime
files.

| Tool | Purpose |
| --- | --- |
| `verify-share-readiness.sh` | Runs automated checks for clean-tree status, expected tracked ignored files, Git binary attributes, patched runtime hashes, representative patch bytes, core docs, maintained-text ASCII, and local Markdown links. |

Run from the repository root:

```sh
tools/verify-share-readiness.sh
```

During an in-progress edit from the repository root, use
`ALLOW_DIRTY=1 tools/verify-share-readiness.sh` to skip only the clean-tree
check.
