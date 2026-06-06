#!/usr/bin/env python3
"""Patch AI ETB player-target land triggers to use engine AI trigger mode.

Piranha Marsh and Bojuka Bog are mandatory AI-controlled enters-the-battlefield
triggers that target a player. Their resolving code already uses
pick_player_duh(), but manual Bojuka Bog testing showed that selector-side,
preselection-only, trigger-time immediate-resolution, and EVENT_END_TRIGGER
suppression caves still let the duel stop with the Spell Chain box open.

This helper replaces each land's call to comes_into_play() with a shared cave
that calls comes_into_play_mode(player, card, event, RESOLVE_TRIGGER_AI(player)).
That uses the trigger engine's normal AI/Duh trigger handling instead of a
custom Spell Chain suppression path. The existing per-card resolving block then
uses pick_player_duh() to choose the opponent without opening a human target
prompt.
"""

from __future__ import annotations

import argparse
import hashlib
import shutil
import struct
from dataclasses import dataclass
from pathlib import Path


HOOK_LENGTH = 5
CAVE_LENGTH = 0x100
CAVE_VIRTUAL_SIZE = 0x200

IMAGE_BASE = 0x02000000
EVENT_TRIGGER = 0x7D
EVENT_END_TRIGGER = 0xB00
TRIGGER_COMES_INTO_PLAY = 0xDB

AI_IS_SPECULATING = 0x00728574
TRACE_MODE = 0x00790640
TRIGGER_CONDITION = 0x007A2D78
TRIGGER_CAUSE = 0x00739A20
TRIGGER_CAUSE_CONTROLLER = 0x0062C17C
REASON_FOR_TRIGGER_CONTROLLER = 0x00737E1C
AFFECTED_CARD = 0x007A398C
AFFECTED_CARD_CONTROLLER = 0x00738C84


@dataclass(frozen=True)
class HookSpec:
    offset: int
    vma: int
    preimage: bytes


@dataclass(frozen=True)
class PatchSpec:
    cave_offset: int
    cave_vma: int
    comes_into_play_vma: int
    comes_into_play_mode_vma: int
    duh_mode_vma: int
    get_card_instance_vma: int
    opponent_is_valid_target_vma: int
    rfg_whole_graveyard_vma: int
    lose_life_vma: int
    hooks: tuple[HookSpec, ...]


KNOWN_PATCHES = {
    "ManalinkEh.dll": PatchSpec(
        cave_offset=0x495B00,
        cave_vma=0x02497100,
        comes_into_play_vma=0x02435B90,
        comes_into_play_mode_vma=0x02435AD0,
        duh_mode_vma=0x0242D940,
        get_card_instance_vma=0x0241DE70,
        opponent_is_valid_target_vma=0x0246B490,
        rfg_whole_graveyard_vma=0x024205E0,
        lose_life_vma=0x0243DC70,
        hooks=(
            HookSpec(0x3F63BD, 0x023F6DBD, bytes.fromhex("e8ceed0300")),  # Bojuka Bog
            HookSpec(0x3FE77D, 0x023FF17D, bytes.fromhex("e80e6a0300")),  # Piranha Marsh
        ),
    ),
    "Program/ManalinkEh.dll": PatchSpec(
        cave_offset=0x452D00,
        cave_vma=0x02454100,
        comes_into_play_vma=0x023F7D30,
        comes_into_play_mode_vma=0x023F7C70,
        duh_mode_vma=0x023F06A0,
        get_card_instance_vma=0x023E2970,
        opponent_is_valid_target_vma=0x0242B360,
        rfg_whole_graveyard_vma=0x023E4D50,
        lose_life_vma=0x02400280,
        hooks=(
            HookSpec(0x3BC60D, 0x023BD00D, bytes.fromhex("e81ead0300")),  # Bojuka Bog
            HookSpec(0x3C490D, 0x023C530D, bytes.fromhex("e81e2a0300")),  # Piranha Marsh
        ),
    ),
}


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def rel32(from_vma_after_instruction: int, to_vma: int) -> bytes:
    return struct.pack("<i", to_vma - from_vma_after_instruction)


def call_rel32(from_vma: int, to_vma: int) -> bytes:
    return b"\xe8" + rel32(from_vma + 5, to_vma)


class CaveBuilder:
    def __init__(self, cave_vma: int) -> None:
        self.cave_vma = cave_vma
        self.buf = bytearray()
        self.labels: dict[str, int] = {}
        self.pending_jcc8: list[tuple[int, str]] = []
        self.pending_rel32: list[tuple[int, str]] = []

    def pos(self) -> int:
        return len(self.buf)

    def vma(self) -> int:
        return self.cave_vma + self.pos()

    def mark(self, label: str) -> None:
        self.labels[label] = self.pos()

    def add(self, data: bytes) -> None:
        self.buf += data

    def jne(self, label: str) -> None:
        self.buf += b"\x75\x00"
        self.pending_jcc8.append((self.pos() - 1, label))

    def je(self, label: str) -> None:
        self.buf += b"\x74\x00"
        self.pending_jcc8.append((self.pos() - 1, label))

    def jne32(self, label: str) -> None:
        self.buf += b"\x0f\x85\x00\x00\x00\x00"
        self.pending_rel32.append((self.pos() - 4, label))

    def je32(self, label: str) -> None:
        self.buf += b"\x0f\x84\x00\x00\x00\x00"
        self.pending_rel32.append((self.pos() - 4, label))

    def jmp32(self, label: str) -> None:
        self.buf += b"\xe9\x00\x00\x00\x00"
        self.pending_rel32.append((self.pos() - 4, label))

    def call(self, to_vma: int) -> None:
        self.add(call_rel32(self.vma(), to_vma))

    def finish(self) -> bytes:
        for offset, label in self.pending_jcc8:
            if label not in self.labels:
                raise AssertionError(f"missing label {label}")
            rel = self.labels[label] - (offset + 1)
            if rel < -128 or rel > 127:
                raise AssertionError(f"short jump to {label} out of range: {rel}")
            self.buf[offset] = rel & 0xFF

        for offset, label in self.pending_rel32:
            if label not in self.labels:
                raise AssertionError(f"missing label {label}")
            rel = self.labels[label] - (offset + 4)
            struct.pack_into("<i", self.buf, offset, rel)

        if len(self.buf) > CAVE_LENGTH:
            raise AssertionError(f"cave length is {len(self.buf)}, exceeds {CAVE_LENGTH}")
        self.buf += b"\x00" * (CAVE_LENGTH - len(self.buf))
        return bytes(self.buf)


def u32(value: int) -> bytes:
    return struct.pack("<I", value)


def cmp_edi_imm8(value: int) -> bytes:
    return b"\x83\xff" + bytes([value])


def cmp_edi_imm32(value: int) -> bytes:
    return b"\x81\xff" + u32(value)


def cmp_ebx_imm8(value: int) -> bytes:
    return b"\x83\xfb" + bytes([value])


def cmp_dword_abs_imm8(addr: int, value: int) -> bytes:
    return b"\x83\x3d" + u32(addr) + bytes([value])


def cmp_dword_abs_imm32(addr: int, value: int) -> bytes:
    return b"\x81\x3d" + u32(addr) + u32(value)


def cmp_dword_abs_ebx(addr: int) -> bytes:
    return b"\x39\x1d" + u32(addr)


def cmp_dword_abs_esi(addr: int) -> bytes:
    return b"\x39\x35" + u32(addr)


def cmp_dword_ptr_esp_imm32(value: int) -> bytes:
    return b"\x81\x3c\x24" + u32(value)


def build_hook(hook: HookSpec, cave_vma: int) -> bytes:
    return call_rel32(hook.vma, cave_vma)


def build_preselect_cave(spec: PatchSpec) -> bytes:
    cave = CaveBuilder(spec.cave_vma)

    cave.add(cmp_edi_imm8(EVENT_TRIGGER))
    cave.jne("call_original")
    cave.add(cmp_ebx_imm8(1))
    cave.jne("call_original")
    cave.add(cmp_dword_abs_imm8(AI_IS_SPECULATING, 1))
    cave.je("call_original")
    cave.add(b"\xf6\x05" + u32(TRACE_MODE) + b"\x02")  # test byte [trace_mode], 2
    cave.jne("call_original")
    cave.add(cmp_dword_abs_imm32(TRIGGER_CONDITION, TRIGGER_COMES_INTO_PLAY))
    cave.jne("call_original")
    cave.add(cmp_dword_abs_ebx(AFFECTED_CARD_CONTROLLER))
    cave.jne("call_original")
    cave.add(cmp_dword_abs_esi(AFFECTED_CARD))
    cave.jne("call_original")
    cave.add(cmp_dword_abs_ebx(REASON_FOR_TRIGGER_CONTROLLER))
    cave.jne("call_original")
    cave.add(cmp_dword_abs_ebx(TRIGGER_CAUSE_CONTROLLER))
    cave.jne("call_original")
    cave.add(cmp_dword_abs_esi(TRIGGER_CAUSE))
    cave.jne("call_original")

    cave.add(b"\x56")  # push esi
    cave.add(b"\x53")  # push ebx
    cave.call(spec.opponent_is_valid_target_vma)
    cave.add(b"\x83\xc4\x08")  # add esp, 8
    cave.add(b"\x85\xc0")  # test eax, eax
    cave.je("call_original")

    cave.add(b"\x56")  # push esi
    cave.add(b"\x53")  # push ebx
    cave.call(spec.get_card_instance_vma)
    cave.add(b"\x83\xc4\x08")  # add esp, 8
    cave.add(b"\xba\x01\x00\x00\x00")  # mov edx, 1
    cave.add(b"\x29\xda")  # sub edx, ebx
    cave.add(b"\x89\x50\x74")  # mov [eax+0x74], edx
    cave.add(b"\xc7\x40\x78\xff\xff\xff\xff")  # mov dword [eax+0x78], -1
    cave.add(b"\xc6\x40\x36\x01")  # mov byte [eax+0x36], 1

    cave.mark("call_original")
    cave.add(b"\x57")  # push edi
    cave.add(b"\x56")  # push esi
    cave.add(b"\x53")  # push ebx
    cave.call(spec.comes_into_play_vma)
    cave.add(b"\x83\xc4\x0c")  # add esp, 12
    cave.add(b"\xc3")  # ret

    return cave.finish()


def build_immediate_cave(spec: PatchSpec) -> bytes:
    cave = CaveBuilder(spec.cave_vma)
    bojuka_return_vma = spec.hooks[0].vma + HOOK_LENGTH
    piranha_return_vma = spec.hooks[1].vma + HOOK_LENGTH

    cave.add(cmp_edi_imm8(EVENT_TRIGGER))
    cave.jne32("call_original")
    cave.add(cmp_ebx_imm8(1))
    cave.jne32("call_original")
    cave.add(cmp_dword_abs_imm8(AI_IS_SPECULATING, 1))
    cave.je32("call_original")
    cave.add(b"\xf6\x05" + u32(TRACE_MODE) + b"\x02")  # test byte [trace_mode], 2
    cave.jne32("call_original")
    cave.add(cmp_dword_abs_imm32(TRIGGER_CONDITION, TRIGGER_COMES_INTO_PLAY))
    cave.jne32("call_original")
    cave.add(cmp_dword_abs_ebx(AFFECTED_CARD_CONTROLLER))
    cave.jne32("call_original")
    cave.add(cmp_dword_abs_esi(AFFECTED_CARD))
    cave.jne32("call_original")
    cave.add(cmp_dword_abs_ebx(REASON_FOR_TRIGGER_CONTROLLER))
    cave.jne32("call_original")
    cave.add(cmp_dword_abs_ebx(TRIGGER_CAUSE_CONTROLLER))
    cave.jne32("call_original")
    cave.add(cmp_dword_abs_esi(TRIGGER_CAUSE))
    cave.jne32("call_original")

    cave.add(b"\x56")  # push esi
    cave.add(b"\x53")  # push ebx
    cave.call(spec.opponent_is_valid_target_vma)
    cave.add(b"\x83\xc4\x08")  # add esp, 8
    cave.add(b"\x85\xc0")  # test eax, eax
    cave.je32("call_original")

    cave.add(b"\xb8\x01\x00\x00\x00")  # mov eax, 1
    cave.add(b"\x29\xd8")  # sub eax, ebx
    cave.add(cmp_dword_ptr_esp_imm32(bojuka_return_vma))
    cave.je("resolve_bojuka")
    cave.add(cmp_dword_ptr_esp_imm32(piranha_return_vma))
    cave.je("resolve_piranha")
    cave.jmp32("call_original")

    cave.mark("resolve_bojuka")
    cave.add(b"\x50")  # push eax
    cave.call(spec.rfg_whole_graveyard_vma)
    cave.add(b"\x83\xc4\x04")  # add esp, 4
    cave.add(b"\x31\xc0")  # xor eax, eax
    cave.add(b"\xc3")  # ret

    cave.mark("resolve_piranha")
    cave.add(b"\x6a\x01")  # push 1
    cave.add(b"\x50")  # push eax
    cave.call(spec.lose_life_vma)
    cave.add(b"\x83\xc4\x08")  # add esp, 8
    cave.add(b"\x31\xc0")  # xor eax, eax
    cave.add(b"\xc3")  # ret

    cave.mark("call_original")
    cave.add(b"\x57")  # push edi
    cave.add(b"\x56")  # push esi
    cave.add(b"\x53")  # push ebx
    cave.call(spec.comes_into_play_vma)
    cave.add(b"\x83\xc4\x0c")  # add esp, 12
    cave.add(b"\xc3")  # ret

    return cave.finish()


def build_end_trigger_cave(spec: PatchSpec) -> bytes:
    cave = CaveBuilder(spec.cave_vma)

    cave.add(cmp_edi_imm8(EVENT_TRIGGER))
    cave.je32("check_context")
    cave.add(cmp_edi_imm32(EVENT_END_TRIGGER))
    cave.jne32("call_original")

    cave.mark("check_context")
    cave.add(cmp_ebx_imm8(1))
    cave.jne32("call_original")
    cave.add(cmp_dword_abs_imm8(AI_IS_SPECULATING, 1))
    cave.je32("call_original")
    cave.add(b"\xf6\x05" + u32(TRACE_MODE) + b"\x02")  # test byte [trace_mode], 2
    cave.jne32("call_original")
    cave.add(cmp_dword_abs_imm32(TRIGGER_CONDITION, TRIGGER_COMES_INTO_PLAY))
    cave.jne32("call_original")
    cave.add(cmp_dword_abs_ebx(AFFECTED_CARD_CONTROLLER))
    cave.jne32("call_original")
    cave.add(cmp_dword_abs_esi(AFFECTED_CARD))
    cave.jne32("call_original")
    cave.add(cmp_dword_abs_ebx(REASON_FOR_TRIGGER_CONTROLLER))
    cave.jne32("call_original")
    cave.add(cmp_dword_abs_ebx(TRIGGER_CAUSE_CONTROLLER))
    cave.jne32("call_original")
    cave.add(cmp_dword_abs_esi(TRIGGER_CAUSE))
    cave.jne32("call_original")

    cave.add(b"\x56")  # push esi
    cave.add(b"\x53")  # push ebx
    cave.call(spec.opponent_is_valid_target_vma)
    cave.add(b"\x83\xc4\x08")  # add esp, 8
    cave.add(b"\x85\xc0")  # test eax, eax
    cave.je32("call_original")

    cave.add(b"\x56")  # push esi
    cave.add(b"\x53")  # push ebx
    cave.call(spec.get_card_instance_vma)
    cave.add(b"\x83\xc4\x08")  # add esp, 8
    cave.add(b"\xba\x01\x00\x00\x00")  # mov edx, 1
    cave.add(b"\x29\xda")  # sub edx, ebx
    cave.add(b"\x89\x50\x74")  # mov [eax+0x74], edx
    cave.add(b"\xc7\x40\x78\xff\xff\xff\xff")  # mov dword [eax+0x78], -1
    cave.add(b"\xc6\x40\x36\x01")  # mov byte [eax+0x36], 1

    cave.add(cmp_edi_imm32(EVENT_END_TRIGGER))
    cave.je("return_resolve")
    cave.add(b"\x31\xc0")  # xor eax, eax
    cave.add(b"\xc3")  # ret

    cave.mark("return_resolve")
    cave.add(b"\xb8\x01\x00\x00\x00")  # mov eax, 1
    cave.add(b"\xc3")  # ret

    cave.mark("call_original")
    cave.add(b"\x57")  # push edi
    cave.add(b"\x56")  # push esi
    cave.add(b"\x53")  # push ebx
    cave.call(spec.comes_into_play_vma)
    cave.add(b"\x83\xc4\x0c")  # add esp, 12
    cave.add(b"\xc3")  # ret

    return cave.finish()


def build_cave(spec: PatchSpec) -> bytes:
    cave = CaveBuilder(spec.cave_vma)

    cave.add(b"\x53")  # push ebx
    cave.call(spec.duh_mode_vma)
    cave.add(b"\x83\xc4\x04")  # add esp, 4
    cave.add(b"\x85\xc0")  # test eax, eax
    cave.add(b"\xb8\x01\x00\x00\x00")  # mov eax, RESOLVE_TRIGGER_OPTIONAL
    cave.je("call_mode")
    cave.add(b"\xb8\x02\x00\x00\x00")  # mov eax, RESOLVE_TRIGGER_MANDATORY

    cave.mark("call_mode")
    cave.add(b"\x50")  # push eax
    cave.add(b"\x57")  # push edi
    cave.add(b"\x56")  # push esi
    cave.add(b"\x53")  # push ebx
    cave.call(spec.comes_into_play_mode_vma)
    cave.add(b"\x83\xc4\x10")  # add esp, 16
    cave.add(b"\xc3")  # ret

    return cave.finish()


def known_key(path: Path) -> str:
    key = path.as_posix()
    if key in KNOWN_PATCHES:
        return key

    parts = path.parts
    for index in range(len(parts)):
        candidate = Path(*parts[index:]).as_posix()
        if candidate in KNOWN_PATCHES:
            return candidate
    raise SystemExit(f"FAIL: no known AI ETB player-target patch site for {path}")


def all_zeroes(data: bytes) -> bool:
    return all(byte == 0 for byte in data)


def pe_section_table_offset(data: bytearray) -> tuple[int, int]:
    pe_offset = struct.unpack_from("<I", data, 0x3C)[0]
    if bytes(data[pe_offset : pe_offset + 4]) != b"PE\0\0":
        raise SystemExit("FAIL: not a PE file")
    number_of_sections = struct.unpack_from("<H", data, pe_offset + 6)[0]
    optional_header_size = struct.unpack_from("<H", data, pe_offset + 20)[0]
    return pe_offset + 24 + optional_header_size, number_of_sections


def ensure_cave_is_mapped(data: bytearray, spec: PatchSpec) -> bool:
    section_table, section_count = pe_section_table_offset(data)
    cave_rva = spec.cave_vma - IMAGE_BASE

    for index in range(section_count):
        header = section_table + index * 40
        virtual_size, virtual_address, raw_size, raw_offset = struct.unpack_from("<IIII", data, header + 8)
        raw_start = raw_offset
        raw_end = raw_offset + raw_size
        if raw_start <= spec.cave_offset and spec.cave_offset + CAVE_LENGTH <= raw_end:
            needed_virtual_size = max(virtual_size, cave_rva - virtual_address + CAVE_LENGTH)
            if needed_virtual_size > raw_size:
                raise SystemExit(
                    f"FAIL: cave at 0x{spec.cave_offset:x} would need virtual size "
                    f"0x{needed_virtual_size:x}, beyond raw section size 0x{raw_size:x}"
                )
            if virtual_size < CAVE_VIRTUAL_SIZE:
                struct.pack_into("<I", data, header + 8, CAVE_VIRTUAL_SIZE)
                return True
            return False

    raise SystemExit(f"FAIL: no PE section raw range contains cave offset 0x{spec.cave_offset:x}")


def patch_file(path: Path, spec: PatchSpec, apply: bool, backup_suffix: str | None) -> None:
    data = bytearray(path.read_bytes())
    cave = build_cave(spec)
    end_trigger_cave = build_end_trigger_cave(spec)
    immediate_cave = build_immediate_cave(spec)
    preselect_cave = build_preselect_cave(spec)
    current_cave = bytes(data[spec.cave_offset : spec.cave_offset + CAVE_LENGTH])
    expected_hooks = [build_hook(hook, spec.cave_vma) for hook in spec.hooks]

    already_patched = current_cave == cave
    for hook, expected in zip(spec.hooks, expected_hooks):
        current = bytes(data[hook.offset : hook.offset + HOOK_LENGTH])
        already_patched = already_patched and current == expected
        if current not in (hook.preimage, expected):
            raise SystemExit(
                f"FAIL: {path} hook @ 0x{hook.offset:x} has {current.hex()}, expected old "
                f"{hook.preimage.hex()} or patched {expected.hex()}"
            )

    if already_patched:
        print(f"ok: {path} already patched; sha256 {sha256_file(path)}")
        return

    if (
        current_cave != cave
        and current_cave != end_trigger_cave
        and current_cave != preselect_cave
        and current_cave != immediate_cave
        and not all_zeroes(current_cave)
    ):
        raise SystemExit(
            f"FAIL: {path} cave @ 0x{spec.cave_offset:x} has {current_cave.hex()}, "
            f"expected zero-filled cave, previous preselection/immediate/end-trigger cave, or patched {cave.hex()}"
        )

    if not apply:
        print(f"would patch: {path} hooks and cave @ 0x{spec.cave_offset:x}")
        return

    if backup_suffix:
        backup_path = path.with_name(path.stem + backup_suffix + path.suffix)
        if not backup_path.exists():
            shutil.copy2(path, backup_path)
            print(f"ok: preserved backup {backup_path}")
        else:
            print(f"ok: backup already exists {backup_path}")

    section_header_changed = ensure_cave_is_mapped(data, spec)
    for hook, expected in zip(spec.hooks, expected_hooks):
        data[hook.offset : hook.offset + HOOK_LENGTH] = expected
    data[spec.cave_offset : spec.cave_offset + CAVE_LENGTH] = cave
    path.write_bytes(data)

    header_note = "; expanded cave section virtual size" if section_header_changed else ""
    print(f"ok: patched {path}{header_note}; sha256 {sha256_file(path)}")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--apply", action="store_true", help="write the patch")
    parser.add_argument(
        "--backup-suffix",
        default=None,
        help="preserve a sibling backup before writing, e.g. .before-ai-etb-target-end-trigger-patch",
    )
    parser.add_argument(
        "paths",
        nargs="*",
        default=["ManalinkEh.dll", "Program/ManalinkEh.dll"],
        help="DLL paths to patch; defaults to root and Program ManalinkEh.dll",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    for text_path in args.paths:
        path = Path(text_path)
        spec = KNOWN_PATCHES[known_key(path)]
        patch_file(path, spec, args.apply, args.backup_suffix)


if __name__ == "__main__":
    main()
