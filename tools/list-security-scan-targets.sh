#!/usr/bin/env bash
set -euo pipefail

repo_root="$(git rev-parse --show-toplevel)"
cd "$repo_root"

python3 - <<'PY'
from pathlib import Path
import hashlib
import subprocess

KINDS = {
    ".exe": "pe-executable",
    ".dll": "pe-dll",
    ".7z": "archive",
    ".zip": "archive",
    ".cab": "archive",
    ".rar": "archive",
    ".bat": "windows-script",
    ".cmd": "windows-script",
    ".vbs": "windows-script",
    ".ps1": "windows-script",
    ".jar": "java-archive",
    ".pl": "perl-script",
    ".sh": "shell-script",
}


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


tracked = subprocess.check_output(["git", "ls-files"], text=True).splitlines()

print("path\tkind\tbytes\tsha256")
for rel_path in sorted(tracked):
    path = Path(rel_path)
    kind = KINDS.get(path.suffix.lower())
    if not kind:
        continue
    print(f"{rel_path}\t{kind}\t{path.stat().st_size}\t{sha256_file(path)}")
PY
