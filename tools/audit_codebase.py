#!/usr/bin/env python3
"""Read-only codebase audit helper for the Shandalar checkout."""

from __future__ import annotations

import argparse
import csv
import os
import re
import shutil
from collections import Counter, defaultdict
from pathlib import Path


SOURCE_SUFFIXES = {
    ".c": "C",
    ".cc": "C++",
    ".cpp": "C++",
    ".cxx": "C++",
    ".h": "C/C++ header",
    ".hpp": "C++ header",
    ".asm": "assembly",
    ".s": "assembly",
    ".pl": "Perl",
    ".pm": "Perl",
    ".py": "Python",
    ".sh": "shell",
    ".bat": "batch",
    ".cmd": "batch",
    ".ps1": "PowerShell",
    ".rc": "resource",
    ".def": "module definition",
    ".mak": "make",
}

SOURCE_NAMES = {"makefile": "make"}

BUILD_SUFFIXES = {".mak", ".sln", ".vcxproj", ".vcproj", ".dsp", ".dsw", ".bat", ".cmd"}
BUILD_NAMES = {"makefile"}

HAZARD_PATTERNS = {
    "buffer-handling": re.compile(r"\b(strcpy|strcat|sprintf|vsprintf|gets\s*\(|scanf\s*\(|sscanf\s*\(|lstrcpy|lstrcat|wsprintf|_snprintf|snprintf)\b", re.I),
    "memory-allocation": re.compile(r"\b(malloc|calloc|realloc|free|new\s+|delete\s+|GlobalAlloc|LocalAlloc)\b"),
    "memory-copy": re.compile(r"\b(memcpy|memmove)\b"),
    "gdi-lifetime": re.compile(r"\b(CreateDIBSection|CreateCompatibleBitmap|CreateCompatibleDC|SelectObject|DeleteObject|DeleteDC|ReleaseDC)\b"),
    "assert-only-error-handling": re.compile(r"\b(assert|ASSERT)\b"),
    "hardcoded-path": re.compile(r"([A-Z]:\\|C:\\|D:\\|MAX_PATH|PATH_MAX|\b260\b)", re.I),
}

TODO_RE = re.compile(r"TODO|FIXME|HACK|XXX|not implemented|unimplemented|stub|temporary|dead code|obsolete|deprecated|remove me|never called|unused", re.I)
RUNTIME_REF_RE = re.compile(
    r"([A-Z]:\\|C:\\|D:\\|Program\\|Program/|Mods\\|Mods/|CardArt|Statwin|Sound|Duelsounds|Spr1024|FaceArt|FaceData|FaceButtons|Magic\.exe|Shandalar\.exe|FaceMaker|zlib\.dll)",
    re.I,
)
PATCH_RE = re.compile(r"(0x[0-9a-fA-F]{4,}|\bpatch(?:es)?\b|patch[_-]|\bhook\b|\bcode cave\b|\boffset\b|\bMagic\.exe\b|\bShandalar\.exe\b|\bManalinkEh\.dll\b|\bFaceMaker\.exe)", re.I)
PERF_RE = re.compile(r"\b(for\s*\(|while\s*\(|malloc|calloc|realloc|fopen|CreateFile|ReadFile|LoadImage|CreateDIBSection|CreateCompatible|GetPixel|SetPixel|sort|qsort|system\s*\(|popen|glob|opendir|readdir)\b")

EXCLUDED_DIRS = {".git", ".codex"}
EXCLUDED_FILES = {
    "security-scan-results.tsv",
    "scan-targets.tsv",
    "clamscan-report.txt",
    "windows-defender-report.txt",
    "codex-shandalar-crossover-updates-test.bundle",
    "codex-shandalar-crossover-updates-test.bundle.sha256",
    "codex-shandalar-crossover-updates-test.patch",
    "codex-shandalar-crossover-updates-test.patch.sha256",
}


def rel(path: Path) -> str:
    return path.as_posix()


def is_binary_sample(data: bytes) -> bool:
    if b"\0" in data:
        return True
    return False


def read_text(path: Path) -> str | None:
    try:
        data = path.read_bytes()
    except OSError:
        return None
    if is_binary_sample(data[:4096]):
        return None
    try:
        return data.decode("utf-8")
    except UnicodeDecodeError:
        try:
            return data.decode("latin-1")
        except UnicodeDecodeError:
            return None


def is_under(path: Path, parent: Path) -> bool:
    try:
        path.relative_to(parent)
        return True
    except ValueError:
        return False


def iter_files(root: Path, out_dir: Path | None) -> list[Path]:
    files: list[Path] = []
    for path in root.rglob("*"):
        relpath = path.relative_to(root)
        if any(part in EXCLUDED_DIRS for part in relpath.parts):
            continue
        if out_dir is not None and is_under(relpath, out_dir):
            continue
        if relpath.name in EXCLUDED_FILES:
            continue
        if path.is_file():
            files.append(relpath)
    return sorted(files, key=lambda p: p.as_posix().lower())


def should_scan(path: Path, scope: str) -> bool:
    if scope == "all":
        return True

    lang = language_for(path)
    area = area_for(path)
    is_root_script = len(path.parts) == 1 and lang in {"batch", "Perl", "Python", "shell", "PowerShell"}

    if scope == "source":
        return area == "source"
    if scope == "tooling":
        return area == "tooling" or is_root_script
    if scope == "focused":
        return area in {"source", "tooling"} or is_root_script

    raise ValueError(f"unknown scan scope: {scope}")


def language_for(path: Path) -> str | None:
    lower_name = path.name.lower()
    if lower_name in SOURCE_NAMES:
        return SOURCE_NAMES[lower_name]
    return SOURCE_SUFFIXES.get(path.suffix.lower())


def area_for(path: Path) -> str:
    first = path.parts[0] if path.parts else ""
    if first in {"src"} or path.as_posix().startswith("Program/src/"):
        return "source"
    if first in {"tools", "magic_updater", "PlayDeckAnalyser"}:
        return "tooling"
    if first in {"Program"}:
        return "runtime-adjacent"
    if first in {"Manalink3", "archive"}:
        return "historical"
    if first in {"docs"}:
        return "documentation"
    if first in {"Mods"}:
        return "mod-staging"
    return "unknown"


def write_tsv(path: Path, rows: list[dict[str, object]], fieldnames: list[str]) -> None:
    with path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames, delimiter="\t", lineterminator="\n")
        writer.writeheader()
        for row in rows:
            writer.writerow({key: str(row.get(key, "")).rstrip() for key in fieldnames})


def write_lines(path: Path, lines: list[str]) -> None:
    path.write_text("".join(f"{line.rstrip()}\n" for line in lines), encoding="utf-8")


def scan_matches(root: Path, files: list[Path], patterns: dict[str, re.Pattern[str]], limit_per_file: int = 200) -> list[dict[str, object]]:
    rows: list[dict[str, object]] = []
    for relpath in files:
        text = read_text(root / relpath)
        if text is None:
            continue
        matches_for_file = 0
        for lineno, line in enumerate(text.splitlines(), 1):
            for category, pattern in patterns.items():
                match = pattern.search(line)
                if match:
                    rows.append(
                        {
                            "path": rel(relpath),
                            "line": lineno,
                            "category": category,
                            "match": match.group(0).strip()[:120],
                            "text": line.strip()[:400].rstrip(),
                        }
                    )
                    matches_for_file += 1
                    break
            if matches_for_file >= limit_per_file:
                rows.append(
                    {
                        "path": rel(relpath),
                        "line": lineno,
                        "category": "truncated",
                        "match": "-",
                        "text": f"Stopped after {limit_per_file} matches in this file.",
                    }
                )
                break
    return rows


DAMAGE_PREVENTION_CALL_RE = re.compile(
    r"\b("
    r"generic_activated_ability"
    r"|granted_generic_activated_ability"
    r"|attachment_granting_activated_ability"
    r"|equipment_granting_activated_ability"
    r")\s*\("
)
DAMAGE_PREVENTION_FLAG_RE = re.compile(r"\bGAA_DAMAGE_PREVENTION(?:_[A-Z_]+)?\b")
FUNCTION_START_RE = re.compile(
    r"(?ms)^[\t ]*"
    r"(?:static[\t ]+)?"
    r"(?:int|void|char|const[\t ]+char\s*\*|uint32_t|int32_t|unsigned[\t ]+int|long|short|double|float|event_t|"
    r"target_definition_t\s*\*|card_instance_t\s*\*)"
    r"[\t \*]+([A-Za-z_]\w*)[\t ]*\([^;{}]*\)[\t \r\n]*\{"
)


def line_number_at(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def find_matching_paren(text: str, open_index: int) -> int | None:
    depth = 0
    i = open_index
    state = "code"
    while i < len(text):
        ch = text[i]
        nxt = text[i + 1] if i + 1 < len(text) else ""

        if state == "line_comment":
            if ch == "\n":
                state = "code"
            i += 1
            continue
        if state == "block_comment":
            if ch == "*" and nxt == "/":
                state = "code"
                i += 2
            else:
                i += 1
            continue
        if state == "string":
            if ch == "\\":
                i += 2
            elif ch == '"':
                state = "code"
                i += 1
            else:
                i += 1
            continue
        if state == "char":
            if ch == "\\":
                i += 2
            elif ch == "'":
                state = "code"
                i += 1
            else:
                i += 1
            continue

        if ch == "/" and nxt == "/":
            state = "line_comment"
            i += 2
            continue
        if ch == "/" and nxt == "*":
            state = "block_comment"
            i += 2
            continue
        if ch == '"':
            state = "string"
            i += 1
            continue
        if ch == "'":
            state = "char"
            i += 1
            continue
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
            if depth == 0:
                return i
        i += 1
    return None


def find_function_span(text: str, offset: int) -> tuple[str, int, int] | None:
    last_match: re.Match[str] | None = None
    for match in FUNCTION_START_RE.finditer(text, 0, offset):
        last_match = match
    if last_match is None:
        return None

    open_brace = text.find("{", last_match.start(), last_match.end())
    if open_brace < 0:
        return None

    depth = 0
    i = open_brace
    state = "code"
    while i < len(text):
        ch = text[i]
        nxt = text[i + 1] if i + 1 < len(text) else ""

        if state == "line_comment":
            if ch == "\n":
                state = "code"
            i += 1
            continue
        if state == "block_comment":
            if ch == "*" and nxt == "/":
                state = "code"
                i += 2
            else:
                i += 1
            continue
        if state == "string":
            if ch == "\\":
                i += 2
            elif ch == '"':
                state = "code"
                i += 1
            else:
                i += 1
            continue
        if state == "char":
            if ch == "\\":
                i += 2
            elif ch == "'":
                state = "code"
                i += 1
            else:
                i += 1
            continue

        if ch == "/" and nxt == "/":
            state = "line_comment"
            i += 2
            continue
        if ch == "/" and nxt == "*":
            state = "block_comment"
            i += 2
            continue
        if ch == '"':
            state = "string"
            i += 1
            continue
        if ch == "'":
            state = "char"
            i += 1
            continue
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return last_match.group(1), last_match.start(), i + 1
        i += 1

    return last_match.group(1), last_match.start(), len(text)


def find_named_function_span(text: str, name: str) -> tuple[str, int, int] | None:
    for match in FUNCTION_START_RE.finditer(text):
        if match.group(1) == name:
            open_brace = text.find("{", match.start(), match.end())
            if open_brace < 0:
                return None
            matched_end = find_matching_brace(text, open_brace)
            return name, match.start(), matched_end if matched_end is not None else len(text)
    return None


def find_matching_brace(text: str, open_index: int) -> int | None:
    depth = 0
    i = open_index
    state = "code"
    while i < len(text):
        ch = text[i]
        nxt = text[i + 1] if i + 1 < len(text) else ""

        if state == "line_comment":
            if ch == "\n":
                state = "code"
            i += 1
            continue
        if state == "block_comment":
            if ch == "*" and nxt == "/":
                state = "code"
                i += 2
            else:
                i += 1
            continue
        if state == "string":
            if ch == "\\":
                i += 2
            elif ch == '"':
                state = "code"
                i += 1
            else:
                i += 1
            continue
        if state == "char":
            if ch == "\\":
                i += 2
            elif ch == "'":
                state = "code"
                i += 1
            else:
                i += 1
            continue

        if ch == "/" and nxt == "/":
            state = "line_comment"
            i += 2
            continue
        if ch == "/" and nxt == "*":
            state = "block_comment"
            i += 2
            continue
        if ch == '"':
            state = "string"
            i += 1
            continue
        if ch == "'":
            state = "char"
            i += 1
            continue
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return i + 1
        i += 1
    return None


def scan_damage_prevention_handlers(root: Path, source_files: list[Path]) -> list[dict[str, object]]:
    rows: list[dict[str, object]] = []
    helper_guard_lines = damage_prevention_helper_guard_lines(root)
    for relpath in source_files:
        if relpath.parts[:1] not in (("src",), ("Program",)):
            continue
        if relpath.parts[0] == "Program" and relpath.parts[:2] != ("Program", "src"):
            continue
        if relpath.suffix.lower() not in {".c", ".h"}:
            continue

        text = read_text(root / relpath)
        if text is None or "GAA_DAMAGE_PREVENTION" not in text:
            continue

        for match in DAMAGE_PREVENTION_CALL_RE.finditer(text):
            open_index = text.find("(", match.start(), match.end())
            close_index = find_matching_paren(text, open_index)
            if close_index is None:
                continue

            call_text = text[match.start() : close_index + 1]
            flags = sorted(set(DAMAGE_PREVENTION_FLAG_RE.findall(call_text)))
            if not flags:
                continue

            span = find_function_span(text, match.start())
            if span is None:
                function = "<unknown>"
                function_text = ""
                function_start = 0
            else:
                function, function_start, function_end = span
                function_text = text[function_start:function_end]

            guard_lines = [
                str(line_number_at(text, function_start + guard_match.start()))
                for guard_match in re.finditer(r"\bLCBP_DAMAGE_PREVENTION\b", function_text)
            ]
            snapshot = "Program/src" if relpath.parts[:2] == ("Program", "src") else relpath.parts[0]
            if guard_lines:
                status = "same-function-guard"
                guard_source = function
            elif helper_guard_lines.get(snapshot):
                status = "covered-by-generic-helper"
                guard_source = "granted_generic_activated_ability"
                guard_lines = helper_guard_lines[snapshot]
            else:
                status = "needs-review"
                guard_source = ""

            rows.append(
                {
                    "snapshot": snapshot,
                    "path": rel(relpath),
                    "line": line_number_at(text, match.start()),
                    "function": function,
                    "call": match.group(1),
                    "flags": ";".join(flags),
                    "window_guard": status,
                    "guard_source": guard_source,
                    "guard_lines": ";".join(guard_lines),
                    "text": " ".join(call_text.split())[:500],
                }
            )

    return rows


def damage_prevention_helper_guard_lines(root: Path) -> dict[str, list[str]]:
    guard_lines_by_snapshot: dict[str, list[str]] = {}
    for snapshot, relpath in {
        "src": Path("src/functions/functions.c"),
        "Program/src": Path("Program/src/functions/functions.c"),
    }.items():
        text = read_text(root / relpath)
        if text is None:
            continue
        span = find_named_function_span(text, "granted_generic_activated_ability")
        if span is None:
            continue
        _, function_start, function_end = span
        function_text = text[function_start:function_end]
        if "GAA_DAMAGE_PREVENTION" not in function_text or "LCBP_DAMAGE_PREVENTION" not in function_text:
            continue
        guard_lines_by_snapshot[snapshot] = [
            str(line_number_at(text, function_start + guard_match.start()))
            for guard_match in re.finditer(r"\bLCBP_DAMAGE_PREVENTION\b", function_text)
        ]
    return guard_lines_by_snapshot


def run_tool_availability(out: Path) -> None:
    tools = [
        "make",
        "gcc",
        "g++",
        "clang",
        "clang++",
        "nasm",
        "yasm",
        "dlltool",
        "objcopy",
        "windres",
        "perl",
        "python3",
        "cppcheck",
        "scan-build",
        "clang-tidy",
        "infer",
        "semgrep",
        "objdump",
        "strings",
        "file",
    ]
    write_lines(out / "tool-availability.txt", [f"{tool}: {shutil.which(tool) or '<not found>'}" for tool in tools])


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--out", default="docs/generated/code-audit", help="Output directory for generated audit reports.")
    parser.add_argument(
        "--scan-scope",
        choices=("focused", "source", "tooling", "all"),
        default="focused",
        help=(
            "Files used for grep-style scan reports. Inventories always cover the whole "
            "working tree; focused scans source/tooling files and root scripts."
        ),
    )
    args = parser.parse_args()

    root = Path.cwd()
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)

    try:
        out_rel: Path | None = out.resolve().relative_to(root.resolve())
    except ValueError:
        out_rel = None
    files = iter_files(root, out_rel)
    scan_files = [path for path in files if should_scan(path, args.scan_scope)]
    write_lines(out / "all-files.txt", [rel(p) for p in files])
    write_lines(out / "scan-files.txt", [rel(p) for p in scan_files])
    write_lines(
        out / "scan-scope.txt",
        [
            f"scan-scope: {args.scan_scope}",
            f"inventory-files: {len(files)}",
            f"scan-files: {len(scan_files)}",
        ],
    )

    source_files = [p for p in files if language_for(p)]
    write_lines(out / "source-like-files.txt", [rel(p) for p in source_files])

    source_rows: list[dict[str, object]] = []
    language_counter: Counter[str] = Counter()
    language_bytes: Counter[str] = Counter()
    language_lines: Counter[str] = Counter()
    for path in source_files:
        full = root / path
        lang = language_for(path) or "unknown"
        text = read_text(full)
        line_count = len(text.splitlines()) if text is not None else ""
        size = full.stat().st_size
        source_rows.append(
            {
                "path": rel(path),
                "language": lang,
                "area": area_for(path),
                "bytes": size,
                "lines": line_count,
            }
        )
        language_counter[lang] += 1
        language_bytes[lang] += size
        if isinstance(line_count, int):
            language_lines[lang] += line_count

    write_tsv(out / "source-files.tsv", source_rows, ["path", "language", "area", "bytes", "lines"])
    write_tsv(
        out / "language-summary.tsv",
        [
            {"language": lang, "files": language_counter[lang], "bytes": language_bytes[lang], "lines": language_lines[lang]}
            for lang in sorted(language_counter)
        ],
        ["language", "files", "bytes", "lines"],
    )

    build_files = [p for p in files if p.name.lower() in BUILD_NAMES or p.suffix.lower() in BUILD_SUFFIXES]
    write_lines(out / "build-files.txt", [rel(p) for p in build_files])
    build_rows: list[dict[str, object]] = []
    for path in build_files:
        lower = path.as_posix().lower()
        if path.name.lower() == "makefile":
            kind = "makefile"
        elif path.suffix.lower() in {".bat", ".cmd"}:
            kind = "batch launcher/tool"
        else:
            kind = path.suffix.lower().lstrip(".")
        build_rows.append({"path": rel(path), "kind": kind, "area": area_for(path), "notes": "read-only inventory"})
    write_tsv(out / "build-targets.tsv", build_rows, ["path", "kind", "area", "notes"])

    run_tool_availability(out)

    hazard_rows = scan_matches(root, scan_files, HAZARD_PATTERNS)
    write_tsv(out / "grep-hazard-results.tsv", hazard_rows, ["path", "line", "category", "match", "text"])
    write_lines(out / "grep-hazard-results.txt", [f"{r['path']}:{r['line']}:{r['category']}:{r['text']}" for r in hazard_rows])

    todo_rows = scan_matches(root, scan_files, {"todo-fixme": TODO_RE})
    write_tsv(out / "todo-fixme-results.tsv", todo_rows, ["path", "line", "category", "match", "text"])
    write_lines(out / "todo-fixme-results.txt", [f"{r['path']}:{r['line']}:{r['text']}" for r in todo_rows])

    runtime_rows = scan_matches(root, scan_files, {"runtime-file-reference": RUNTIME_REF_RE})
    write_tsv(out / "runtime-file-references.tsv", runtime_rows, ["path", "line", "category", "match", "text"])
    write_tsv(
        out / "hardcoded-paths.tsv",
        [row for row in runtime_rows if re.search(r"[A-Z]:\\|C:\\|D:\\", str(row["text"]), re.I)],
        ["path", "line", "category", "match", "text"],
    )
    write_lines(out / "runtime-file-reference-grep.txt", [f"{r['path']}:{r['line']}:{r['text']}" for r in runtime_rows])

    perf_rows = scan_matches(root, scan_files, {"optimization-search": PERF_RE})
    write_tsv(out / "performance-search.tsv", perf_rows, ["path", "line", "category", "match", "text"])
    write_lines(out / "performance-search.txt", [f"{r['path']}:{r['line']}:{r['text']}" for r in perf_rows])

    damage_prevention_rows = scan_damage_prevention_handlers(root, source_files)
    damage_prevention_statuses = Counter(str(row["window_guard"]) for row in damage_prevention_rows)
    write_tsv(
        out / "damage-prevention-handlers.tsv",
        damage_prevention_rows,
        ["snapshot", "path", "line", "function", "call", "flags", "window_guard", "guard_source", "guard_lines", "text"],
    )

    patch_files = [p for p in scan_files if PATCH_RE.search(p.as_posix())]
    write_lines(out / "patch-related-files.txt", [rel(p) for p in patch_files])
    patch_rows: list[dict[str, object]] = []
    for relpath in patch_files:
        text = read_text(root / relpath)
        if text is None:
            continue
        for lineno, line in enumerate(text.splitlines(), 1):
            if PATCH_RE.search(line):
                patch_rows.append(
                    {
                        "path": rel(relpath),
                        "line": lineno,
                        "offset_or_marker": PATCH_RE.search(line).group(0),
                        "text": line.strip()[:400],
                    }
                )
    write_tsv(out / "binary-patch-sites.tsv", patch_rows, ["path", "line", "offset_or_marker", "text"])

    script_rows: list[dict[str, object]] = []
    for relpath in source_files:
        if language_for(relpath) not in {"shell", "batch", "Perl", "Python"}:
            continue
        text = read_text(root / relpath) or ""
        risk = []
        if re.search(r"\brm\s+-rf|\bdel\s+|\bcopy\s+|\bxcopy\s+|\bcp\s+", text, re.I):
            risk.append("file mutation")
        if re.search(r"[A-Z]:\\|c:\\|d:\\", text, re.I):
            risk.append("hardcoded Windows path")
        if relpath.suffix.lower() == ".sh" and not text.startswith("#!"):
            risk.append("missing shebang")
        if relpath.suffix.lower() == ".sh" and "set -euo pipefail" not in text:
            risk.append("no strict shell mode")
        if risk:
            script_rows.append({"path": rel(relpath), "language": language_for(relpath), "risk": "; ".join(risk), "area": area_for(relpath)})
    write_tsv(out / "script-tooling-risks.tsv", script_rows, ["path", "language", "risk", "area"])

    static_rows: list[dict[str, object]] = [
        {
            "tool": "tools/audit_codebase.py",
            "command": f"python3 tools/audit_codebase.py --out docs/generated/code-audit --scan-scope {args.scan_scope}",
            "scope": (
                "full working-tree inventory; grep-style scans limited by "
                f"--scan-scope {args.scan_scope}"
            ),
            "result": f"{len(files)} files inventoried; {len(scan_files)} files scanned; {len(hazard_rows)} hazard matches",
            "report_path": "docs/generated/code-audit/",
            "limitations": "regex inventory only; not a compiler or semantic analyzer",
        },
        {
            "tool": "tools/audit_codebase.py damage-prevention inventory",
            "command": f"python3 tools/audit_codebase.py --out docs/generated/code-audit --scan-scope {args.scan_scope}",
            "scope": "generic activated-ability calls using GAA_DAMAGE_PREVENTION flags in src and Program/src",
            "result": (
                f"{len(damage_prevention_rows)} activated damage-prevention call-site rows inventoried; "
                + ", ".join(f"{key}={damage_prevention_statuses[key]}" for key in sorted(damage_prevention_statuses))
            ),
            "report_path": "docs/generated/code-audit/damage-prevention-handlers.tsv",
            "limitations": "static parser only; runtime DLL proof is recorded separately in docs/bugs/duel-freeze-damage-prevention.md",
        },
        {
            "tool": "tools/check-source-snapshot-parity.sh",
            "command": "tools/check-source-snapshot-parity.sh --report-dir docs/generated/code-audit --report-only",
            "scope": "src and Program/src relative file inventory plus exact-match and marker gates",
            "result": "companion source-provenance checker; see source-snapshot-parity-summary.txt",
            "report_path": "docs/generated/code-audit/source-snapshot-parity.tsv",
            "limitations": "source provenance evidence only; not a build or runtime proof",
        },
    ]
    for tool, scope, limitation in [
        ("cppcheck", "src", "optional tool not run"),
        ("scan-build", "src", "optional tool not run"),
        ("clang-tidy", "src", "no compilation database and tool not installed"),
        ("infer", "src", "optional tool not run"),
        ("semgrep", "working tree", "optional tool not run"),
    ]:
        static_rows.append(
            {
                "tool": tool,
                "command": f"command -v {tool}",
                "scope": scope,
                "result": "available" if shutil.which(tool) else "not installed",
                "report_path": "-",
                "limitations": limitation,
            }
        )
    write_tsv(
        out / "static-analysis-results.tsv",
        static_rows,
        ["tool", "command", "scope", "result", "report_path", "limitations"],
    )

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
