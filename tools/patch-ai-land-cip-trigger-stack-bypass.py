#!/usr/bin/env python3
"""Patch AI-controlled land ETB triggers before they enter Spell Chain.

Manual testing showed that card-specific Piranha Marsh/Bojuka Bog target
preselection and direct card-callsite wrappers still allowed the duel to freeze
with the movable Spell Chain box open. The underlying bad layer is
resolve_trigger(): it pushes the opponent's mandatory ETB trigger onto Spell
Chain and opens the processing dialog before the land can auto-pick the player
target.

This patch restores the failed Piranha Marsh and Bojuka Bog ManalinkEh.dll
callsites to their normal comes_into_play() calls, then uses a generic
resolve_trigger() bypass. Non-speculating AI-owned lands whose trigger context
is exactly TRIGGER_COMES_INTO_PLAY resolve EVENT_RESOLVE_TRIGGER directly on the
card, preserve trigger globals, dispatch EVENT_TRIGGER_RESOLVED, and return
before the stack/dialog path runs. Human, speculative, trace, and non-land
triggers keep the original resolver path.

Shandalar adventure duels load Shandalar.dll, not ManalinkEh.dll, so this helper
also patches the Shandalar.dll resolver. That binary does not have enough clean
unused executable padding for the cave, so the helper adds a small executable
.cdxai section and jumps there from resolve_trigger().
"""

from __future__ import annotations

import argparse
import hashlib
import shutil
import struct
from dataclasses import dataclass
from pathlib import Path


RESOLVE_HOOK_LENGTH = 10
SHANDALAR_RESOLVE_HOOK_LENGTH = 9
CARD_CALL_LENGTH = 5
CAVE_LENGTH = 0x100
SHANDALAR_CAVE_LENGTH = 0x200
SHANDALAR_CAVE_SECTION = b".cdxai"
NOP = b"\x90"

EVENT_RESOLVE_TRIGGER = 0x7E
EVENT_TRIGGER_RESOLVED = 0x300
TRIGGER_COMES_INTO_PLAY = 0xDB
TYPE_LAND = 0x1

AI_IS_SPECULATING = 0x00728574
TRIGGER_CONDITION = 0x007A2D78
REASON_FOR_TRIGGER_CONTROLLER = 0x00737E1C
TRIGGER_CAUSE_CONTROLLER = 0x0062C17C
TRIGGER_CAUSE = 0x00739A20
SUPPRESS_THIS_TRIGGER = 0x0060E9F8

STATE_PROCESSING_OR_IS_TRIGGERING = 0x10000100
STATE_PROCESSING_AND_IS_TRIGGERING_CLEAR_MASK = 0xEFFFFEFF

OLD_CARD_WRAPPER_CAVE_PREFIX = bytes.fromhex("83ff7e0f8429000000")
SHANDALAR_RESOLVE_HOOK_PREIMAGE = bytes.fromhex("8d149b8b0d381c8e00")


@dataclass(frozen=True)
class CardCallsite:
    label: str
    offset: int
    vma: int
    original_call: bytes


@dataclass(frozen=True)
class PatchSpec:
    resolve_hook_offset: int
    resolve_hook_vma: int
    normal_continue_vma: int
    epilogue_vma: int
    cave_offset: int
    cave_vma: int
    get_card_instance_vma: int
    is_what_vma: int
    dispatch_event_with_attacker_to_one_card_vma: int
    dispatch_event_vma: int
    xtrigger_addr: int
    card_callsites: tuple[CardCallsite, ...]


@dataclass(frozen=True)
class ShandalarPatchSpec:
    resolve_hook_offset: int
    resolve_hook_vma: int
    normal_continue_vma: int
    epilogue_vma: int
    type_instance_vma: int
    dispatch_event_to_one_card_vma: int
    dispatch_event_vma: int
    ai_is_speculating_addr: int
    trace_mode_addr: int
    trigger_condition_addr: int
    reason_for_trigger_controller_addr: int
    trigger_cause_controller_addr: int
    trigger_cause_addr: int
    suppress_this_trigger_addr: int
    xtrigger_addr: int


RESOLVE_HOOK_PREIMAGE = bytes.fromhex("c705f8e9600000000000")

KNOWN_PATCHES = {
    "ManalinkEh.dll": PatchSpec(
        resolve_hook_offset=0x429ACF,
        resolve_hook_vma=0x0242A4CF,
        normal_continue_vma=0x0242A4D9,
        epilogue_vma=0x0242A63E,
        cave_offset=0x495B00,
        cave_vma=0x02497100,
        get_card_instance_vma=0x0241DE70,
        is_what_vma=0x02433610,
        dispatch_event_with_attacker_to_one_card_vma=0x02429BB0,
        dispatch_event_vma=0x004359B0,
        xtrigger_addr=0x025902A4,
        card_callsites=(
            CardCallsite("Bojuka Bog", 0x3F63BD, 0x023F6DBD, bytes.fromhex("e8ceed0300")),
            CardCallsite("Piranha Marsh", 0x3FE77D, 0x023FF17D, bytes.fromhex("e80e6a0300")),
        ),
    ),
    "Program/ManalinkEh.dll": PatchSpec(
        resolve_hook_offset=0x3EC7CF,
        resolve_hook_vma=0x023ED1CF,
        normal_continue_vma=0x023ED1D9,
        epilogue_vma=0x023ED33E,
        cave_offset=0x452D00,
        cave_vma=0x02454100,
        get_card_instance_vma=0x023E2970,
        is_what_vma=0x023F5B60,
        dispatch_event_with_attacker_to_one_card_vma=0x023ECB30,
        dispatch_event_vma=0x004359B0,
        xtrigger_addr=0x02540524,
        card_callsites=(
            CardCallsite("Bojuka Bog", 0x3BC60D, 0x023BD00D, bytes.fromhex("e81ead0300")),
            CardCallsite("Piranha Marsh", 0x3C490D, 0x023C530D, bytes.fromhex("e81e2a0300")),
        ),
    ),
}

KNOWN_SHANDALAR_PATCHES = {
    "Shandalar.dll": ShandalarPatchSpec(
        resolve_hook_offset=0x94D34,
        resolve_hook_vma=0x06095934,
        normal_continue_vma=0x0609593D,
        epilogue_vma=0x06095B7A,
        type_instance_vma=0x0607BD90,
        dispatch_event_to_one_card_vma=0x06089940,
        dispatch_event_vma=0x06056860,
        ai_is_speculating_addr=0x008BD200,
        trace_mode_addr=0x0093A934,
        trigger_condition_addr=0x0094D178,
        reason_for_trigger_controller_addr=0x008E1C38,
        trigger_cause_controller_addr=0x007BF4B8,
        trigger_cause_addr=0x008E2FC4,
        suppress_this_trigger_addr=0x007BE9AC,
        xtrigger_addr=0x06D1B764,
    ),
    "Program/Shandalar.dll": ShandalarPatchSpec(
        resolve_hook_offset=0x94D34,
        resolve_hook_vma=0x06095934,
        normal_continue_vma=0x0609593D,
        epilogue_vma=0x06095B7A,
        type_instance_vma=0x0607BD90,
        dispatch_event_to_one_card_vma=0x06089940,
        dispatch_event_vma=0x06056860,
        ai_is_speculating_addr=0x008BD200,
        trace_mode_addr=0x0093A934,
        trigger_condition_addr=0x0094D178,
        reason_for_trigger_controller_addr=0x008E1C38,
        trigger_cause_controller_addr=0x007BF4B8,
        trigger_cause_addr=0x008E2FC4,
        suppress_this_trigger_addr=0x007BE9AC,
        xtrigger_addr=0x06D1B764,
    ),
}


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def u32(value: int) -> bytes:
    return struct.pack("<I", value)


def rel32(from_vma_after_instruction: int, to_vma: int) -> bytes:
    return struct.pack("<i", to_vma - from_vma_after_instruction)


def jmp_rel32(from_vma: int, to_vma: int) -> bytes:
    return b"\xe9" + rel32(from_vma + 5, to_vma)


def call_rel32(from_vma: int, to_vma: int) -> bytes:
    return b"\xe8" + rel32(from_vma + 5, to_vma)


class CaveBuilder:
    def __init__(self, cave_vma: int, cave_length: int = CAVE_LENGTH) -> None:
        self.cave_vma = cave_vma
        self.cave_length = cave_length
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

    def jmp32(self, label: str) -> None:
        self.buf += b"\xe9\x00\x00\x00\x00"
        self.pending_rel32.append((self.pos() - 4, label))

    def jmp_abs(self, to_vma: int) -> None:
        self.add(jmp_rel32(self.vma(), to_vma))

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

        if len(self.buf) > self.cave_length:
            raise AssertionError(f"cave length is {len(self.buf)}, exceeds {self.cave_length}")

        self.buf += b"\x00" * (self.cave_length - len(self.buf))
        return bytes(self.buf)


def cmp_ebx_imm8(value: int) -> bytes:
    return b"\x83\xfb" + bytes([value])


def cmp_esi_imm8(value: int) -> bytes:
    return b"\x83\xfe" + bytes([value])


def test_byte_abs_imm8(addr: int, mask: int) -> bytes:
    return b"\xf6\x05" + u32(addr) + bytes([mask])


def cmp_dword_abs_imm8(addr: int, value: int) -> bytes:
    return b"\x83\x3d" + u32(addr) + bytes([value])


def cmp_dword_abs_imm32(addr: int, value: int) -> bytes:
    return b"\x81\x3d" + u32(addr) + u32(value)


def cmp_dword_abs_ebx(addr: int) -> bytes:
    return b"\x39\x1d" + u32(addr)


def cmp_dword_abs_esi(addr: int) -> bytes:
    return b"\x39\x35" + u32(addr)


def mov_dword_abs_imm32(addr: int, value: int) -> bytes:
    return b"\xc7\x05" + u32(addr) + u32(value)


def push_abs(addr: int) -> bytes:
    return b"\xff\x35" + u32(addr)


def pop_abs(addr: int) -> bytes:
    return b"\x8f\x05" + u32(addr)


def restore_abs_from_stack(slot: int, addr: int) -> bytes:
    if slot == 0:
        return b"\x8b\x04\x24" + b"\xa3" + u32(addr)
    return b"\x8b\x44\x24" + bytes([slot]) + b"\xa3" + u32(addr)


def build_resolve_hook(spec: PatchSpec) -> bytes:
    return jmp_rel32(spec.resolve_hook_vma, spec.cave_vma) + NOP * (RESOLVE_HOOK_LENGTH - 5)


def build_shandalar_resolve_hook(spec: ShandalarPatchSpec, cave_vma: int) -> bytes:
    return jmp_rel32(spec.resolve_hook_vma, cave_vma) + NOP * (SHANDALAR_RESOLVE_HOOK_LENGTH - 5)


def build_cave(spec: PatchSpec) -> bytes:
    cave = CaveBuilder(spec.cave_vma)

    cave.add(cmp_ebx_imm8(1))
    cave.jne("fail")
    cave.add(cmp_dword_abs_imm8(AI_IS_SPECULATING, 1))
    cave.je("fail")
    cave.add(cmp_dword_abs_imm32(TRIGGER_CONDITION, TRIGGER_COMES_INTO_PLAY))
    cave.jne("fail")
    cave.add(cmp_dword_abs_ebx(REASON_FOR_TRIGGER_CONTROLLER))
    cave.jne("fail")
    cave.add(cmp_dword_abs_ebx(TRIGGER_CAUSE_CONTROLLER))
    cave.jne("fail")
    cave.add(cmp_dword_abs_esi(TRIGGER_CAUSE))
    cave.jne("fail")

    cave.add(b"\x6a" + bytes([TYPE_LAND]))  # push TYPE_LAND
    cave.add(b"\x56")  # push esi
    cave.add(b"\x53")  # push ebx
    cave.call(spec.is_what_vma)
    cave.add(b"\x83\xc4\x0c")  # add esp, 12
    cave.add(b"\x85\xc0")  # test eax, eax
    cave.je("fail")
    cave.jmp32("handled")

    cave.mark("fail")
    cave.add(mov_dword_abs_imm32(SUPPRESS_THIS_TRIGGER, 0))
    cave.jmp_abs(spec.normal_continue_vma)

    cave.mark("handled")
    cave.add(mov_dword_abs_imm32(SUPPRESS_THIS_TRIGGER, 0))

    cave.add(push_abs(TRIGGER_CONDITION))
    cave.add(push_abs(spec.xtrigger_addr))
    cave.add(push_abs(TRIGGER_CAUSE_CONTROLLER))
    cave.add(push_abs(TRIGGER_CAUSE))

    cave.add(b"\x56")  # push esi
    cave.add(b"\x53")  # push ebx
    cave.call(spec.get_card_instance_vma)
    cave.add(b"\x83\xc4\x08")  # add esp, 8
    cave.add(b"\x89\xc7")  # mov edi, eax
    cave.add(b"\x81\x4f\x08" + u32(STATE_PROCESSING_OR_IS_TRIGGERING))

    cave.add(b"\x6a\xff")  # push -1
    cave.add(b"\xba\x01\x00\x00\x00")  # mov edx, 1
    cave.add(b"\x29\xda")  # sub edx, ebx
    cave.add(b"\x52")  # push edx
    cave.add(b"\x6a" + bytes([EVENT_RESOLVE_TRIGGER]))  # push EVENT_RESOLVE_TRIGGER
    cave.add(b"\x56")  # push esi
    cave.add(b"\x53")  # push ebx
    cave.call(spec.dispatch_event_with_attacker_to_one_card_vma)
    cave.add(b"\x83\xc4\x14")  # add esp, 20

    cave.add(restore_abs_from_stack(0, TRIGGER_CAUSE))
    cave.add(restore_abs_from_stack(4, TRIGGER_CAUSE_CONTROLLER))
    cave.add(restore_abs_from_stack(8, spec.xtrigger_addr))
    cave.add(restore_abs_from_stack(12, TRIGGER_CONDITION))

    cave.add(b"\x81\x67\x08" + u32(STATE_PROCESSING_AND_IS_TRIGGERING_CLEAR_MASK))

    cave.add(b"\x68" + u32(EVENT_TRIGGER_RESOLVED))
    cave.add(b"\x56")  # push esi
    cave.add(b"\x53")  # push ebx
    cave.call(spec.dispatch_event_vma)
    cave.add(b"\x83\xc4\x0c")  # add esp, 12

    cave.add(pop_abs(TRIGGER_CAUSE))
    cave.add(pop_abs(TRIGGER_CAUSE_CONTROLLER))
    cave.add(pop_abs(spec.xtrigger_addr))
    cave.add(pop_abs(TRIGGER_CONDITION))
    cave.jmp_abs(spec.epilogue_vma)

    return cave.finish()


def build_shandalar_cave(spec: ShandalarPatchSpec, cave_vma: int) -> bytes:
    cave = CaveBuilder(cave_vma, SHANDALAR_CAVE_LENGTH)

    cave.add(cmp_esi_imm8(1))
    cave.jne("fail")
    cave.add(cmp_dword_abs_imm8(spec.ai_is_speculating_addr, 1))
    cave.je("fail")
    cave.add(test_byte_abs_imm8(spec.trace_mode_addr, 2))
    cave.jne("fail")
    cave.add(cmp_dword_abs_imm32(spec.trigger_condition_addr, TRIGGER_COMES_INTO_PLAY))
    cave.jne("fail")
    cave.add(cmp_dword_abs_esi(spec.reason_for_trigger_controller_addr))
    cave.jne("fail")
    cave.add(cmp_dword_abs_esi(spec.trigger_cause_controller_addr))
    cave.jne("fail")
    cave.add(cmp_dword_abs_ebx(spec.trigger_cause_addr))
    cave.jne("fail")

    cave.add(b"\x8d\x14\x9b")  # lea edx, [ebx + 4*ebx]
    cave.add(b"\x89\xd0")  # mov eax, edx
    cave.add(b"\xc1\xe0\x04")  # shl eax, 4
    cave.add(b"\x29\xd0")  # sub eax, edx
    cave.add(b"\x8d\x14\x87")  # lea edx, [edi + 4*eax]
    cave.add(b"\x52")  # push edx
    cave.call(spec.type_instance_vma)
    cave.add(b"\xa9" + u32(TYPE_LAND))  # test eax, TYPE_LAND
    cave.je("fail_pop_instance")
    cave.add(b"\x5f")  # pop edi
    cave.jmp32("handled")

    cave.mark("fail_pop_instance")
    cave.add(b"\x5a")  # pop edx

    cave.mark("fail")
    cave.add(b"\x8d\x14\x9b")  # lea edx, [ebx + 4*ebx]
    cave.add(b"\x8b\x0d" + u32(spec.reason_for_trigger_controller_addr))
    cave.jmp_abs(spec.normal_continue_vma)

    cave.mark("handled")
    cave.add(mov_dword_abs_imm32(spec.suppress_this_trigger_addr, 0))

    cave.add(push_abs(spec.trigger_condition_addr))
    cave.add(push_abs(spec.xtrigger_addr))
    cave.add(push_abs(spec.trigger_cause_controller_addr))
    cave.add(push_abs(spec.trigger_cause_addr))

    cave.add(b"\x81\x4f\x08" + u32(STATE_PROCESSING_OR_IS_TRIGGERING))

    cave.add(b"\x6a\x00")  # push 0
    cave.add(b"\x6a\xff")  # push -1
    cave.add(b"\xba\x01\x00\x00\x00")  # mov edx, 1
    cave.add(b"\x29\xf2")  # sub edx, esi
    cave.add(b"\x52")  # push edx
    cave.add(b"\x53")  # push ebx
    cave.add(b"\x56")  # push esi
    cave.add(b"\x6a" + bytes([EVENT_RESOLVE_TRIGGER]))
    cave.add(b"\x53")  # push ebx
    cave.add(b"\x56")  # push esi
    cave.call(spec.dispatch_event_to_one_card_vma)
    cave.add(b"\x83\xc4\x20")  # add esp, 32

    cave.add(restore_abs_from_stack(0, spec.trigger_cause_addr))
    cave.add(restore_abs_from_stack(4, spec.trigger_cause_controller_addr))
    cave.add(restore_abs_from_stack(8, spec.xtrigger_addr))
    cave.add(restore_abs_from_stack(12, spec.trigger_condition_addr))

    cave.add(b"\x81\x67\x08" + u32(STATE_PROCESSING_AND_IS_TRIGGERING_CLEAR_MASK))
    cave.add(mov_dword_abs_imm32(spec.suppress_this_trigger_addr, 0))

    cave.add(b"\x68" + u32(EVENT_TRIGGER_RESOLVED))
    cave.add(b"\x53")  # push ebx
    cave.add(b"\x56")  # push esi
    cave.call(spec.dispatch_event_vma)
    cave.add(b"\x83\xc4\x0c")  # add esp, 12

    cave.add(pop_abs(spec.trigger_cause_addr))
    cave.add(pop_abs(spec.trigger_cause_controller_addr))
    cave.add(pop_abs(spec.xtrigger_addr))
    cave.add(pop_abs(spec.trigger_condition_addr))
    cave.jmp_abs(spec.epilogue_vma)

    return cave.finish()


def align(value: int, alignment: int) -> int:
    return (value + alignment - 1) & ~(alignment - 1)


@dataclass(frozen=True)
class SectionInfo:
    name: bytes
    virtual_size: int
    virtual_address: int
    raw_size: int
    raw_offset: int
    header_offset: int


@dataclass(frozen=True)
class PeInfo:
    pe_offset: int
    coff_offset: int
    optional_offset: int
    optional_size: int
    section_table_offset: int
    num_sections: int
    image_base: int
    section_alignment: int
    file_alignment: int
    size_of_image: int
    size_of_headers: int
    sections: tuple[SectionInfo, ...]


def read_u16(data: bytes | bytearray, offset: int) -> int:
    return struct.unpack_from("<H", data, offset)[0]


def read_u32(data: bytes | bytearray, offset: int) -> int:
    return struct.unpack_from("<I", data, offset)[0]


def parse_pe(data: bytes | bytearray) -> PeInfo:
    pe_offset = read_u32(data, 0x3C)
    if bytes(data[pe_offset : pe_offset + 4]) != b"PE\x00\x00":
        raise ValueError("not a PE file")

    coff_offset = pe_offset + 4
    num_sections = read_u16(data, coff_offset + 2)
    optional_size = read_u16(data, coff_offset + 16)
    optional_offset = coff_offset + 20
    magic = read_u16(data, optional_offset)
    if magic != 0x10B:
        raise ValueError(f"unsupported PE optional-header magic 0x{magic:x}")

    image_base = read_u32(data, optional_offset + 28)
    section_alignment = read_u32(data, optional_offset + 32)
    file_alignment = read_u32(data, optional_offset + 36)
    size_of_image = read_u32(data, optional_offset + 56)
    size_of_headers = read_u32(data, optional_offset + 60)
    section_table_offset = optional_offset + optional_size

    sections = []
    for index in range(num_sections):
        header_offset = section_table_offset + index * 40
        raw_name = bytes(data[header_offset : header_offset + 8])
        name = raw_name.rstrip(b"\x00")
        sections.append(
            SectionInfo(
                name=name,
                virtual_size=read_u32(data, header_offset + 8),
                virtual_address=read_u32(data, header_offset + 12),
                raw_size=read_u32(data, header_offset + 16),
                raw_offset=read_u32(data, header_offset + 20),
                header_offset=header_offset,
            )
        )

    return PeInfo(
        pe_offset=pe_offset,
        coff_offset=coff_offset,
        optional_offset=optional_offset,
        optional_size=optional_size,
        section_table_offset=section_table_offset,
        num_sections=num_sections,
        image_base=image_base,
        section_alignment=section_alignment,
        file_alignment=file_alignment,
        size_of_image=size_of_image,
        size_of_headers=size_of_headers,
        sections=tuple(sections),
    )


def ensure_cave_section(data: bytearray, section_name: bytes) -> tuple[int, int, bool]:
    info = parse_pe(data)
    for section in info.sections:
        if section.name == section_name:
            if section.raw_size < SHANDALAR_CAVE_LENGTH or section.virtual_size < SHANDALAR_CAVE_LENGTH:
                raise SystemExit(f"FAIL: existing {section_name.decode()} section is too small")
            return section.raw_offset, info.image_base + section.virtual_address, True

    header_offset = info.section_table_offset + info.num_sections * 40
    if header_offset + 40 > info.size_of_headers:
        raise SystemExit("FAIL: no PE header room for .cdxai cave section")
    if any(data[header_offset : header_offset + 40]):
        raise SystemExit("FAIL: next PE section header slot is not empty")

    raw_size = align(SHANDALAR_CAVE_LENGTH, info.file_alignment)
    raw_offset = align(len(data), info.file_alignment)
    virtual_size = SHANDALAR_CAVE_LENGTH
    virtual_address = align(info.size_of_image, info.section_alignment)
    size_of_image = align(virtual_address + virtual_size, info.section_alignment)

    if len(data) < raw_offset:
        data.extend(b"\x00" * (raw_offset - len(data)))
    data.extend(b"\x00" * raw_size)

    name = section_name[:8].ljust(8, b"\x00")
    section_header = (
        name
        + u32(virtual_size)
        + u32(virtual_address)
        + u32(raw_size)
        + u32(raw_offset)
        + u32(0)
        + u32(0)
        + struct.pack("<H", 0)
        + struct.pack("<H", 0)
        + u32(0x60000020)
    )
    data[header_offset : header_offset + 40] = section_header
    struct.pack_into("<H", data, info.coff_offset + 2, info.num_sections + 1)
    struct.pack_into("<I", data, info.optional_offset + 56, size_of_image)

    return raw_offset, info.image_base + virtual_address, False


def known_key(path: Path) -> str:
    parts = path.parts
    if len(parts) >= 2 and parts[-2] == "Program" and parts[-1] == "ManalinkEh.dll":
        return "Program/ManalinkEh.dll"
    if parts and parts[-1] == "ManalinkEh.dll":
        return "ManalinkEh.dll"
    raise SystemExit(f"FAIL: no known AI land ETB stack-bypass patch site for {path}")


def known_shandalar_key(path: Path) -> str:
    parts = path.parts
    if len(parts) >= 2 and parts[-2] == "Program" and parts[-1] == "Shandalar.dll":
        return "Program/Shandalar.dll"
    if parts and parts[-1] == "Shandalar.dll":
        return "Shandalar.dll"
    raise SystemExit(f"FAIL: no known Shandalar AI land ETB stack-bypass patch site for {path}")


def is_shandalar_path(path: Path) -> bool:
    return path.parts and path.parts[-1] == "Shandalar.dll"


def current_is_acceptable_cave_preimage(current_cave: bytes, new_cave: bytes) -> bool:
    return (
        current_cave == new_cave
        or all(byte == 0 for byte in current_cave)
        or current_cave.startswith(OLD_CARD_WRAPPER_CAVE_PREFIX)
    )


def patch_file(path: Path, spec: PatchSpec, apply: bool, backup_suffix: str | None) -> None:
    data = bytearray(path.read_bytes())
    hook = build_resolve_hook(spec)
    cave = build_cave(spec)
    label = f"{path} resolver hook @ 0x{spec.resolve_hook_offset:x}, cave @ 0x{spec.cave_offset:x}"

    current_hook = bytes(data[spec.resolve_hook_offset : spec.resolve_hook_offset + RESOLVE_HOOK_LENGTH])
    current_cave = bytes(data[spec.cave_offset : spec.cave_offset + CAVE_LENGTH])

    if current_hook not in (RESOLVE_HOOK_PREIMAGE, hook):
        raise SystemExit(
            f"FAIL: {path} resolver hook has {current_hook.hex()}, expected old "
            f"{RESOLVE_HOOK_PREIMAGE.hex()} or patched {hook.hex()}"
        )
    if not current_is_acceptable_cave_preimage(current_cave, cave):
        raise SystemExit(
            f"FAIL: {path} cave @ 0x{spec.cave_offset:x} does not match an expected "
            "empty, previous-wrapper, or patched preimage"
        )

    card_site_status: list[tuple[CardCallsite, bytes, bytes]] = []
    for site in spec.card_callsites:
        current = bytes(data[site.offset : site.offset + CARD_CALL_LENGTH])
        wrapper_call = call_rel32(site.vma, spec.cave_vma)
        if current not in (site.original_call, wrapper_call):
            raise SystemExit(
                f"FAIL: {path} {site.label} callsite @ 0x{site.offset:x} has "
                f"{current.hex()}, expected original {site.original_call.hex()} "
                f"or wrapper {wrapper_call.hex()}"
            )
        card_site_status.append((site, current, wrapper_call))

    already_patched = (
        current_hook == hook
        and current_cave == cave
        and all(current == site.original_call for site, current, _ in card_site_status)
    )
    if already_patched:
        print(f"ok: {label} already patched; sha256 {sha256_file(path)}")
        return

    if not apply:
        print(f"would patch: {label}; cave length {len(cave.rstrip(bytes([0])))} bytes")
        for site, current, wrapper_call in card_site_status:
            if current == wrapper_call:
                print(f"would restore: {path} {site.label} callsite @ 0x{site.offset:x}")
        return

    if backup_suffix:
        backup_path = path.with_name(path.stem + backup_suffix + path.suffix)
        if not backup_path.exists():
            shutil.copy2(path, backup_path)
            print(f"ok: preserved backup {backup_path}")
        else:
            print(f"ok: backup already exists {backup_path}")

    data[spec.resolve_hook_offset : spec.resolve_hook_offset + RESOLVE_HOOK_LENGTH] = hook
    data[spec.cave_offset : spec.cave_offset + CAVE_LENGTH] = cave
    for site, _current, _wrapper_call in card_site_status:
        data[site.offset : site.offset + CARD_CALL_LENGTH] = site.original_call

    path.write_bytes(data)
    print(f"ok: patched {label}; sha256 {sha256_file(path)}")


def patch_shandalar_file(path: Path, spec: ShandalarPatchSpec, apply: bool, backup_suffix: str | None) -> None:
    data = bytearray(path.read_bytes())
    cave_offset, cave_vma, section_already_present = ensure_cave_section(data, SHANDALAR_CAVE_SECTION)
    hook = build_shandalar_resolve_hook(spec, cave_vma)
    cave = build_shandalar_cave(spec, cave_vma)
    label = (
        f"{path} resolver hook @ 0x{spec.resolve_hook_offset:x}, "
        f"{SHANDALAR_CAVE_SECTION.decode()} cave @ 0x{cave_offset:x}"
    )

    current_hook = bytes(data[spec.resolve_hook_offset : spec.resolve_hook_offset + SHANDALAR_RESOLVE_HOOK_LENGTH])
    current_cave = bytes(data[cave_offset : cave_offset + SHANDALAR_CAVE_LENGTH])

    if current_hook not in (SHANDALAR_RESOLVE_HOOK_PREIMAGE, hook):
        raise SystemExit(
            f"FAIL: {path} Shandalar resolver hook has {current_hook.hex()}, expected old "
            f"{SHANDALAR_RESOLVE_HOOK_PREIMAGE.hex()} or patched {hook.hex()}"
        )
    if current_cave not in (bytes(SHANDALAR_CAVE_LENGTH), cave):
        raise SystemExit(
            f"FAIL: {path} .cdxai cave @ 0x{cave_offset:x} does not match an expected "
            "empty or patched preimage"
        )

    already_patched = current_hook == hook and current_cave == cave and section_already_present
    if already_patched:
        print(f"ok: {label} already patched; sha256 {sha256_file(path)}")
        return

    if not apply:
        action = "would validate existing section and patch" if section_already_present else "would add .cdxai and patch"
        print(f"{action}: {label}; cave length {len(cave.rstrip(bytes([0])))} bytes")
        return

    if backup_suffix:
        backup_path = path.with_name(path.stem + backup_suffix + path.suffix)
        if not backup_path.exists():
            shutil.copy2(path, backup_path)
            print(f"ok: preserved backup {backup_path}")
        else:
            print(f"ok: backup already exists {backup_path}")

    data[spec.resolve_hook_offset : spec.resolve_hook_offset + SHANDALAR_RESOLVE_HOOK_LENGTH] = hook
    data[cave_offset : cave_offset + SHANDALAR_CAVE_LENGTH] = cave

    path.write_bytes(data)
    print(f"ok: patched {label}; sha256 {sha256_file(path)}")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--apply",
        action="store_true",
        help="write the patch; without this, only validate and print planned changes",
    )
    parser.add_argument(
        "--backup-suffix",
        default=".before-ai-land-cip-stack-bypass-patch",
        help="preserve a sibling backup before writing; set to an empty string to disable",
    )
    parser.add_argument(
        "paths",
        nargs="*",
        default=["ManalinkEh.dll", "Program/ManalinkEh.dll", "Shandalar.dll", "Program/Shandalar.dll"],
        help="DLL paths to patch; defaults to root and Program ManalinkEh.dll plus Shandalar.dll",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    backup_suffix = args.backup_suffix if args.backup_suffix else None
    for text_path in args.paths:
        path = Path(text_path)
        if is_shandalar_path(path):
            spec = KNOWN_SHANDALAR_PATCHES[known_shandalar_key(path)]
            patch_shandalar_file(path, spec, args.apply, backup_suffix)
        else:
            spec = KNOWN_PATCHES[known_key(path)]
            patch_file(path, spec, args.apply, backup_suffix)


if __name__ == "__main__":
    main()
