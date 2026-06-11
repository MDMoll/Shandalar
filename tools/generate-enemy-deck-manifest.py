#!/usr/bin/env python3
"""Generate Shandalar enemy deck manifests and candidate variant lists.

This is intentionally report-only. It reads the exported enemy table and deck
family folders, then writes generated docs that can guide later encounter/deck
pool work without changing executable data or runtime deck folders.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import re
from collections import Counter
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Iterable


MIN_DECK_SIZE = 40
ENEMY_TABLE = Path("CSV/Shandalar.exe.Enemies.csv")
DEFAULT_OUTPUT_DIR = Path("docs/generated/enemy-decks")

DECK_SOURCES = [
    ("active-root", Path("decks"), "active root Shandalar adventure decks"),
    ("active-program", Path("Program/decks"), "active Program-path adventure decks"),
    ("variant-harder", Path("Decks - Harder"), "harder same-id variants"),
    ("variant-new-original", Path("decks - new original"), "new-original same-id variants"),
    ("variant-original-spaced", Path("Decks - Original"), "original same-id variants"),
    ("variant-original-underscore", Path("Decks_original"), "original same-id variants"),
    ("variant-alt", Path("Decks_alt"), "alternate same-id variants"),
    ("variant-revision", Path("Decks - revision"), "revision same-id variants"),
    ("variant-tim", Path("Decks - Tim"), "Tim same-id variants"),
    ("variant-rawky", Path("decks - rawky"), "rawky same-id variants"),
]

COLOR_BITS = [
    (0x08, "black"),
    (0x10, "blue"),
    (0x20, "green"),
    (0x40, "red"),
    (0x80, "white"),
]


@dataclass(frozen=True)
class DeckInfo:
    source_key: str
    source_dir: str
    source_note: str
    deck_id: int
    deck_id_text: str
    path: str
    title: str
    main_cards: int
    all_cards: int
    unique_main_cards: int
    unique_all_cards: int
    variant_sections: int
    sha256: str
    warnings: str


@dataclass(frozen=True)
class EnemyInfo:
    enemy_number: int
    article: str
    name: str
    plural: str
    gender: str
    base_life: str
    victory_type: str
    color_raw: str
    color_label: str
    deck_num: int
    deck_id_text: str
    ability: str
    reward: str
    card_id: str


def ascii_clean(value: object) -> str:
    text = "" if value is None else str(value)
    text = text.replace("\t", " ").replace("\r", " ").replace("\n", " ")
    text = re.sub(r"\s+", " ", text).strip()
    return text.encode("ascii", "replace").decode("ascii")


def markdown_cell(value: object) -> str:
    return ascii_clean(value).replace("|", "\\|")


def read_latin1(path: Path) -> str:
    return path.read_text(encoding="latin1", errors="replace")


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def deck_id_from_path(path: Path) -> int | None:
    match = re.fullmatch(r"(\d{4})\.[dD][cC][kK]", path.name)
    if not match:
        return None
    return int(match.group(1))


def parse_color(raw: str) -> str:
    try:
        value = int(raw, 0)
    except ValueError:
        if raw.startswith("0b"):
            value = int(raw[2:], 2)
        else:
            return "unknown"

    labels = [label for bit, label in COLOR_BITS if value & bit]
    if value & 0x04 and not labels:
        labels.append("colorless")
    return "+".join(labels) if labels else "none"


def parse_deck(source_key: str, source_dir: Path, source_note: str, path: Path) -> DeckInfo:
    text = read_latin1(path)
    title = ""
    fallback_comment_title = ""
    main_cards = 0
    all_cards = 0
    unique_main_cards = 0
    unique_all_cards = 0
    variant_sections = 0
    in_variant_section = False

    for raw_line in text.splitlines():
        line = raw_line.strip()
        if not line:
            continue
        if line.startswith(";"):
            if not fallback_comment_title:
                fallback_comment_title = line.lstrip(";").strip()
            continue
        if not line.startswith(".") and not title:
            title = line
            continue
        if re.match(r"^\.[vV]\S*", line):
            variant_sections += 1
            in_variant_section = True
            continue
        match = re.match(r"^\.(\d+)\s+(-?\d+)\b", line)
        if not match:
            continue
        count = int(match.group(2))
        all_cards += count
        unique_all_cards += 1
        if not in_variant_section:
            main_cards += count
            unique_main_cards += 1

    if not title:
        title = fallback_comment_title or path.stem

    warnings = []
    if main_cards < MIN_DECK_SIZE:
        warnings.append(f"main deck below {MIN_DECK_SIZE}")
    if variant_sections:
        warnings.append(f"{variant_sections} variant section(s)")

    deck_id = deck_id_from_path(path)
    assert deck_id is not None

    return DeckInfo(
        source_key=source_key,
        source_dir=source_dir.as_posix(),
        source_note=source_note,
        deck_id=deck_id,
        deck_id_text=f"{deck_id:04d}",
        path=path.as_posix(),
        title=ascii_clean(title),
        main_cards=main_cards,
        all_cards=all_cards,
        unique_main_cards=unique_main_cards,
        unique_all_cards=unique_all_cards,
        variant_sections=variant_sections,
        sha256=sha256(path),
        warnings="; ".join(warnings),
    )


def load_decks() -> dict[str, dict[int, DeckInfo]]:
    decks_by_source: dict[str, dict[int, DeckInfo]] = {}
    for source_key, source_dir, source_note in DECK_SOURCES:
        source_decks: dict[int, DeckInfo] = {}
        if source_dir.is_dir():
            for path in sorted(source_dir.iterdir(), key=lambda p: p.name.lower()):
                deck_id = deck_id_from_path(path)
                if deck_id is None:
                    continue
                source_decks[deck_id] = parse_deck(source_key, source_dir, source_note, path)
        decks_by_source[source_key] = source_decks
    return decks_by_source


def load_enemies() -> list[EnemyInfo]:
    with ENEMY_TABLE.open("r", encoding="latin1", newline="") as handle:
        rows = list(csv.DictReader(handle, delimiter=";"))

    enemies = []
    for row in rows:
        deck_num = int(row["Deck Num"])
        enemies.append(
            EnemyInfo(
                enemy_number=int(row["Enemy#"]),
                article=row["Article"],
                name=row["Name Singular"],
                plural=row["Name Plural"],
                gender=row["Gender"],
                base_life=row["Base Life"],
                victory_type=row["Victory Type"],
                color_raw=row["Color"],
                color_label=parse_color(row["Color"]),
                deck_num=deck_num,
                deck_id_text=f"{deck_num:04d}",
                ability=row["Ability"],
                reward=row["Reward"],
                card_id=row["Card ID"],
            )
        )
    return enemies


def active_status(enemy: EnemyInfo, root: DeckInfo | None, program: DeckInfo | None) -> str:
    if enemy.deck_num == 0:
        return "SPECIAL_NO_DECK"

    issues = []
    if root is None:
        issues.append("missing root deck")
    if program is None:
        issues.append("missing Program deck")
    if root is not None and root.main_cards < MIN_DECK_SIZE:
        issues.append("root below min")
    if program is not None and program.main_cards < MIN_DECK_SIZE:
        issues.append("Program below min")
    if root is not None and program is not None and root.sha256 != program.sha256:
        issues.append("root/Program hash mismatch")
    if root is not None and root.variant_sections:
        issues.append("root has variant sections")
    if program is not None and program.variant_sections:
        issues.append("Program has variant sections")
    return "OK" if not issues else "; ".join(issues)


def build_manifest_rows(enemies: list[EnemyInfo], decks_by_source: dict[str, dict[int, DeckInfo]]) -> list[dict[str, object]]:
    rows = []
    root_decks = decks_by_source["active-root"]
    program_decks = decks_by_source["active-program"]

    for enemy in enemies:
        root = root_decks.get(enemy.deck_num)
        program = program_decks.get(enemy.deck_num)
        candidate_sources = [
            source_key
            for source_key, source_decks in decks_by_source.items()
            if enemy.deck_num in source_decks
        ]
        rows.append(
            {
                "enemy_number": enemy.enemy_number,
                "enemy_name": enemy.name,
                "color": enemy.color_label,
                "base_life": enemy.base_life,
                "deck_id": enemy.deck_id_text,
                "root_path": root.path if root else "",
                "root_title": root.title if root else "",
                "root_main_cards": root.main_cards if root else "",
                "program_path": program.path if program else "",
                "program_main_cards": program.main_cards if program else "",
                "root_program_hash_match": (
                    "yes"
                    if root is not None and program is not None and root.sha256 == program.sha256
                    else "no"
                    if root is not None and program is not None
                    else ""
                ),
                "candidate_source_count": len(candidate_sources),
                "candidate_sources": ", ".join(candidate_sources),
                "status": active_status(enemy, root, program),
            }
        )
    return rows


def candidate_relation(source_key: str, info: DeckInfo, root: DeckInfo | None) -> str:
    if source_key == "active-root":
        return "current root deck"
    if source_key == "active-program":
        return "current Program deck"
    if root is not None and info.sha256 == root.sha256:
        return "same-id variant duplicate of active root"
    return "same-id variant candidate"


def build_candidate_rows(enemies: list[EnemyInfo], decks_by_source: dict[str, dict[int, DeckInfo]]) -> list[dict[str, object]]:
    rows = []
    root_decks = decks_by_source["active-root"]
    for enemy in enemies:
        if enemy.deck_num == 0:
            continue
        root = root_decks.get(enemy.deck_num)
        for source_key, _source_dir, _source_note in DECK_SOURCES:
            info = decks_by_source[source_key].get(enemy.deck_num)
            if info is None:
                continue
            rows.append(
                {
                    "enemy_number": enemy.enemy_number,
                    "enemy_name": enemy.name,
                    "color": enemy.color_label,
                    "active_deck_id": enemy.deck_id_text,
                    "candidate_source": source_key,
                    "candidate_path": info.path,
                    "candidate_title": info.title,
                    "main_cards": info.main_cards,
                    "all_cards": info.all_cards,
                    "unique_main_cards": info.unique_main_cards,
                    "variant_sections": info.variant_sections,
                    "sha256": info.sha256,
                    "relation": candidate_relation(source_key, info, root),
                    "warnings": info.warnings,
                }
            )
    return rows


def build_inventory_rows(decks_by_source: dict[str, dict[int, DeckInfo]]) -> list[dict[str, object]]:
    rows = []
    active_hashes = {info.sha256 for info in decks_by_source["active-root"].values()}
    for source_key, source_dir, source_note in DECK_SOURCES:
        source_decks = decks_by_source[source_key]
        ids = sorted(source_decks)
        main_card_counts = [source_decks[deck_id].main_cards for deck_id in ids]
        variant_count = sum(1 for info in source_decks.values() if info.variant_sections)
        below_min = sum(1 for info in source_decks.values() if info.main_cards < MIN_DECK_SIZE)
        duplicate_of_active = sum(
            1 for info in source_decks.values() if source_key != "active-root" and info.sha256 in active_hashes
        )
        rows.append(
            {
                "source_key": source_key,
                "source_dir": source_dir.as_posix(),
                "source_note": source_note,
                "deck_files": len(source_decks),
                "deck_ids": ", ".join(f"{deck_id:04d}" for deck_id in ids),
                "min_main_cards": min(main_card_counts) if main_card_counts else "",
                "max_main_cards": max(main_card_counts) if main_card_counts else "",
                "below_min_count": below_min,
                "decks_with_variant_sections": variant_count,
                "duplicates_of_active_root": duplicate_of_active,
            }
        )
    return rows


def write_tsv(path: Path, rows: list[dict[str, object]], headers: list[str]) -> None:
    with path.open("w", encoding="utf-8", newline="\n") as handle:
        handle.write("\t".join(headers) + "\n")
        for row in rows:
            values = [ascii_clean(row.get(header, "")) for header in headers]
            if values and values[-1] == "":
                values[-1] = "none"
            handle.write("\t".join(values) + "\n")


def write_json(path: Path, payload: object) -> None:
    with path.open("w", encoding="utf-8", newline="\n") as handle:
        json.dump(payload, handle, indent=2, ensure_ascii=True)
        handle.write("\n")


def write_summary(
    path: Path,
    enemies: list[EnemyInfo],
    manifest_rows: list[dict[str, object]],
    candidate_rows: list[dict[str, object]],
    inventory_rows: list[dict[str, object]],
) -> None:
    status_counts = Counter(str(row["status"]) for row in manifest_rows)
    non_lair_enemies = [enemy for enemy in enemies if enemy.deck_num != 0]
    unique_active_decks = sorted({enemy.deck_num for enemy in non_lair_enemies})
    variant_candidate_rows = [
        row for row in candidate_rows if str(row["relation"]).startswith("same-id variant")
    ]

    lines = [
        "# Enemy Deck Manifest",
        "",
        "Generated by `python3 tools/generate-enemy-deck-manifest.py`.",
        "",
        "This is a report-only inventory. It does not change Shandalar.exe, saves,",
        "encounter tables, or any `.dck` files.",
        "",
        "## Summary",
        "",
        "| Metric | Value |",
        "| --- | ---: |",
        f"| Enemy table rows | {len(enemies)} |",
        f"| Rows with a nonzero deck id | {len(non_lair_enemies)} |",
        f"| Unique referenced deck ids | {len(unique_active_decks)} |",
        f"| Candidate rows across same-id deck-family sources | {len(candidate_rows)} |",
        f"| Same-id variant candidate rows | {len(variant_candidate_rows)} |",
        "",
        "## Active Mapping Status",
        "",
        "| Status | Count |",
        "| --- | ---: |",
    ]
    for status, count in sorted(status_counts.items()):
        lines.append(f"| {markdown_cell(status)} | {count} |")

    lines.extend(
        [
            "",
            "## Deck-Family Sources",
            "",
            "| Source | Deck Files | Below Min | Variant Sections | Duplicates Of Active Root |",
            "| --- | ---: | ---: | ---: | ---: |",
        ]
    )
    for row in inventory_rows:
        lines.append(
            "| {source} | {files} | {below} | {variants} | {dupes} |".format(
                source=markdown_cell(f"{row['source_key']} ({row['source_dir']})"),
                files=row["deck_files"],
                below=row["below_min_count"],
                variants=row["decks_with_variant_sections"],
                dupes=row["duplicates_of_active_root"],
            )
        )

    lines.extend(
        [
            "",
            "## Per-Enemy Association Candidates",
            "",
            "Candidates here are same deck-id examples from the active and variant deck",
            "family folders. They are possible source files to review for a future",
            "enemy-deck pool, not implemented encounter mappings.",
            "",
            "`Playdeck/` and `Mods/PlayDeck/` are not included in these per-enemy",
            "candidates because the Shandalar adventure enemy table references numeric",
            "`decks/NNNN.dck` ids, while PlayDeck files are named deckbuilder/player",
            "decks.",
            "",
            "| Enemy | Color | Active Deck | Current Status | Candidate Sources |",
            "| --- | --- | --- | --- | --- |",
        ]
    )
    for row in manifest_rows:
        lines.append(
            "| {enemy} | {color} | {deck} | {status} | {sources} |".format(
                enemy=markdown_cell(f"{row['enemy_number']} {row['enemy_name']}"),
                color=markdown_cell(row["color"]),
                deck=markdown_cell(row["deck_id"]),
                status=markdown_cell(row["status"]),
                sources=markdown_cell(row["candidate_sources"]),
            )
        )

    lines.extend(
        [
            "",
            "## Generated Files",
            "",
            "- `enemy-deck-manifest.tsv`: one row per enemy table row with active root/Program validation.",
            "- `enemy-deck-candidates.tsv`: one row per enemy plus same-id source deck candidate.",
            "- `deck-source-inventory.tsv`: one row per deck-family source folder.",
            "- `enemy-deck-manifest.json`: structured copy of the same data for future tools.",
            "",
        ]
    )

    path.write_text("\n".join(lines), encoding="utf-8", newline="\n")


def build_payload(
    enemies: list[EnemyInfo],
    manifest_rows: list[dict[str, object]],
    candidate_rows: list[dict[str, object]],
    inventory_rows: list[dict[str, object]],
) -> dict[str, object]:
    return {
        "notes": [
            "Report-only inventory; no encounter/deck mappings are modified.",
            "Candidates are same deck-id examples from active and variant deck-family folders.",
            "Deck card counts stop at .v* variant markers for main-deck validation.",
        ],
        "min_deck_size": MIN_DECK_SIZE,
        "enemy_table": ENEMY_TABLE.as_posix(),
        "deck_sources": [
            {"source_key": key, "source_dir": path.as_posix(), "source_note": note}
            for key, path, note in DECK_SOURCES
        ],
        "enemies": [asdict(enemy) for enemy in enemies],
        "manifest": manifest_rows,
        "candidates": candidate_rows,
        "inventory": inventory_rows,
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=DEFAULT_OUTPUT_DIR,
        help=f"Directory for generated reports. Defaults to {DEFAULT_OUTPUT_DIR}.",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    output_dir: Path = args.output_dir
    if not ENEMY_TABLE.is_file():
        raise SystemExit(f"missing enemy table: {ENEMY_TABLE}")

    enemies = load_enemies()
    decks_by_source = load_decks()
    manifest_rows = build_manifest_rows(enemies, decks_by_source)
    candidate_rows = build_candidate_rows(enemies, decks_by_source)
    inventory_rows = build_inventory_rows(decks_by_source)

    output_dir.mkdir(parents=True, exist_ok=True)
    write_tsv(
        output_dir / "enemy-deck-manifest.tsv",
        manifest_rows,
        [
            "enemy_number",
            "enemy_name",
            "color",
            "base_life",
            "deck_id",
            "root_path",
            "root_title",
            "root_main_cards",
            "program_path",
            "program_main_cards",
            "root_program_hash_match",
            "candidate_source_count",
            "candidate_sources",
            "status",
        ],
    )
    write_tsv(
        output_dir / "enemy-deck-candidates.tsv",
        candidate_rows,
        [
            "enemy_number",
            "enemy_name",
            "color",
            "active_deck_id",
            "candidate_source",
            "candidate_path",
            "candidate_title",
            "main_cards",
            "all_cards",
            "unique_main_cards",
            "variant_sections",
            "sha256",
            "relation",
            "warnings",
        ],
    )
    write_tsv(
        output_dir / "deck-source-inventory.tsv",
        inventory_rows,
        [
            "source_key",
            "source_dir",
            "source_note",
            "deck_files",
            "deck_ids",
            "min_main_cards",
            "max_main_cards",
            "below_min_count",
            "decks_with_variant_sections",
            "duplicates_of_active_root",
        ],
    )
    write_json(
        output_dir / "enemy-deck-manifest.json",
        build_payload(enemies, manifest_rows, candidate_rows, inventory_rows),
    )
    write_summary(output_dir / "README.md", enemies, manifest_rows, candidate_rows, inventory_rows)

    print(f"wrote {output_dir / 'README.md'}")
    print(f"wrote {output_dir / 'enemy-deck-manifest.tsv'}")
    print(f"wrote {output_dir / 'enemy-deck-candidates.tsv'}")
    print(f"wrote {output_dir / 'deck-source-inventory.tsv'}")
    print(f"wrote {output_dir / 'enemy-deck-manifest.json'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
