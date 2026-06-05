#!/usr/bin/env python3
"""Inspect Shandalar .SVE deck-table card counts.

This is a read-only clue-finder for the deck table used by the existing Java
save tooling. It is not a complete save parser.
"""

from __future__ import annotations

import argparse
import csv
from collections import Counter
from dataclasses import dataclass
from pathlib import Path
import re
import sys


DECK_TABLE_OFFSET = 0x1420
DEFAULT_MIN_DECK_SIZE = 40
DECK_MASKS = {
    1: 0x01,
    2: 0x02,
    3: 0x04,
}
BASIC_LAND_NAMES = {
    "Swamp",
    "Island",
    "Forest",
    "Mountain",
    "Plains",
}
CANONICAL_SAVE_RE = re.compile(r"^MAGIC[0-9a-d]\.SVE$", re.IGNORECASE)


@dataclass(frozen=True)
class SaveCard:
    index: int
    offset: int
    card_id: int
    deck_mask: int
    extra: int


@dataclass(frozen=True)
class SaveDeckSummary:
    path: Path
    entries: list[SaveCard]
    terminator_offset: int | None


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Read Shandalar save deck tables and flag deck 1 counts below the "
            f"default campaign minimum ({DEFAULT_MIN_DECK_SIZE})."
        )
    )
    parser.add_argument(
        "saves",
        nargs="*",
        type=Path,
        help="Save files to inspect. Defaults to MAGIC*.SVE in the install root.",
    )
    parser.add_argument(
        "--install-root",
        type=Path,
        default=Path("."),
        help="Install/save root used when no explicit save paths are provided.",
    )
    parser.add_argument(
        "--crossover-bottle",
        help=(
            "Inspect saves from a local CrossOver bottle, for example MTG. "
            "This only reads drive_c/Shandalar/MAGIC*.SVE."
        ),
    )
    parser.add_argument(
        "--bottles-dir",
        type=Path,
        default=Path.home() / "Library/Application Support/CrossOver/Bottles",
        help="CrossOver bottles directory used with --crossover-bottle.",
    )
    parser.add_argument(
        "--cards-csv",
        type=Path,
        default=Path("CSV/Magic.exe.Cards.csv"),
        help="Optional card-name CSV used by --inactive-basics and --detail.",
    )
    parser.add_argument(
        "--min-deck-size",
        type=int,
        default=DEFAULT_MIN_DECK_SIZE,
        help="Minimum nonempty deck count to treat as OK.",
    )
    parser.add_argument(
        "--check-deck",
        type=int,
        action="append",
        choices=sorted(DECK_MASKS),
        default=None,
        help="Deck number to enforce. Defaults to deck 1 only.",
    )
    parser.add_argument(
        "--check-nonempty-decks",
        action="store_true",
        help=(
            "Check every nonempty deck slot and ignore empty optional slots. "
            "This is useful for campaign save health checks."
        ),
    )
    parser.add_argument(
        "--inactive-basics",
        action="store_true",
        help="List inactive basic lands that could be flipped into a deck.",
    )
    parser.add_argument(
        "--detail",
        action="store_true",
        help="List every parsed card entry.",
    )
    return parser.parse_args()


def fail(message: str) -> None:
    print(f"FAIL: {message}", file=sys.stderr)
    sys.exit(2)


def load_card_names(path: Path) -> dict[int, str]:
    if not path.is_file():
        return {}

    names: dict[int, str] = {}
    with path.open("r", encoding="utf-8-sig", errors="replace", newline="") as handle:
        reader = csv.DictReader(handle, delimiter=";")
        for row in reader:
            try:
                card_id = int(row["Card#"])
            except (KeyError, TypeError, ValueError):
                continue
            name = row.get("Card Name", "").strip()
            if name:
                names[card_id] = name
    return names


def find_save_paths(args: argparse.Namespace) -> list[Path]:
    if args.saves:
        return args.saves

    root = args.install_root
    if args.crossover_bottle:
        root = args.bottles_dir / args.crossover_bottle / "drive_c" / "Shandalar"

    paths = sorted(path for path in root.glob("MAGIC*.SVE") if CANONICAL_SAVE_RE.match(path.name))
    if not paths:
        fail(f"no MAGIC*.SVE files found under {root}")
    return paths


def parse_save(path: Path) -> SaveDeckSummary:
    if not path.is_file():
        fail(f"missing save file: {path}")

    data = path.read_bytes()
    if len(data) < DECK_TABLE_OFFSET + 4:
        fail(f"{path} is too small to contain deck table offset 0x{DECK_TABLE_OFFSET:x}")

    entries: list[SaveCard] = []
    offset = DECK_TABLE_OFFSET
    while offset + 4 <= len(data):
        low = data[offset]
        high = data[offset + 1]
        if low == 0xFF and high == 0xFF:
            return SaveDeckSummary(path=path, entries=entries, terminator_offset=offset)

        entries.append(
            SaveCard(
                index=len(entries) + 1,
                offset=offset,
                card_id=low | (high << 8),
                deck_mask=data[offset + 2],
                extra=data[offset + 3],
            )
        )
        offset += 4

    return SaveDeckSummary(path=path, entries=entries, terminator_offset=None)


def deck_counts(entries: list[SaveCard]) -> dict[int, int]:
    return {
        deck_number: sum(1 for entry in entries if entry.deck_mask & mask)
        for deck_number, mask in DECK_MASKS.items()
    }


def status_for_counts(
    counts: dict[int, int],
    check_decks: list[int],
    minimum: int,
    *,
    allow_empty: bool,
) -> str:
    low = [
        f"deck{deck}={counts[deck]}"
        for deck in check_decks
        if 0 < counts[deck] < minimum
    ]
    empty = [
        f"deck{deck}=0"
        for deck in check_decks
        if counts[deck] == 0 and not allow_empty
    ]
    if low:
        return f"LOW({', '.join(low)})"
    if empty:
        return f"EMPTY({', '.join(empty)})"
    return "OK"


def format_flags(entries: list[SaveCard]) -> str:
    counts = Counter(entry.deck_mask for entry in entries)
    return " ".join(f"0x{flag:02x}:{count}" for flag, count in sorted(counts.items()))


def print_detail(summary: SaveDeckSummary, card_names: dict[int, str]) -> None:
    for entry in summary.entries:
        name = card_names.get(entry.card_id, "?")
        print(
            f"  {entry.index:3d} off=0x{entry.offset:04x} "
            f"id={entry.card_id:5d} flag=0x{entry.deck_mask:02x} "
            f"extra=0x{entry.extra:02x} {name}"
        )


def print_inactive_basics(summary: SaveDeckSummary, card_names: dict[int, str]) -> None:
    rows = []
    for entry in summary.entries:
        name = card_names.get(entry.card_id)
        if name in BASIC_LAND_NAMES and entry.deck_mask in (0x00, 0x08):
            rows.append(
                f"  off=0x{entry.offset + 2:04x} "
                f"id={entry.card_id:5d} flag=0x{entry.deck_mask:02x} {name}"
            )

    if rows:
        print("  inactive basic land flag bytes:")
        print("\n".join(rows))
    else:
        print("  inactive basic land flag bytes: none found")


def main() -> int:
    args = parse_args()
    if args.check_nonempty_decks:
        check_decks = sorted(DECK_MASKS)
        allow_empty = True
    else:
        check_decks = args.check_deck if args.check_deck is not None else [1]
        allow_empty = False
    card_names = load_card_names(args.cards_csv)
    save_paths = find_save_paths(args)

    had_problem = False
    for path in save_paths:
        summary = parse_save(path)
        counts = deck_counts(summary.entries)
        status = status_for_counts(
            counts,
            check_decks,
            args.min_deck_size,
            allow_empty=allow_empty,
        )
        if not status.startswith("OK"):
            had_problem = True

        terminator = (
            f"0x{summary.terminator_offset:x}"
            if summary.terminator_offset is not None
            else "missing"
        )
        print(
            f"{path}: entries={len(summary.entries)} "
            f"deck1={counts[1]} deck2={counts[2]} deck3={counts[3]} "
            f"terminator={terminator} status={status} flags={format_flags(summary.entries)}"
        )

        if args.inactive_basics:
            print_inactive_basics(summary, card_names)
        if args.detail:
            print_detail(summary, card_names)

    return 1 if had_problem else 0


if __name__ == "__main__":
    raise SystemExit(main())
